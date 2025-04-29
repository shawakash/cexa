#pragma once

#include <curl/curl.h>
#include <string>
#include <cstddef>
#include <vector>
#include <map>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <type_traits>


/**
* @brief Multi worker Async http request class
*/
class AsyncHttp {
    public:
        struct Response {
            long status_code;
            std::string body;
            std::string error;
            std::map<std::string, std::string> headers;
        };

        enum class Method {
            GET,
            POST
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
            const std::map<std::string, std::string>& headers = {});

        template<typename T>
        std::future<T> post(const std::string& url,
            const nlohmann::json& json_body,
            const std::map<std::string, std::string>& headers = {});

        // Method to parse the body
        template<typename T>
        static T parse(const Response& response);

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

        std::future<Response> request(const std::string& url,
                                        const Method& method,
                                        const std::string& body,
                                        const std::map<std::string, std::string>& headers);

        static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata);
};
