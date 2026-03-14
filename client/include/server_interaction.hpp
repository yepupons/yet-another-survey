#ifndef SERVER_INTERACTION_HPP_
#define SERVER_INTERACTION_HPP_

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace survey {
class ServerInteraction {
public:
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
    static nlohmann::json load_survey(int requested_data_id);
    static void
    save_survey_to_server(const std::string &id, const nlohmann::json &j);
    static void post_answers(const nlohmann::json &answers, int survey_id);
    static nlohmann::json get_passed_ids(const int user_id);
};
#endif  // SERVER_INTERACTION_HPP_
}
