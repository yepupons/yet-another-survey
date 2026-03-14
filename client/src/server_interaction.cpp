#include "server_interaction.hpp"
#include <curl/curl.h>
#include "session_id.hpp"

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
    }
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::generate_answer_template(
    int survey_id,
    int number_of_questions
) {
    nlohmann::json answer = {
        {"data",
         {{"survey_id", survey_id},
          {"answer_id", 67},
          {"respondent_id", session_id}}},
        {"sections", nlohmann::json::array()}};
    for (int i = 0; i < number_of_questions; ++i) {
        answer["sections"].push_back(nlohmann::json::array());
    }
    return answer;
}

void ServerInteraction::save_survey_to_server(
    const std::string &id,
    const nlohmann::json &j
) {
    CURL *curl = curl_easy_init();
    std::string url = "http://127.0.0.1:8080/registertest?id=" + id;
    std::string payload = j.dump();
    std::string readBuffer;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // check curl example for example if u like
    // https://curl.se/libcurl/c/http-post.html
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, survey::ServerInteraction::WriteCallback
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    handle_http_code(curl, res, readBuffer, headers);
}

void ServerInteraction::post_answers(
    const nlohmann::json &answers,
    int survey_id
) {
    CURL *curl = curl_easy_init();
    std::string url =
        "http://127.0.0.1:8080/response?id=" + std::to_string(survey_id);
    std::string payload = answers.dump();
    std::string readBuffer;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, survey::ServerInteraction::WriteCallback
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    handle_http_code(curl, res, readBuffer, headers);
}

nlohmann::json ServerInteraction::get_passed_ids(const int user_id) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string request_url =
        "http://127.0.0.1:8080/getpassed?session_id=" + std::to_string(user_id);
    struct curl_slist *headers = nullptr;
    curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, survey::ServerInteraction::WriteCallback
    );
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

void ServerInteraction::handle_http_code(
    CURL *curl,
    CURLcode &res,
    std::string &readBuffer,
    curl_slist *headers
) {
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (!(res == CURLE_OK && http_code >= 200 && http_code < 300)) {
        if (res != CURLE_OK) {
            auto error_message =
                "Network error: " + std::string(curl_easy_strerror(res));
            throw std::runtime_error(error_message);
        }
        std::string error_message =
            "Server error. HTTP code: " + std::to_string(http_code);
        if (!readBuffer.empty()) {
            error_message += ". " + readBuffer;
        }
        throw std::runtime_error(error_message);
    }
}

}  // namespace survey
