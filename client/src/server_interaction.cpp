#include "server_interaction.hpp"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <string>
#include "session.hpp"

namespace survey {
static curl_slist *append_auth_header(curl_slist *headers);

static curl_slist *append_json_auth_headers(curl_slist *headers) {
    headers = curl_slist_append(headers, "Content-Type: application/json");
    return append_auth_header(headers);
}

static curl_slist *append_auth_header(curl_slist *headers) {
    if (!session().is_authenticated()) {
        curl_slist_free_all(headers);
        throw std::runtime_error("Please log in first.");
    }

    const std::string auth_header =
        "Authorization: Bearer " + session().get_access_token();
    return curl_slist_append(headers, auth_header.c_str());
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
        std::stringstream error_message;
        if (res != CURLE_OK) {
            error_message << "Network error: " << curl_easy_strerror(res);
            throw std::runtime_error(error_message.str());
        }
        error_message << "Server error. HTTP code: " << http_code << ".";
        if (!readBuffer.empty()) {
            error_message << " " << readBuffer;
        }
        throw std::runtime_error(error_message.str());
    }
}

nlohmann::json ServerInteraction::get_survey(int survey_id) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url =
        "http://127.0.0.1:8080/survey?id=" + std::to_string(survey_id);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, nullptr);
    return nlohmann::json::parse(readBuffer);
}

void ServerInteraction::post_survey(const nlohmann::json &survey_data) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/survey";
    std::string payload = survey_data.dump();
    curl_slist *headers = nullptr;
    headers = append_json_auth_headers(headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
}

void ServerInteraction::post_answer(const nlohmann::json &answer_data) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/answer";
    std::string payload = answer_data.dump();
    curl_slist *headers = nullptr;
    headers = append_json_auth_headers(headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
}

nlohmann::json ServerInteraction::get_passed_surveys(const std::string &user_id
) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url =
        "http://127.0.0.1:8080/passed-surveys?session-id=" + user_id;
    curl_slist *headers = nullptr;
    headers = append_auth_header(headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::get_created_surveys(const std::string &user_id
) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url =
        "http://127.0.0.1:8080/created-surveys?session-id=" + user_id;
    curl_slist *headers = nullptr;
    headers = append_auth_header(headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::get_survey_statistics(int survey_id) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/statistics?survey-id=" +
                      std::to_string(survey_id);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, nullptr);
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::get_survey_results(
    const std::string &session_id,
    int survey_id
) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url =
        "http://127.0.0.1:8080/survey-results?session-id=" + session_id +
        "&survey-id=" + std::to_string(survey_id);
    curl_slist *headers = nullptr;
    headers = append_auth_header(headers);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::check_answer(const nlohmann::json &answer_data
) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/check";
    std::string payload = answer_data.dump();
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

std::string ServerInteraction::post_image(const std::string &image_path) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/image";

    curl_mime *mime;
    curl_mimepart *part;
    mime = curl_mime_init(curl);
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "image");
    curl_mime_filedata(part, image_path.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, nullptr);
    return readBuffer;
}

std::string ServerInteraction::get_image(const std::string &image_oid) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/image?id=" + image_oid;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, nullptr);
    return readBuffer;
}

nlohmann::json ServerInteraction::request_challenge() {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/challenge";
    std::string payload = "{}";
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

nlohmann::json ServerInteraction::get_challenge_status(
    const std::string &challenge_id
) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url =
        "http://127.0.0.1:8080/api/auth/telegram/challenge/" + challenge_id;
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);

    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
};

nlohmann::json ServerInteraction::complete_auth(std::string challenge_id) {
    CURL *curl = curl_easy_init();
    std::string readBuffer;
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/complete";
    nlohmann::json challenge = {};
    challenge["challenge_id"] = challenge_id;
    std::string payload = challenge.dump();
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    handle_http_code(curl, res, readBuffer, headers);
    return nlohmann::json::parse(readBuffer);
}

}  // namespace survey
