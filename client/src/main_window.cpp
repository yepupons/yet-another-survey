#include "main_window.hpp"
#include <qboxlayout.h>
#include <qlabel.h>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QSettings>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QTimer>
#include <QSizePolicy>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "created_surveys_window.hpp"
#include "enums.hpp"
#include "login_window.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "top_surveys_window.hpp"
#include "session.hpp"
#include "survey_builder_window.hpp"
#include "survey_taking.hpp"
#include "survey_window.hpp"
#include "view_passed_surveys.hpp"

namespace survey {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(tr("ЯЗЬ"));
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

    auto *header_title = new QLabel(tr("yet-another-survey"), header);
    header_title->setObjectName("headerTitle");
    header_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    header_layout->addWidget(header_title);

    auto *header_spacer = new QWidget(header);
    header_spacer->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred
    );
    header_layout->addWidget(header_spacer);

    auto *trending_button = new QPushButton(tr("Trending"), header);
    trending_button->setObjectName("headerNavButton");
    header_layout->addWidget(trending_button);

    QSettings settings("yet-another-survey", "yet-another-survey");
    const QString current_lang = settings.value("language", "ru").toString();
    const QString other_lang = (current_lang == "ru") ? "en" : "ru";

    auto *lang_button = new QPushButton(other_lang.toUpper(), header);
    lang_button->setObjectName("headerNavButton");
    header_layout->addWidget(lang_button);

    connect(lang_button, &QPushButton::clicked, this, [other_lang]() {
        QSettings s("yet-another-survey", "yet-another-survey");
        s.setValue("language", other_lang);
#ifdef __EMSCRIPTEN__
        EM_ASM(location.reload(););
#else
        QApplication::exit(67);
#endif
    });

    auto *profile_button = new QToolButton(header);
    profile_button->setObjectName("profileButton");
    profile_button->setIcon(QIcon(":/icons/profile.svg"));
    profile_button->setIconSize(QSize(24, 24));
    profile_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    profile_button->setFixedSize(44, 44);

    auto *profile_menu = new QMenu(this);
    profile_menu->setObjectName("profileMenu");

    auto *get_created_surveys_button = profile_menu->addAction(tr("Your surveys"));
    auto *get_passed_surveys_button = profile_menu->addAction(tr("Passed surveys"));

    profile_menu->addSeparator();

    auth_action_ = profile_menu->addAction("");
    update_auth_action();

    header_layout->addWidget(profile_button);

    auto *content = new QWidget(central);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    content->setMaximumWidth(720);

    auto *content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(18);

    auto *open_label = new QLabel(tr("Take a survey:"), content);
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

    auto *id_label = new QLabel(tr("Survey ID:"), card);
    id_label->setObjectName("sectionLabel");
    id_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    card_layout->addWidget(id_label);

    auto *input_row = new QGridLayout();
    input_row->setHorizontalSpacing(12);
    input_row->setVerticalSpacing(12);
    input_row->setColumnStretch(0, 1);
    input_row->setColumnStretch(1, 1);

    id_input_ = new QLineEdit(card);
    id_input_->setPlaceholderText(tr("Enter survey id"));
    id_input_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    input_row->addWidget(id_input_, 0, 0);

    open_survey_button_ = new QPushButton(tr("Open Survey"), card);
    open_survey_button_->setObjectName("primaryButton");
    open_survey_button_->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed
    );
    input_row->addWidget(open_survey_button_, 0, 1);

    card_layout->addLayout(input_row);

    auto *or_row = new QHBoxLayout();
    or_row->setContentsMargins(0, 4, 0, 4);
    or_row->setSpacing(12);

    auto *left_line = new QFrame(card);
    left_line->setFrameShape(QFrame::HLine);
    left_line->setFrameShadow(QFrame::Plain);
    left_line->setObjectName("dividerLine");

    auto *or_label = new QLabel(tr("OR"), card);
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

    auto *create_label = new QLabel(tr("Create your own:"), card);
    create_label->setObjectName("sectionLabel");
    create_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    card_layout->addWidget(create_label);

    create_survey_button_ = new QPushButton(tr("Create survey"), card);
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

    auto *stats_row = new QWidget(content);
    stats_row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *stats_layout = new QHBoxLayout(stats_row);
    stats_layout->setContentsMargins(0, 0, 0, 0);
    stats_layout->setSpacing(12);

    auto make_stat_card = [&](const QString &title) -> QLabel * {
        auto *card_widget = new QWidget(stats_row);
        card_widget->setObjectName("statsCard");
        card_widget->setAttribute(Qt::WA_StyledBackground, true);
        card_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        auto *vbox = new QVBoxLayout(card_widget);
        vbox->setContentsMargins(16, 14, 16, 14);
        vbox->setSpacing(4);

        auto *value_label = new QLabel("—", card_widget);
        value_label->setObjectName("statsValue");
        value_label->setAlignment(Qt::AlignHCenter);
        vbox->addWidget(value_label);

        auto *title_label = new QLabel(title, card_widget);
        title_label->setObjectName("statsTitle");
        title_label->setAlignment(Qt::AlignHCenter);
        vbox->addWidget(title_label);

        stats_layout->addWidget(card_widget);
        return value_label;
    };

    surveys_stat_label_ = make_stat_card(tr("Surveys"));
    answers_stat_label_ = make_stat_card(tr("Completions"));
    ratings_stat_label_ = make_stat_card(tr("Ratings"));
    users_stat_label_ = make_stat_card(tr("Users"));

    auto *stats_header = new QLabel(tr("Already in our service:"), content);
    stats_header->setObjectName("sectionLabel");
    stats_header->setAlignment(Qt::AlignHCenter);
    content_layout->addWidget(stats_header);

    content_layout->addWidget(stats_row);

    outer_layout->addWidget(header);
    outer_layout->addWidget(content_wrapper);

    auto *stats_timer = new QTimer(this);
    stats_timer->setInterval(1000);
    connect(stats_timer, &QTimer::timeout, this, &MainWindow::load_global_stats);
    stats_timer->start();
    load_global_stats();

    connect(
        trending_button, &QPushButton::clicked, this,
        &MainWindow::show_trending
    );
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
        profile_button, &QToolButton::clicked, this,
        [profile_button, profile_menu]() {
            profile_menu->popup(
                profile_button->mapToGlobal(QPoint(0, profile_button->height()))
            );
        }
    );
    connect(
        get_created_surveys_button, &QAction::triggered, this,
        &MainWindow::get_created_surveys
    );
    connect(
        get_passed_surveys_button, &QAction::triggered, this,
        &MainWindow::get_passed_surveys
    );
    connect(auth_action_, &QAction::triggered, this, [this]() {
        if (session().is_authenticated()) {
            session().clear_auth();
            update_auth_action();
            show_message_box(
                this, QMessageBox::Information, tr("Info"), tr("You are logged out.")
            );
            return;
        }

        auto *login_window = new LoginWindow(this);
        login_window->setAttribute(Qt::WA_DeleteOnClose);
        connect(
            login_window, &LoginWindow::login_completed, this,
            &MainWindow::update_auth_action
        );
        login_window->showFullScreen();
    });
}

