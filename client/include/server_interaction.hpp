#ifndef SERVER_INTERACTION_HPP_
#define SERVER_INTERACTION_HPP_

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <nlohmann/json.hpp>
#include <string>

namespace survey {
class ServerInteraction : public QObject {
    Q_OBJECT
public:
    static nlohmann::json get_survey(int survey_id);
    static void post_survey(const nlohmann::json &survey_data);
    static void post_answer(const nlohmann::json &answer_data);
    static nlohmann::json check_answer(const nlohmann::json &answer_data);
    static nlohmann::json get_passed_surveys(const std::string &user_id);
    static nlohmann::json get_created_surveys(const std::string &user_id);
    static nlohmann::json get_survey_statistics(int survey_id);
    static nlohmann::json
    get_survey_results(const std::string &user_id, int survey_id);
    static std::string get_image(const std::string &image_oid);
    static std::string post_image(const std::string &image_path);
    static nlohmann::json request_challenge();
    static nlohmann::json get_challenge_status(const std::string &challenge_id);
    static nlohmann::json complete_auth(const std::string &challenge_id);

private:
    enum class Method { POST, GET };
    static std::string send_request(
        const std::string &url,
        Method method,
        bool auth_required = false,
        const std::string &payload = ""
    );
};
}  // namespace survey

#endif  // SERVER_INTERACTION_HPP_
