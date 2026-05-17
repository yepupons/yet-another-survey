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

namespace survey {
LoginWindow::LoginWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Login");
    resize(420, 240);

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto *title = new QLabel("Login", central);
    title->setObjectName("titleLabel");
    layout->addWidget(title);

    status_label_ =
        new QLabel("Authorize with Telegram to link your account.", central);
    status_label_->setObjectName("subtitleLabel");
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);

    login_button_ = new QPushButton("Login with Telegram", central);
    login_button_->setObjectName("primaryButton");
    layout->addWidget(login_button_);

    layout->addStretch();
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
            status_label_->setText("Login link expired.");
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
            "Open Telegram to authorize. This link expires in " +
            QString::number(expires_in_) + " seconds."
        );
    });

    poll_timer_ = new QTimer(this);
    connect(poll_timer_, &QTimer::timeout, this, [this]() {
        try {
            auto status_json = ServerInteraction::get_challenge_status(
                challenge_id_.toStdString()
            );
            std::string status = status_json.at("status").get<std::string>();
            if (status == "pending") {
                return;
            }
            poll_timer_->stop();
            countdown_timer_->stop();
            if (status == "confirmed") {
                status_label_->setText("Telegram login confirmed.");
            } else if (status == "expired") {
                status_label_->setText("Link expired. Please, login again.");
            } else if (status == "used") {
                status_label_->setText("This link was already used.");
            }
        } catch (const std::exception &e) {
            poll_timer_->stop();
            status_label_->setText(
                "Unable to check login status: " +
                QString::fromStdString(e.what())
            );
        }
    });
}

void LoginWindow::start_telegram_login() {
    login_button_->setEnabled(false);
    status_label_->setText("Stand by, requesting auth...");

    try {
        const nlohmann::json challenge_info =
            ServerInteraction::request_challenge();
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

        login_button_->setText("Copy link");
        login_button_->setEnabled(true);
        disconnect(
            login_button_, &QPushButton::clicked, this,
            &LoginWindow::start_telegram_login
        );
        connect(login_button_, &QPushButton::clicked, this, [this]() {
            QApplication::clipboard()->setText(telegram_url_);
            login_button_->setText("Telegram link copied to clipboard.");
        });

    } catch (const std::exception &e) {
        status_label_->setText(
            "Unable to start Telegram login: " +
            QString::fromStdString(e.what())
        );
        login_button_->setEnabled(true);
    }
}
}  // namespace survey
