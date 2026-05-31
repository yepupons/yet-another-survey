#ifndef SERVER_INTERACTION_HPP_
#define SERVER_INTERACTION_HPP_

#include <qstringview.h>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <nlohmann/json.hpp>
#include <string>
#include "enums.hpp"

namespace survey {
class ServerInteraction : public QObject {
    Q_OBJECT
public:
    void get_survey(
        int survey_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void post_survey(
        const nlohmann::json &survey_data,
        std::function<void()> success,
        std::function<void(const std::string &)> failure
    );
    void post_answer(
        const nlohmann::json &answer_data,
        std::function<void()> success,
        std::function<void(const std::string &)> failure
    );
    void check_answer(
        const nlohmann::json &answer_data,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_passed_surveys(
        const std::string &user_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_created_surveys(
        const std::string &user_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_survey_statistics(
        int survey_id,
        const std::string &file_format,
        std::function<void(const std::string &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_survey_results(
        const std::string &user_id,
        int survey_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_image(
        const std::string &image_oid,
        std::function<void(const std::string &)> success,
        std::function<void(const std::string &)> failure
    );
    void post_image(
        const std::string &image_name,
        const QByteArray &image_data,
        std::function<void(const std::string &)> success,
        std::function<void(const std::string &)> failure
    );
    void request_challenge(
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void get_challenge_status(
        const std::string &challenge_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void complete_auth(
        const std::string &challenge_id,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );
    void generate_question(
        const nlohmann::json &section_data,
        SurveyType survey_type,
        BlockType block_type,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    );

private:
    enum class Method { POST, GET };

    QNetworkAccessManager manager_;

    void send_request(
        const std::string &url,
        Method method,
        std::function<void(const std::string &)> success,
        std::function<void(const std::string &)> failure,
        bool auth_required = false,
        const std::string &payload = ""
    );
};

inline ServerInteraction &server() {
    static ServerInteraction server;
    return server;
}
}  // namespace survey

#endif  // SERVER_INTERACTION_HPP_
