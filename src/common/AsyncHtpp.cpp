#include "common/AsyncHttp.hpp"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstddef>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

AsyncHttp::AsyncHttp(): multi_handle_(nullptr), running_(false) {
    curl_global_init(CURL_GLOBAL_ALL);
}

void AsyncHttp::init(size_t pool_size) {
    multi_handle_ = curl_multi_init();

    // connection pool pre-allocation
    std::lock_guard<std::mutex> lock(pool_mutex_);
    for (size_t i = 0; i < pool_size; i++) {
        CURL* conn = curl_easy_init();
        if (conn) {
            curl_easy_setopt(conn, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(conn, CURLOPT_TCP_KEEPIDLE, 120L);
            curl_easy_setopt(conn, CURLOPT_TCP_KEEPINTVL, 60L);
            curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(conn, CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(conn, CURLOPT_DNS_CACHE_TIMEOUT, 100L);
            curl_easy_setopt(conn, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
            curl_easy_setopt(conn, CURLOPT_HEADER, 0L);

            connection_pool_.push_back(conn);
        }
    }

    running_ = true;
    worker_thread_ = std::thread(&AsyncHttp::worker_loop, this);
}

std::future<AsyncHttp::Response> AsyncHttp::get_raw(
    const std::string& url,
    const std::map<std::string, std::string>& headers) {
    return request(url, Method::GET, "", headers);
}

std::future<AsyncHttp::Response> AsyncHttp::post_raw(
    const std::string& url,
    const nlohmann::json& json_body,
    const std::map<std::string, std::string>& headers
) {
    std::string body = json_body.dump();

    auto request_headers = headers;
    if (request_headers.find("Content-Type") == request_headers.end()) {
        request_headers["Content-Type"] = "application/json";
    }

    return request(url, Method::POST, body, request_headers);
}

template<typename T>
std::future<T> AsyncHttp::get(const std::string& url,
    const std::map<std::string, std::string>& headers) {
    auto promise = std::make_shared<std::promise<T>>();
    std::future<T> future = promise->get_future();

    auto response_future = get_raw(url, headers);

    std::thread([promise, response_future = std::move(response_future)]() mutable {
        try {
            Response response = response_future.get();
            if (response.status_code >= 200 && response.status_code < 300) {
                T result = parse<T>(response);
                promise->set_value(std::move(result));
            } else {
                std::string error_msg = "HTTP error: " + std::to_string(response.status_code);
                if (!response.error.empty()) {
                    error_msg += " - " + response.error;
                } else if (!response.body.empty()) {
                    error_msg += " - " + response.body;
                }
                promise->set_exception(std::make_exception_ptr(std::runtime_error(error_msg)));
            }
        } catch (const std::exception& e) {
            promise->set_exception(std::current_exception());
        }
    }).detach();

    return future;
}

template<typename T>
std::future<T> AsyncHttp::post(const std::string& url,
    const nlohmann::json& json_body,
    const std::map<std::string, std::string>& headers) {
    auto promise = std::make_shared<std::promise<T>>();
    std::future<T> future = promise->get_future();

    auto response_future = post_raw(url, json_body, headers);

    std::thread([promise, response_future = std::move(response_future)]() mutable {
        try {
            Response response = response_future.get();
            if (response.status_code >= 200 && response.status_code < 300) {
                T result = parse<T>(response);
                promise->set_value(std::move(result));
            } else {
                std::string error_msg = "HTTP error: " + std::to_string(response.status_code);
                if (!response.error.empty()) {
                    error_msg += " - " + response.error;
                } else if (!response.body.empty()) {
                    error_msg += " - " + response.body;
                }
                promise->set_exception(std::make_exception_ptr(std::runtime_error(error_msg)));
            }
        } catch (const std::exception& e) {
            promise->set_exception(std::current_exception());
        }
    }).detach();

    return future;
}

std::future<AsyncHttp::Response> AsyncHttp::request(
    const std::string& url,
    const Method& method,
    const std::string& body,
    const std::map<std::string, std::string>& headers
) {
    auto promise = std::make_shared<std::promise<Response>>();
    std::future<Response> future = promise->get_future();

    std::function<void()> task = [this, url, method, body, headers, promise]() {
        Response res;
        CURL* conn = this->get_connection();

        if (!conn) {
            res.status_code = -1;
            promise->set_value(res);
            return;
        }

        curl_easy_setopt(conn, CURLOPT_URL, url.c_str());

        struct ResponseData {
            std::string body;
            std::map<std::string, std::string> headers;
        };
        ResponseData data;

        curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(conn, CURLOPT_WRITEDATA, &data.body);
        curl_easy_setopt(conn, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(conn, CURLOPT_HEADERDATA, &data.headers);

        if (method == Method::POST) {
            curl_easy_setopt(conn, CURLOPT_POST, 1L);
            curl_easy_setopt(conn, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, body.size());
        } else if (method == Method::GET) {
            curl_easy_setopt(conn, CURLOPT_HTTPGET, 1L);
        }

        struct curl_slist* curl_headers = nullptr;
        for (const auto& header : headers) {
            std::string header_str = header.first + ": " + header.second;
            curl_headers = curl_slist_append(curl_headers, header_str.c_str());
        }

        if (curl_headers) {
            curl_easy_setopt(conn, CURLOPT_HTTPHEADER, curl_headers);
        }

        // TODO: Bad take timeout as param
        curl_easy_setopt(conn, CURLOPT_TIMEOUT_MS, 500L);
        curl_easy_setopt(conn, CURLOPT_CONNECTTIMEOUT_MS, 300L);

        CURLcode response = curl_easy_perform(conn);

        if (response == CURLE_OK) {
            curl_easy_getinfo(conn, CURLINFO_RESPONSE_CODE, &res.status_code);
            res.body = std::move(data.body);
            res.headers = std::move(data.headers);
        } else {
            res.status_code = -1;
            res.body = curl_easy_strerror(response);
        }

        if (curl_headers) {
            curl_slist_free_all(curl_headers);
        }

        this->return_connection(conn);

        promise->set_value(res);
    };

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(std::move(task));
    }

    cv_.notify_one();

    return future;
}

void AsyncHttp::worker_loop() {
    while (running_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this]() { return !task_queue_.empty() || !running_; });

            if (!running_ && task_queue_.empty()) {
                break;
            }

            if (!task_queue_.empty()) {
                task = std::move(task_queue_.front());
                task_queue_.pop();
            }
        }

        if (task) {
            task();
        }
    }
}

