#include "main_window.hpp"
#include <curl/curl.h>
#include <QUrl>
#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QToolButton>
#include <QDesktopServices>
#include <nlohmann/json.hpp>
#include "created_surveys_window.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "survey_builder_window.hpp"
#include "survey_taking.hpp"
#include "survey_window.hpp"
#include "view_passed_surveys.hpp"

namespace survey {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("ЯЗЬ");
    resize(1100, 800);
    setMinimumSize(520, 520);

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto *outer_layout = new QVBoxLayout(central);
    outer_layout->setContentsMargins(0, 0, 0, 0);
    outer_layout->setSpacing(0);

    auto *header = new QWidget(central);
    header->setObjectName("topHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setMinimumWidth(500);

    auto *header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(32, 18, 32, 18);
    header_layout->setSpacing(12);

    auto *header_title = new QLabel("yet-another-survey", header);
    header_title->setObjectName("headerTitle");
    header_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    header_layout->addWidget(header_title);

    auto *header_spacer = new QWidget(header);
    header_spacer->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred
    );
    header_layout->addWidget(header_spacer);

    auto *profile_button = new QToolButton(header);
    profile_button->setObjectName("profileButton");
    profile_button->setIcon(QIcon(":/icons/profile.svg"));
    profile_button->setIconSize(QSize(24, 24));
    profile_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    profile_button->setFixedSize(44, 44);
    profile_button->setPopupMode(QToolButton::InstantPopup);

    auto *profile_menu = new QMenu(profile_button);
    profile_menu->setObjectName("profileMenu");
    
    auto *get_session_id_button_ = profile_menu->addAction("Get session ID");
    auto *set_session_id_button_ = profile_menu->addAction("Set Session ID");

    profile_menu->addSeparator();

    auto *get_created_surveys_button_ = profile_menu->addAction("Your surveys");
    auto *get_passed_surveys_button_ = profile_menu->addAction("Passed surveys");

    profile_menu->addSeparator();

    auto *auth_button = profile_menu->addAction("Log in with Telegram");

    profile_button->setMenu(profile_menu);
    header_layout->addWidget(profile_button);

    auto *content = new QWidget(central);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    content->setMaximumWidth(720);

    auto *content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(18);

    auto *open_label = new QLabel("Take a survey:", content);
    open_label->setObjectName("titleLabel");
    open_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    content_layout->addWidget(open_label, 0, Qt::AlignHCenter);

    // auto *auth_button = new QPushButton("Log in with Telegram", content);
    // auth_button->setObjectName("telegramAuthButton");
    // auth_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // auth_button->setMinimumWidth(240);
    // content_layout->addWidget(auth_button, 0, Qt::AlignHCenter);

    auto *card = new QWidget(content);
    card->setObjectName("card");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *card_layout = new QVBoxLayout(card);
    card_layout->setContentsMargins(24, 24, 24, 24);
    card_layout->setSpacing(16);

    auto *id_label = new QLabel("Survey ID:", card);
    id_label->setObjectName("sectionLabel");
    id_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    card_layout->addWidget(id_label);

    auto *input_row = new QGridLayout();
    input_row->setHorizontalSpacing(12);
    input_row->setVerticalSpacing(12);
    input_row->setColumnStretch(0, 1);
    input_row->setColumnStretch(1, 0);

    id_input_ = new QLineEdit(card);
    id_input_->setPlaceholderText("Enter survey id");
    id_input_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    input_row->addWidget(id_input_, 0, 0);

