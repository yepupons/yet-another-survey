#include "server_interaction.hpp"

namespace survey {
nlohmann::json ServerInteraction::load_survey(int requested_data_id) {
    CURL *curl = curl_easy_init();

    std::string readBuffer;
    std::string request_url =
        "http://127.0.0.1:8080/file?id=" + std::to_string(requested_data_id);
    curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, survey::ServerInteraction::WriteCallback
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK || http_code != 200 || readBuffer.empty()) {
        throw std::runtime_error("Failed to load survey data.");
        return;
    }
    return nlohmann::json::parse(readBuffer);
}

bool ServerInteraction::save_survey_to_server(
    const std::string &id,
    const nlohmann::json &j
) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    std::string url = "http://127.0.0.1:8080/registertest?id=" + id;
    std::string payload = j.dump();

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // check curl example for example if u like
    // https://curl.se/libcurl/c/http-post.html
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res == CURLE_OK && http_code >= 200 && http_code < 300;
}

void ServerInteraction::post_answers(
    const nlohmann::json &answers,
    int survey_id
) {
    CURL *curl = curl_easy_init();
    std::string url =
        "http://127.0.0.1:8080/response?id=" + std::to_string(survey_id);
    std::string payload = answers.dump();

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (!(res == CURLE_OK && http_code >= 200 && http_code < 300)) {
        throw std::runtime_error("Failed to send answers to the server.");
    }
}
}  // namespace survey
