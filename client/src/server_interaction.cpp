#include "server_interaction.hpp"
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "session.hpp"

namespace survey {
std::string ServerInteraction::send_request(
    const std::string &url,
    Method method,
    bool auth_required,
    const std::string &payload
) {
    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(QString::fromStdString(url))));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (auth_required) {
        request.setRawHeader("Authorization", ("Bearer " + session().get_access_token()).c_str());
    }

    QNetworkReply *reply = nullptr;
    if (method == Method::POST) {
        reply = manager.post(request, QByteArray::fromStdString(payload));
    } else {
        reply = manager.get(request);
    }

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QString error_msg = QString("Network error: %1. %2")
                                .arg(reply->errorString())
                                .arg(QString(reply->readAll()));
        delete reply;
        throw std::runtime_error(error_msg.toStdString());
    }

    std::string result = reply->readAll().toStdString();
    delete reply;
    return result;
}

nlohmann::json ServerInteraction::get_survey(int survey_id) {
    std::string url =
        "http://127.0.0.1:8080/survey?id=" + std::to_string(survey_id);
    return nlohmann::json::parse(send_request(url, Method::GET));
}

void ServerInteraction::post_survey(const nlohmann::json &survey_data) {
    std::string url = "http://127.0.0.1:8080/survey";
    send_request(url, Method::POST, true, survey_data.dump());
}

void ServerInteraction::post_answer(const nlohmann::json &answer_data) {
    std::string url = "http://127.0.0.1:8080/answer";
    send_request(url, Method::POST, true, answer_data.dump());
}

nlohmann::json ServerInteraction::get_passed_surveys(const std::string &user_id) {
    std::string url = "http://127.0.0.1:8080/passed-surveys?session-id=" + user_id;
    return nlohmann::json::parse(send_request(url, Method::GET, true));
}

nlohmann::json ServerInteraction::get_created_surveys(const std::string &user_id) {
    std::string url = "http://127.0.0.1:8080/created-surveys?session-id=" + user_id;
    return nlohmann::json::parse(send_request(url, Method::GET, true));
}

nlohmann::json ServerInteraction::get_survey_statistics(int survey_id) {
    std::string url = "http://127.0.0.1:8080/statistics?survey-id=" +
                      std::to_string(survey_id);
    return nlohmann::json::parse(send_request(url, Method::GET, true));
}

nlohmann::json ServerInteraction::get_survey_results(
    const std::string &user_id,
    int survey_id
) {
    std::string url = "http://127.0.0.1:8080/survey-results?session-id=" +
                      user_id + "&survey-id=" + std::to_string(survey_id);
    return nlohmann::json::parse(send_request(url, Method::GET, true));
}

nlohmann::json ServerInteraction::check_answer(const nlohmann::json &answer_data
) {
    std::string url = "http://127.0.0.1:8080/check";
    return nlohmann::json::parse(
        send_request(url, Method::POST, false, answer_data.dump())
    );
}

std::string ServerInteraction::post_image(const std::string &image_path) {
    QNetworkAccessManager manager;
    QUrl url("http://127.0.0.1:8080/image");

    QHttpMultiPart *multiPart =
        new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;

    imagePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(
            QString("form-data; name=\"image\"; filename=\"%1\"")
                .arg(QFileInfo(QString::fromStdString(image_path)).fileName())
        )
    );

    QFile *file = new QFile(QString::fromStdString(image_path));
    if (!file->open(QIODevice::ReadOnly)) {
        delete multiPart;
        delete file;
        throw std::runtime_error("Cannot open image file for reading");
    }

    imagePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(imagePart);

    QNetworkReply *reply = manager.post(QNetworkRequest(url), multiPart);
    multiPart->setParent(reply);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        std::string err = reply->readAll().toStdString();
        delete reply;
        throw std::runtime_error("Image upload error: " + err);
    }

    std::string result = reply->readAll().toStdString();
    delete reply;
    return result;
}

std::string ServerInteraction::get_image(const std::string &image_oid) {
    std::string url = "http://127.0.0.1:8080/image?id=" + image_oid;
    return send_request(url, Method::GET);
}
nlohmann::json ServerInteraction::request_challenge() {
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/challenge";
    std::string response = send_request(url, Method::POST, false, "{}");
    return nlohmann::json::parse(response);
}

nlohmann::json ServerInteraction::get_challenge_status(const std::string &challenge_id) {
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/challenge/" + challenge_id;
    std::string response = send_request(url, Method::GET);
    return nlohmann::json::parse(response);
}

nlohmann::json ServerInteraction::complete_auth(const std::string &challenge_id) {
    std::string url = "http://127.0.0.1:8080/api/auth/telegram/complete";
    nlohmann::json challenge = {};
    challenge["challenge_id"] = challenge_id;
    std::string response = send_request(url, Method::POST, false, challenge.dump());
    return nlohmann::json::parse(response);
}
}  // namespace survey
