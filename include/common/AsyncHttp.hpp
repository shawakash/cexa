#include <curl/curl.h>
#include <string>
#include <cstddef>
#include <vector>
#include <map>
#include <future>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>


/**
* @brief Multi worker Async http request class
*/
class AsyncHttp {
    public:
        struct Response {
            long status_code;
            std::string body;
            std::map<std::string, std::string> headers;
        };

        AsyncHttp();
        ~AsyncHttp();

        // Copy disable
        AsyncHttp(const AsyncHttp&) = delete;
        AsyncHttp& operator=(const AsyncHttp&) = delete;

        std::future<Response> get_raw(const std::string& url,
            const std::map<std::string, std::string>& headers = {});

        std::future<Response> post_raw(const std::string& url, const nlohmann::json& json_body,
            const std::map<std::string, std::string>& headers = {});

        template<typename T>
        std::future<T> get(const std::string& url,
            const std::map<std::string, std::string>& headers = {}) {
            auto promise = std::make_shared<std::promise<T>>();
            std::future<T> future = promise->get_future();

            auto response_future = get_raw(url, headers);

            std::thread([promise, response_future = std::move(response_future)]() mutable {
                try {
                    Response response = response_future.get();
                    if (response.status_code >= 200 && response.status_code < 300) {
                        // Success - parse the response
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
        std::future<T> post(const std::string& url,
            const nlohmann::json& json_body,
            const std::map<std::string, std::string>& headers = {}) {
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

        // Method to parse the body
        template<typename T>
        T parse(const Response& response);

        void init(size_t pool_size = 10);
        void destroy();

    private:
        CURLM* multi_handle_;
        std::vector<CURL*> connection_pool_;
        std::mutex pool_mutex_;

        // threads for processing requests
        std::thread worker_thread_;
        // denotes if a request is running
        std::atomic<bool> running_;
        std::queue<std::function<void()>> task_queue_;
        std::mutex queue_mutex_;
        // condvar as in rust
        std::condition_variable cv_;

        // Internal request processing function
        void worker_loop();
        // Get the connection from pool
        CURL* get_connection();
        void return_connection(CURL* conn);

        enum Method {
            GET = 'GET',
            POST = 'POST'
        };

        std::future<Response> request(const std::string& url,
                                        const Method& method,
                                        const std::string& body,
                                        const std::map<std::string, std::string>& headers);

        static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata);
}
