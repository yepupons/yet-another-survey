#include "login_window.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "server_interaction.hpp"
#include "session.hpp"

namespace survey {
LoginWindow::LoginWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("Login"));
    resize(420, 240);

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *header = new QWidget(central);
    header->setObjectName("topHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setMinimumWidth(500);

    auto *header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(24, 4, 24, 4);
    header_layout->setSpacing(12);

    auto *back_button = new QPushButton("←", header);
    back_button->setObjectName("headerNavButton");
    header_layout->addWidget(back_button);
    header_layout->addStretch();

    connect(back_button, &QPushButton::clicked, this, &LoginWindow::close);

    layout->addWidget(header);

    auto *content_wrapper = new QWidget(central);
    auto *content_layout = new QVBoxLayout(content_wrapper);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(16);

    auto *title = new QLabel(tr("Login"), content_wrapper);
    title->setObjectName("titleLabel");
    content_layout->addWidget(title);

    status_label_ =
        new QLabel(tr("Authorize with Telegram to link your account."), content_wrapper);
    status_label_->setObjectName("subtitleLabel");
    status_label_->setWordWrap(true);
    content_layout->addWidget(status_label_);

    login_button_ = new QPushButton(tr("Login with Telegram"), content_wrapper);
    login_button_->setObjectName("primaryButton");
    content_layout->addWidget(login_button_);

    content_layout->addStretch();
    layout->addWidget(content_wrapper);
    setCentralWidget(central);

    connect(
        login_button_, &QPushButton::clicked, this,
        &LoginWindow::start_telegram_login
    );

    countdown_timer_ = new QTimer(this);
    connect(countdown_timer_, &QTimer::timeout, this, [this]() {
        --expires_in_;
        if (expires_in_ <= 0) {
            countdown_timer_->stop();
            poll_timer_->stop();
            status_label_->setText(tr("Login link expired."));
            login_button_->setText("Login with Telegram");
            login_button_->setEnabled(true);
            disconnect(login_button_, nullptr, this, nullptr);
            connect(
                login_button_, &QPushButton::clicked, this,
                &LoginWindow::start_telegram_login
            );
            return;
        }

        status_label_->setText(
            tr("Open Telegram to authorize. This link expires in %1 seconds.").arg(expires_in_)
        );
    });

    poll_timer_ = new QTimer(this);
    connect(poll_timer_, &QTimer::timeout, this, [this]() {
        server().get_challenge_status(
            challenge_id_.toStdString(),
            [=, this](const nlohmann::json &status_json) {
                std::string status =
                    status_json.at("status").get<std::string>();
                if (status == "pending") {
                    return;
                }
                poll_timer_->stop();
                countdown_timer_->stop();
                if (status == "confirmed") {
                    server().complete_auth(
                        challenge_id_.toStdString(),
                        [=, this](const nlohmann::json &data) {
                            session().set_auth(
                                data.at("user").at("id").get<std::string>(),
                                data.at("access_token").get<std::string>()
                            );
                            status_label_->setText(tr("Telegram login confirmed."));
                            emit login_completed();
                        },
                        [=, this](const std::string &error) {
                            poll_timer_->stop();
                            status_label_->setText(
                                "Unable to complete authentication: " +
                                QString::fromStdString(error)
                            );
                        }
                    );
                } else if (status == "expired") {
                    status_label_->setText(tr("Link expired. Please, login again."));
                } else if (status == "used") {
                    status_label_->setText(tr("This link was already used."));
                }
            },
            [=, this](const std::string &error) {
                poll_timer_->stop();
                status_label_->setText(
                    "Unable to check login status: " +
                    QString::fromStdString(error)
                );
            }
        );
    });
}

void LoginWindow::start_telegram_login() {
    login_button_->setEnabled(false);
    status_label_->setText(tr("Stand by, requesting auth..."));

    server().request_challenge(
        [=, this](const nlohmann::json &challenge_info) {
            telegram_url_ = QString::fromStdString(
                challenge_info.at("telegram_url").get<std::string>()
            );
            expires_in_ = challenge_info.at("expires_in").get<int>();
            challenge_id_ = QString::fromStdString(
                challenge_info.at("challenge_id").get<std::string>()
            );

            countdown_timer_->start(1000);
            poll_timer_->start(2000);
            QDesktopServices::openUrl(QUrl(telegram_url_));

            login_button_->setText(tr("Copy link"));
            login_button_->setEnabled(true);
            disconnect(
                login_button_, &QPushButton::clicked, this,
                &LoginWindow::start_telegram_login
            );
            connect(login_button_, &QPushButton::clicked, this, [this]() {
                QApplication::clipboard()->setText(telegram_url_);
                login_button_->setText(tr("Telegram link copied to clipboard."));
            });
        },
        [=, this](const std::string &error) {
            status_label_->setText(
                "Unable to start Telegram login: " +
                QString::fromStdString(error)
            );
            login_button_->setEnabled(true);
        }
    );
}
}  // namespace survey
