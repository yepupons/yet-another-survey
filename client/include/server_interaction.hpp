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

    static nlohmann::json load_survey(int requested_data_id);
    static bool
    save_survey_to_server(const std::string &id, const nlohmann::json &j);
    static void post_answers(const nlohmann::json &answers, int survey_id);
};
#endif  // SERVER_INTERACTION_HPP_
}
