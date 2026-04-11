#ifndef SERVER_INTERACTION_HPP_
#define SERVER_INTERACTION_HPP_

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace survey {
class ServerInteraction {
    static size_t
    WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    static void handle_http_code(
        CURL *curl,
        CURLcode &res,
        std::string &readBuffer,
        curl_slist *headers
    );

public:
    static nlohmann::json get_survey(int survey_id);
    static void post_survey(const nlohmann::json &survey_data);
    static void post_answer(const nlohmann::json &answer_data);
    static nlohmann::json check_answer(const nlohmann::json &answer_data);
    static nlohmann::json get_passed_surveys(int user_id);
    static nlohmann::json get_survey_results(int session_id, int survey_id);
};
#endif  // SERVER_INTERACTION_HPP_
}
