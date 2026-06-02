#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include "nlohmann/json_fwd.hpp"

static size_t write_cb(char *ptr, size_t size, size_t nmemb, std::string *out) {
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

nlohmann::json send_generate_request(const std::string &user_message) {
    nlohmann::json request_body;
    request_body["model"] = "qwen2.5:1.5b";
    request_body["format"] = "json";
    request_body["stream"] = false;
    request_body["messages"] = nlohmann::json::array();
    static const std::string system_message =
        "Ты — помощник для создания опросов.\nНа основе созданного "
        "пользователем тематического раздела с названием и вопросами, "
        "представленного в формате JSON, cгенерируй для данного раздела "
        "дополнительно еще один релевантный вопрос одного из 3 доступных "
        "типов: текстовый, с множественным выбором ответа, с единичным выбором "
        "ответа.\nОтветь строго в формате JSON.\nФормат для текстового "
        "вопроса: {\"text\": \"Текст вопроса\", \"answer\": [\"Правильный "
        "ответ 1\", \"Правильный ответ 2\", ...], \"required\": true/false - "
        "обязательно ли отвечать на вопрос}.\nФормат для вопроса c единичным "
        "выбором ответа: {\"text\": \"Текст вопроса\", \"options\": [\"Вариант "
        "ответа 1\", \"Вариант ответа 2\", ...], \"answer\": число - номер "
        "правильного варианта ответа (нумерация с 1), \"required\": true/false "
        "- обязательно ли отвечать на вопрос}.\nФормат для вопроса c "
        "множественным выбором ответа: {\"text\": \"Текст вопроса\", "
        "\"options\": [\"Вариант ответа 1\", \"Вариант ответа 2\", ...], "
        "\"answer\": [число, число, ...] - номера правильных вариантов ответа "
        "(нумерация с 1), \"required\": true/false - обязательно ли отвечать "
        "на вопрос}.\nВАЖНО: поле \"answer\" требуется добавить только в том "
        "случае, если пользователь явно указал, что создает тест.";
    request_body["messages"].push_back(
        {{"role", "system"}, {"content", system_message}}
    );
    request_body["messages"].push_back(
        {{"role", "user"}, {"content", user_message}}
    );
    std::string request = request_body.dump();

    CURL *curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("CURL is not initialized");
    }

    struct curl_slist *headers = nullptr;
    std::string response;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(res));
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    nlohmann::json response_json = nlohmann::json::parse(response);
    if (response_json.contains("message") &&
        response_json["message"].contains("content")) {
        return response_json["message"]["content"];
    }
    return nlohmann::json();
}