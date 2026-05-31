#include "server_interaction.hpp"
#include <qstringview.h>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <string>
#include "enums.hpp"
#include "nlohmann/json_fwd.hpp"
#include "session.hpp"

namespace survey {
void ServerInteraction::send_request(
    const std::string &url,
    Method method,
    std::function<void(const std::string &)> success,
    std::function<void(const std::string &)> failure,
    bool auth_required,
    const std::string &payload
) {
    QNetworkRequest request((QUrl(QString::fromStdString(url))));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (auth_required) {
        request.setRawHeader(
            "Authorization",
            QByteArray::fromStdString("Bearer " + session().get_access_token())
        );
    }

    QNetworkReply *reply = nullptr;
    switch (method) {
        case Method::GET:
            reply = manager_.get(request);
            break;
        case Method::POST:
            reply = manager_.post(request, QByteArray::fromStdString(payload));
            break;
    }

    connect(reply, &QNetworkReply::finished, [reply, success, failure]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString error_msg = QString("Network error: %1. %2")
                                    .arg(reply->errorString())
                                    .arg(QString(reply->readAll()));
            failure(error_msg.toStdString());
        } else {
            std::string result = reply->readAll().toStdString();
            success(result);
        }
        reply->deleteLater();
    });
}

void ServerInteraction::get_survey(
    int survey_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url =
        "http://127.0.0.1:8080/survey?id=" + std::to_string(survey_id);
    send_request(
        url, Method::GET,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure
    );
}

void ServerInteraction::post_survey(
    const nlohmann::json &survey_data,
    std::function<void()> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/survey";
    send_request(
        url, Method::POST, [success](const std::string &) { success(); },
        failure, true, survey_data.dump()
    );
}

void ServerInteraction::post_answer(
    const nlohmann::json &answer_data,
    std::function<void()> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/answer";
    send_request(
        url, Method::POST, [success](const std::string &) { success(); },
        failure, true, answer_data.dump()
    );
}

void ServerInteraction::get_passed_surveys(
    const std::string &user_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url =
        "http://127.0.0.1:8080/passed-surveys?session-id=" + user_id;
    send_request(
        url, Method::GET,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, true
    );
}

void ServerInteraction::get_created_surveys(
    const std::string &user_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url =
        "http://127.0.0.1:8080/created-surveys?session-id=" + user_id;
    send_request(
        url, Method::GET,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, true
    );
}

void ServerInteraction::get_survey_statistics(
    int survey_id,
    const std::string &file_format,
    std::function<void(const std::string &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/statistics?survey-id=" +
                      std::to_string(survey_id) + "&format=" + file_format;
    send_request(url, Method::GET, success, failure, true);
}

void ServerInteraction::get_survey_results(
    const std::string &user_id,
    int survey_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url =
        "http://127.0.0.1:8080/survey-results?session-id=" + user_id +
        "&survey-id=" + std::to_string(survey_id);
    send_request(
        url, Method::GET,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, true
    );
}

void ServerInteraction::check_answer(
    const nlohmann::json &answer_data,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/check";
    send_request(
        url, Method::POST,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, true, answer_data.dump()
    );
}

void ServerInteraction::post_image(
    const std::string &image_name,
    const QByteArray &image_data,
    std::function<void(const std::string &)> success,
    std::function<void(const std::string &)> failure
) {
    QUrl url("http://127.0.0.1:8080/image");

    QHttpMultiPart *multiPart =
        new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;

    imagePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"image\"; filename=\"%1\"")
                     .arg(QString::fromStdString(image_name)))
    );

    imagePart.setBody(image_data);
    multiPart->append(imagePart);

    QNetworkReply *reply = manager_.post(QNetworkRequest(url), multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, [reply, success, failure]() {
        if (reply->error() != QNetworkReply::NoError) {
            std::string err = reply->readAll().toStdString();
            failure("Image upload error: " + err);
        } else {
            std::string result = reply->readAll().toStdString();
            success(result);
        }
        reply->deleteLater();
    });
}

void ServerInteraction::get_image(
    const std::string &image_oid,
    std::function<void(const std::string &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/image?id=" + image_oid;
    send_request(url, Method::GET, success, failure);
}

void ServerInteraction::request_challenge(
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/challenge";
    send_request(
        url, Method::POST,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, false, "{}"
    );
}

void ServerInteraction::get_challenge_status(
    const std::string &challenge_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url =
        "http://127.0.0.1:8080/api/auth/telegram/challenge/" + challenge_id;
    send_request(
        url, Method::GET,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure
    );
}

void ServerInteraction::complete_auth(
    const std::string &challenge_id,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/complete";
    nlohmann::json challenge = {{"challenge_id", challenge_id}};
    send_request(
        url, Method::POST,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, false, challenge.dump()
    );
}

void ServerInteraction::generate_question(
    const nlohmann::json &section_data,
    SurveyType survey_type,
    BlockType block_type,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) {
    std::string url = "http://127.0.0.1:8080/generate-question";

    std::string user_message =
        "Текущее состояние раздела:\n" + section_data.dump() + '\n';
    switch (survey_type) {
        case SurveyType::Survey:
            user_message += "Создаю опрос.";
            break;
        case SurveyType::Test:
            user_message += "Создаю тест.";
            break;
        case SurveyType::Quiz:
            user_message += "Создаю квиз.";
            break;
    }
    switch (block_type) {
        case BlockType::Text:
            user_message += "Сгенерируй текстовый вопрос.";
            break;
        case BlockType::Single:
            user_message += "Сгенерируй вопрос с единичным выбором ответа.";
            break;
        case BlockType::Multiple:
            user_message += "Сгенерируй вопрос с множественным выбором ответа.";
            break;
    }

    send_request(
        url, Method::POST,
        [success](const std::string &result) {
            success(nlohmann::json::parse(result));
        },
        failure, false, user_message
    );
}
}  // namespace survey
