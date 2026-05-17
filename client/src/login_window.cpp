#include "login_window.hpp"
#include "server_interaction.hpp"
#include <nlohmann/json.hpp>
#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>

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

    status_label_ = new QLabel("Authorize with Telegram to link your account.", central);
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
}

void LoginWindow::start_telegram_login() {
    login_button_->setEnabled(false);
    status_label_->setText("Stand by, requesting auth...");

    try {
        const nlohmann::json challenge_info = ServerInteraction::request_challenge();
        telegram_url_ = QString::fromStdString(challenge_info.at("telegram_url").get<std::string>());
        const int expires_in = challenge_info.at("expires_in").get<int>();

        QDesktopServices::openUrl(QUrl(telegram_url_));

        status_label_->setText(
            "Open Telegram to authorize. This link expires in " +
            QString::number(expires_in / 60) + " minutes."
        );
        
        login_button_->setText("Copy link");
        login_button_->setEnabled(true);
        disconnect(login_button_, &QPushButton::clicked, this, &LoginWindow::start_telegram_login);
        connect(login_button_, &QPushButton::clicked, this, [this]() {
            QApplication::clipboard()->setText(telegram_url_);
            status_label_->setText("Telegram link copied to clipboard.");
        });

    } catch (const std::exception &e) {
        status_label_->setText(
            "Unable to start Telegram login: " + QString::fromStdString(e.what())
        );
        login_button_->setEnabled(true);
    }
}
}  // namespace survey
