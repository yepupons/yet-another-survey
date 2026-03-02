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

namespace survey {
// whatever it is, it is needed to write data from curl to string
static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

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

    CURL *curl = curl_easy_init();

    std::string readBuffer;
    std::string request_url =
        "http://127.0.0.1:8080/file?id=" + std::to_string(id);
    curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, survey::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK || http_code != 200 || readBuffer.empty()) {
        QMessageBox::warning(this, "Error", "Failed to load survey data.");
        return;
    }

    nlohmann::json survey_data = nlohmann::json::parse(readBuffer);
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
