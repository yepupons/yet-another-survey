#include "survey_builder_window.hpp"
#include <curl/curl.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include "multiple_choice_block_editor.hpp"
#include "single_choice_block_editor.hpp"
#include "text_question_block_editor.hpp"

namespace survey {

SurveyBuilderWindow::SurveyBuilderWindow(Mode mode, QWidget *parent)
    : QWidget(parent), mode_(mode) {
    setWindowTitle(
        mode_ == Mode::Survey ? "Survey Builder (Survey)"
                              : "Survey Builder (Test)"
    );
    resize(800, 600);

    auto *root = new QVBoxLayout(this);

    auto *title = new QLabel("Survey constructor", this);
    root->addWidget(title);

    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);

    content_ = new QWidget(scroll_);
    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setAlignment(Qt::AlignTop);

    scroll_->setWidget(content_);
    root->addWidget(scroll_, 1);

    auto *bottomRow = new QHBoxLayout();
    bottomRow->addStretch();

    addBlockButton_ = new QPushButton("+", this);
    addBlockButton_->setFixedSize(40, 40);
    bottomRow->addWidget(addBlockButton_);

    root->addLayout(bottomRow);

    saveSurveyButton_ = new QPushButton("Save survey", this);
    root->addWidget(saveSurveyButton_);

    connect(
        addBlockButton_, &QPushButton::clicked, this,
        &SurveyBuilderWindow::add_block_menu
    );
    connect(
        saveSurveyButton_, &QPushButton::clicked, this,
        &SurveyBuilderWindow::save_survey
    );
}

void SurveyBuilderWindow::add_block_menu() {
    auto *menu = new QMenu(this);
    auto *single = menu->addAction("Single choice");
    auto *multiple = menu->addAction("Multiple choice");
    auto *text = menu->addAction("Text block");

    QAction *chosen = menu->exec(
        addBlockButton_->mapToGlobal(QPoint(0, addBlockButton_->height()))
    );
    if (!chosen) {
        return;
    }

    if (chosen == single) {
        add_single_choice();
    } else if (chosen == multiple) {
        add_multiple_choice();
    } else if (chosen == text) {
        add_text_block();
    }
}

void SurveyBuilderWindow::add_single_choice() {
    auto *w = new SingleChoiceBlockEditor(content_);
    contentLayout_->addWidget(w);
}

void SurveyBuilderWindow::add_multiple_choice() {
    auto *w = new MultipleChoiceBlockEditor(content_);
    contentLayout_->addWidget(w);
}

void SurveyBuilderWindow::add_text_block() {
    auto *w = new TextBlockEditor(content_);
    contentLayout_->addWidget(w);
}

nlohmann::json SurveyBuilderWindow::build_survey_json(int id) const {
    nlohmann::json j;
    j["survey_data"] = {{"id", id}, {"type", "survey"}};
    j["questions"] = nlohmann::json::array();

    for (int i = 0; i < contentLayout_->count(); ++i) {
        QWidget *w = contentLayout_->itemAt(i)->widget();
        if (!w) {
            continue;
        }

        auto *base = dynamic_cast<BlockEditor *>(w);
        if (!base) {
            continue;
        }

        if (!base->is_saved()) {
            continue;
        }

        j["questions"].push_back(base->to_json());
    }
    return j;
}

int SurveyBuilderWindow::generate_survey_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
    )
                  .count();

    return static_cast<int>(ms % 1000000000);
}

bool save_survey_to_server(const std::string &id, const nlohmann::json &j) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    std::string url = "http://127.0.0.1:8080/registertest?id=" + id;
    std::string payload = j.dump();

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // check curl example for example if u like
    // https://curl.se/libcurl/c/http-post.html
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res == CURLE_OK && http_code >= 200 && http_code < 300;
}

void SurveyBuilderWindow::save_survey() {
    int id = generate_survey_id();
    nlohmann::json j = build_survey_json(id);

    if (!j.contains("questions") || j["questions"].empty()) {
        QMessageBox::warning(
            this, "Error",
            "There are no saved blocks. First, add and save at least one block."
        );
        return;
    }

    if (!save_survey_to_server(std::to_string(id), j)) {
        QMessageBox::warning(this, "Error", "Failed to save survey to server.");
        return;
    }

    QMessageBox::information(
        this, "Saved",
        "Survey has been saved.\nYour ID:\n" + QString::number(id)
    );
}
}  // namespace survey