void MainWindow::set_survey_id(QString survey_id) {
    id_input_->setText(survey_id);
}

void MainWindow::update_auth_action() {
    auth_action_->setText(session().is_authenticated() ? tr("Log out") : tr("Log in"));
}

void MainWindow::load_global_stats() {
    server().get_global_stats(
        [this](const nlohmann::json &data) {
            surveys_stat_label_->setText(QString::number(data.value("surveys_count", 0)));
            answers_stat_label_->setText(QString::number(data.value("answers_count", 0)));
            ratings_stat_label_->setText(QString::number(data.value("ratings_count", 0)));
            users_stat_label_->setText(QString::number(data.value("users_count", 0)));
        },
        [](const std::string &err) {
            qWarning() << "[stats]" << QString::fromStdString(err);
        }
    );
}

void MainWindow::open_survey() {
    const std::string id = id_input_->text().trimmed().toStdString();
    if (id.empty()) {
        show_message_box(
            this, QMessageBox::Warning, tr("Error"),
            tr("Please enter a valid survey id.")
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
            this, QMessageBox::Warning, tr("Error"),
            tr("You're already taking the survey")
        );
    }
}

void MainWindow::create_survey() {
    auto *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setMinimumWidth(create_survey_button_->width());
    menu->setStyleSheet(styleSheet());

    auto *survey = menu->addAction(tr("Survey"));
    survey->setData(static_cast<int>(SurveyType::Survey));
    menu->addSeparator();
    auto *test = menu->addAction(tr("Test"));
    test->setData(static_cast<int>(SurveyType::Test));
    menu->addSeparator();
    auto *quiz = menu->addAction(tr("Quiz"));
    quiz->setData(static_cast<int>(SurveyType::Quiz));

    connect(
        menu, &QMenu::triggered, this,
        [this, survey, test](QAction *chosen) {
            if (!chosen) {
                return;
            }
            auto chosen_type = static_cast<SurveyType>(chosen->data().toInt());
            auto *builder = new SurveyBuilderWindow(chosen_type);
            builder->setAttribute(Qt::WA_DeleteOnClose);
            builder->showFullScreen();
            builder->raise();
            builder->activateWindow();
        }
    );
    menu->popup(create_survey_button_->mapToGlobal(
        QPoint(0, create_survey_button_->height())
    ));
}

void MainWindow::get_created_surveys() {
    auto *created_surveys = new CreatedSurveysWindow(this);
    created_surveys->setAttribute(Qt::WA_DeleteOnClose);
    created_surveys->showFullScreen();
}

void MainWindow::show_trending() {
    server().get_surveys_top(
        [this](const nlohmann::json &surveys) {
            auto *window = new TopSurveysWindow(surveys, this);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->showFullScreen();
        },
        [this](const std::string &error) {
            show_message_box(
                this, QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
        }
    );
}

void MainWindow::get_passed_surveys() {
    if (!session().is_authenticated()) {
        show_message_box(
            this, QMessageBox::Warning, tr("Error"),
            tr("Please log in first.")
        );
        return;
    }
    auto *view = new ViewPassedSurveys(this);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->showFullScreen();
}
}  // namespace survey