CURL* AsyncHttp::get_connection() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    if (connection_pool_.empty()) {
        return nullptr;
    }

    CURL* conn = connection_pool_.back();
    connection_pool_.pop_back();
    return conn;
}

void AsyncHttp::return_connection(CURL* conn) {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    connection_pool_.push_back(conn);
}

AsyncHttp::~AsyncHttp() {
    destroy();
    curl_global_cleanup();
}

void AsyncHttp::destroy() {
    running_ = false;
    cv_.notify_all();

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        for (auto conn : connection_pool_) {
            curl_easy_cleanup(conn);
        }
        connection_pool_.clear();
    }

    if (multi_handle_) {
        curl_multi_cleanup(multi_handle_);
        multi_handle_ = nullptr;
    }
}

// Generic template
template<typename T>
T AsyncHttp::parse(const Response &response) {
    throw std::runtime_error("No parser implemented for this type");
}

template<>
nlohmann::json AsyncHttp::parse<nlohmann::json>(const Response& response) {
    return nlohmann::json::parse(response.body);
}

size_t AsyncHttp::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;
    std::string* body = static_cast<std::string*>(userdata);
    body->append(ptr, real_size);
    return real_size;
}

size_t AsyncHttp::header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t real_size = size * nitems;
    std::string header(buffer, real_size);

    auto headers = static_cast<std::map<std::string, std::string>*>(userdata);

    size_t colon_pos = header.find(':');
    if (colon_pos != std::string::npos) {
        std::string key = header.substr(0, colon_pos);
        std::string value = header.substr(colon_pos + 1);

        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        (*headers)[key] = value;
    }

    return real_size;
}
