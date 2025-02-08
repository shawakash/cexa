#include "http.hpp"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

HttpClient::HttpClient() : initialized(false) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        initialized = true;
    }
}

HttpClient::~HttpClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

size_t HttpClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

size_t HttpClient::HeaderCallback(void* contents, size_t size, size_t nmemb, std::map<std::string, std::string>* headers) {
    size_t totalSize = size * nmemb;
    std::string header(static_cast<char*>(contents), totalSize);

    size_t colonPos = header.find(':');
    if (colonPos != std::string::npos) {
        std::string key = header.substr(0, colonPos);
        std::string value = header.substr(colonPos + 2, header.length() - colonPos - 3); // -3 for \r\n
        (*headers)[key] = value;
    }

    return totalSize;
}

HttpResponse HttpClient::fetch(const std::string& url, const HttpRequestOptions& options) {
    HttpResponse response;
    response.statusCode = -1;

    if (!initialized || !curl) {
        throw std::runtime_error("CURL not initialized");
    }

    std::string responseBody;
    struct curl_slist* headersList = nullptr;

    // Reset CURL options
    curl_easy_reset(curl);

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set method
    switch (options.method) {
        case HttpMethod::POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            break;
        case HttpMethod::PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case HttpMethod::DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case HttpMethod::PATCH:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        default:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }

    // Set headers
    for (const auto& header : options.headers) {
        std::string headerString = header.first + ": " + header.second;
        headersList = curl_slist_append(headersList, headerString.c_str());
    }

    if (!options.body.empty()) {
        std::string bodyStr = options.body.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
        headersList = curl_slist_append(headersList, "Content-Type: application/json");
    }

    if (headersList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headersList);
    }

    // Set callbacks
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

    // Set other options
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, options.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeout);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        if (headersList) {
            curl_slist_free_all(headersList);
        }
        throw std::runtime_error(std::string("Curl error: ") + curl_easy_strerror(res));
    }

    // Get status code
    long httpCode;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    response.statusCode = static_cast<int>(httpCode);
    response.body = responseBody;

    if (headersList) {
        curl_slist_free_all(headersList);
    }

    return response;
}
