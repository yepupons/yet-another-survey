#include "main_window.hpp"
#include <curl/curl.h>
#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "survey_builder_window.hpp"
#include "survey_window.hpp"
#include "server_interaction.hpp"

namespace survey {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("ЯЗЬ");

    auto *central = new QWidget(this);
    auto *central_layout = new QVBoxLayout(central);

    auto *open_label = new QLabel("Take a survey:", central);
    central_layout->addWidget(open_label);

    auto *input_row = new QHBoxLayout();

    auto *id_label = new QLabel("Survey ID:", central);
    input_row->addWidget(id_label);

    id_input_ = new QLineEdit(central);
    id_input_->setPlaceholderText("Enter survey id");
    input_row->addWidget(id_input_);

    open_survey_button_ = new QPushButton("OK", central);
    input_row->addWidget(open_survey_button_);

    central_layout->addLayout(input_row);

    auto *or_label = new QLabel("OR", central);
    central_layout->addWidget(or_label);

    auto *create_label = new QLabel("Create your own:", central);
    central_layout->addWidget(create_label);

    create_survey_button_ = new QPushButton("Create survey", central);
    central_layout->addWidget(create_survey_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

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
}

void MainWindow::open_survey() {
    bool ok;
    int id = id_input_->text().trimmed().toInt(&ok);
    if (!ok || id <= 0) {
        QMessageBox::warning(this, "Error", "Please enter a valid survey id.");
        return;
    }

    nlohmann::json survey_data;
    try {
        survey_data = ServerInteraction::load_survey(id);
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error", e.what());
        return;
    }

    if (!opened_survey_) {
        opened_survey_ = new SurveyWindow(survey_data, this);
        opened_survey_->setAttribute(Qt::WA_DeleteOnClose);
        connect(opened_survey_, &QObject::destroyed, this, [this]() {
            opened_survey_ = nullptr;
        });
        opened_survey_->show();
        opened_survey_->raise();
        opened_survey_->activateWindow();
    } else {
        QMessageBox::warning(this, "Error", "You're already taking the survey");
    }
}

void MainWindow::create_survey() {
    auto *menu = new QMenu(this);

    auto *pollAction = menu->addAction("Survey");
    auto *testAction = menu->addAction("Test");

    QAction *chosen = menu->exec(create_survey_button_->mapToGlobal(
        QPoint(0, create_survey_button_->height())
    ));

    if (!chosen) {
        return;
    }

    if (chosen == pollAction) {
        auto *builder =
            new SurveyBuilderWindow(SurveyBuilderWindow::Mode::Survey, nullptr);
        builder->setAttribute(Qt::WA_DeleteOnClose);
        builder->setWindowFlag(Qt::Window, true);
        builder->show();
        builder->raise();
        builder->activateWindow();
    } else if (chosen == testAction) {
        QMessageBox::information(
            this, "Info", "This mode doesn't availible yet"
        );
    }
}
}  // namespace survey
