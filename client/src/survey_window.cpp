#include "survey_window.hpp"
#include <curl/curl.h>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "multiple_choice_question.hpp"
#include "nlohmann/json_fwd.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}};

static bool post_answers(const nlohmann::json &answers, int survey_id) {
    CURL *curl = curl_easy_init();
    std::string url =
        "http://127.0.0.1:8080/response?id=" + std::to_string(survey_id);
    std::string payload = answers.dump();

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

SurveyWindow::SurveyWindow(const nlohmann::json &survey_data, QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("SURVEY");

    auto *central = new QWidget(this);
    auto *central_layout = new QVBoxLayout(central);

    auto *scroll_area = new QScrollArea(central);
    auto *content = new QWidget(scroll_area);
    auto *content_layout = new QVBoxLayout(content);

    survey_id_ = survey_data.at("survey_data").at("id");
    for (const auto &block : survey_data.at("questions")) {
        const std::string block_type = block.at("type").get<std::string>();
        switch (COMPARATOR.at(block_type)) {
            case BlockType::Text: {
                questions_.push_back(new TextBlock(block, central));
                break;
            }
            case BlockType::Multiple: {
                questions_.push_back(new MultipleChoiceBlock(block, central));
                break;
            }
            case BlockType::Single: {
                questions_.push_back(new SingleChoiceBlock(block, central));
                break;
            }
        }
        content_layout->insertWidget(
            content_layout->count() - 1, questions_.back()
        );
    }
    content->setLayout(content_layout);

    scroll_area->setWidget(content);
    scroll_area->setWidgetResizable(true);
    central_layout->addWidget(scroll_area);

    save_answer_button_ = new QPushButton("Save answers", central);
    central_layout->addWidget(save_answer_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

    connect(
        save_answer_button_, &QPushButton::clicked, this,
        &SurveyWindow::save_answer
    );
}

void SurveyWindow::save_answer() {
    for (auto question : questions_) {
        if (!question->is_valid()) {
            QMessageBox::warning(
                this, "Error",
                "Some answers are missing. Please complete all sections."
            );
            return;
        }
    }

    bool all_answered = true;
    for (auto question : questions_) {
        if (!question->has_answer()) {
            all_answered = false;
            break;
        }
    }

    QString message = all_answered
                          ? "Are you sure you want to finish the survey?"
                          : "Some answers sre missing. Are you sure you want "
                            "to finish the survey?";
    auto want_to_save = QMessageBox::question(
        this, "Save?", message, QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (want_to_save == QMessageBox::No) {
        return;
    }

    nlohmann::json answers = {
        {"answer_data", {{"survey_id", survey_id_}, {"answer_id", 67}}},
        {"answers", {}}};
    for (auto question : questions_) {
        question->save_answer(answers);
    }
    // TODO: MAKE COOLDOWN, OUR SERVER CAN BE DDOSED BY THIS BUTTON
    if (!post_answers(answers, survey_id_)) {
        QMessageBox::warning(
            this, "Error", "Failed to send answers to the server."
        );
        return;
    }

    QMessageBox::information(
        this, "Saved", "Your answers have been successfully saved."
    );
}
}  // namespace survey
