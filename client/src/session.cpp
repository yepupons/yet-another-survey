#include "session.hpp"
#include <QSettings>
#include <QString>
#include <string>

namespace survey {

static QSettings session_settings() {
    return QSettings("yet-another-survey", "client");
}

Session::Session() {
    QSettings settings = session_settings();
    id_ = settings.value("auth/user_id").toString().toStdString();
    access_token_ =
        settings.value("auth/access_token").toString().toStdString();
}

std::string Session::get_id() const {
    return id_;
}

std::string Session::get_access_token() const {
    return access_token_;
}

bool Session::is_authenticated() const {
    return !id_.empty() && !access_token_.empty();
}

void Session::set_auth(const std::string &id, const std::string &access_token) {
    id_ = id;
    access_token_ = access_token;
    QSettings settings = session_settings();
    settings.setValue("auth/user_id", QString::fromStdString(id_));
    settings.setValue(
        "auth/access_token", QString::fromStdString(access_token_)
    );
}

void Session::clear_auth() {
    id_.clear();
    access_token_.clear();

    QSettings settings = session_settings();
    settings.remove("auth/user_id");
    settings.remove("auth/access_token");
}

Session &session() {
    static Session session;
    return session;
}
}  // namespace survey
