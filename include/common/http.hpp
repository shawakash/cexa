#pragma once
#include <string>
#include <map>
#include <functional>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH
};

struct HttpResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;
};

struct HttpRequestOptions {
    HttpMethod method = HttpMethod::GET;
    std::map<std::string, std::string> headers;
    json body;
    bool followRedirects = true;
    long timeout = 30L;
};

class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        HttpResponse fetch(const std::string& url, const HttpRequestOptions& options = HttpRequestOptions());

    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
        static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, std::map<std::string, std::string>* headers);

        CURL* curl;
        bool initialized;
};