    open_survey_button_ = new QPushButton("Open Survey", card);
    open_survey_button_->setObjectName("primaryButton");
    open_survey_button_->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::Fixed
    );
    open_survey_button_->setMinimumWidth(180);
    input_row->addWidget(open_survey_button_, 0, 1);

    card_layout->addLayout(input_row);

    auto *or_row = new QHBoxLayout();
    or_row->setContentsMargins(0, 4, 0, 4);
    or_row->setSpacing(12);

    auto *left_line = new QFrame(card);
    left_line->setFrameShape(QFrame::HLine);
    left_line->setFrameShadow(QFrame::Plain);
    left_line->setObjectName("dividerLine");

    auto *or_label = new QLabel("OR", card);
    or_label->setObjectName("orLabel");
    or_label->setAlignment(Qt::AlignCenter);

    auto *right_line = new QFrame(card);
    right_line->setFrameShape(QFrame::HLine);
    right_line->setFrameShadow(QFrame::Plain);
    right_line->setObjectName("dividerLine");

    or_row->addWidget(left_line, 1);
    or_row->addWidget(or_label);
    or_row->addWidget(right_line, 1);

    card_layout->addLayout(or_row);

    auto *create_label = new QLabel("Create your own:", card);
    create_label->setObjectName("sectionLabel");
    create_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    card_layout->addWidget(create_label);

    create_survey_button_ = new QPushButton("Create survey", card);
    create_survey_button_->setObjectName("secondaryButton");
    create_survey_button_->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed
    );
    card_layout->addWidget(create_survey_button_);

    auto *content_wrapper = new QWidget(central);
    content_wrapper->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred
    );

    auto *content_wrapper_layout = new QVBoxLayout(content_wrapper);
    content_wrapper_layout->setContentsMargins(24, 24, 24, 24);
    content_wrapper_layout->setSpacing(0);
    content_wrapper_layout->addStretch();
    content_wrapper_layout->addWidget(content, 0, Qt::AlignHCenter);
    content_wrapper_layout->addStretch();

    content_layout->addWidget(card);

    outer_layout->addWidget(header);
    outer_layout->addWidget(content_wrapper);

    connect(
        open_survey_button_, &QPushButton::clicked, this,
        &MainWindow::open_survey
    );
    connect(
        id_input_, &QLineEdit::returnPressed, this, &MainWindow::open_survey
    );
    connect(
        create_survey_button_, &QPushButton::clicked, this,
        &MainWindow::create_survey
    );
    connect(
        set_session_id_button_, &QAction::triggered, this,
        &MainWindow::change_session_id
    );
    connect(
        get_created_surveys_button_, &QAction::triggered, this,
        &MainWindow::get_created_surveys
    );
    connect(
        get_passed_surveys_button_, &QAction::triggered, this,
        &MainWindow::get_passed_surveys
    );
    connect(get_session_id_button_, &QAction::triggered, this, [this]() {
        show_message_box(
            this, QMessageBox::Information, "Info",
            "Your session ID is: " + QString::number(session().get_id())
        );
    });
    connect(auth_button, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(QUrl("https://t.me/yet_another_survey_auth_bot?start=test_login_token"));
    });
}

void MainWindow::open_survey() {
    bool ok;
    int id = id_input_->text().trimmed().toInt(&ok);
    if (!ok || id <= 0) {
        show_message_box(
            this, QMessageBox::Warning, "Error",
            "Please enter a valid survey id."
        );
        return;
    }

    if (!opened_survey_) {
        opened_survey_ = new SurveyTaking(id);
        connect(opened_survey_, &QObject::destroyed, this, [this]() {
            opened_survey_ = nullptr;
        });
    } else {
        show_message_box(
            this, QMessageBox::Warning, "Error",
            "You're already taking the survey"
        );
    }
}

void MainWindow::create_survey() {
    auto *menu = new QMenu(this);
    menu->setMinimumWidth(create_survey_button_->width());
    menu->setStyleSheet(styleSheet());

    auto *survey = menu->addAction("Survey");
    menu->addSeparator();
    auto *test = menu->addAction("Test");

    QAction *chosen = menu->exec(create_survey_button_->mapToGlobal(
        QPoint(0, create_survey_button_->height())
    ));

    if (!chosen) {
        return;
    }
    auto *builder = new SurveyBuilderWindow(chosen == test);
    builder->setAttribute(Qt::WA_DeleteOnClose);
    builder->showMaximized();
    builder->raise();
    builder->activateWindow();
}

void MainWindow::change_session_id() {
        bool ok;
        QString text = QInputDialog::getText(
            this, "Set Session ID", "Enter new session ID:", QLineEdit::Normal,
            QString::number(session().get_id()), &ok
        );

        if (!ok) {
            return;
        }

        bool convert_ok;
        int new_id = text.trimmed().toInt(&convert_ok);
        if (!convert_ok || new_id <= 0) {
            show_message_box(
                this, QMessageBox::Warning, "Error",
                "Please enter a valid session id."
            );
            return;
        }

        session().set_id(new_id);
        show_message_box(
            this, QMessageBox::Information, "Info",
            "Session ID changed to " + QString::number(session().get_id())
        );
}

void MainWindow::get_created_surveys() {
    auto *created_surveys = new CreatedSurveysWindow(this);
    created_surveys->setAttribute(Qt::WA_DeleteOnClose);
    created_surveys->showMaximized();
}

void MainWindow::get_passed_surveys() {
    auto *view = new ViewPassedSurveys(this);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->showMaximized();
}
}  // namespace survey
