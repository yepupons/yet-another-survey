#include "main_window.hpp"
#include <curl/curl.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "multiple_choice_question.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}};

// whatever it is, it is needed to write data from curl to string
static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

MainWindow::MainWindow(nlohmann::json &out_file, QWidget *parent)
    : QMainWindow(parent), answers_(out_file) {
    central_ = new QWidget(this);
    layout_ = new QVBoxLayout(central_);

    auto *input_row = new QHBoxLayout();
    auto *label = new QLabel("Survey ID:", central_);
    id_input_ = new QLineEdit(central_);
    id_input_->setPlaceholderText("Enter survey id");
    load_button_ = new QPushButton("OK", central_);
    input_row->addWidget(label);
    input_row->addWidget(id_input_);
    input_row->addWidget(load_button_);
    layout_->addLayout(input_row);

    save_answer_ = new QPushButton("Save answers", central_);
    save_answer_->setEnabled(false);
    layout_->addWidget(save_answer_);

    central_->setLayout(layout_);
    setCentralWidget(central_);

    connect(
        load_button_, &QPushButton::clicked, this, &MainWindow::load_survey
    );
    connect(
        save_answer_, &QPushButton::clicked, this, &MainWindow::save_answer
    );
}

void MainWindow::load_survey() {
    bool ok = false;
    int id = id_input_->text().trimmed().toInt(&ok);
    if (!ok || id <= 0) {
        QMessageBox::warning(this, "Error", "Please enter a valid survey id.");
        return;
    }

    requested_id_ = id;

    CURL *curl = curl_easy_init();

    std::string readBuffer;
    std::string request_url =
        "http://127.0.0.1:8080/file?id=" + std::to_string(requested_id_);
    curl_easy_setopt(curl, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, survey::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        QMessageBox::warning(this, "Error", "Failed to load survey data.");
        return;
    }

    nlohmann::json survey_data = nlohmann::json::parse(readBuffer);

    clear_questions();
    build_questions(survey_data);

    answers_ = {
        {"answer_data", {{"survey_id", 0}, {"answer_id", 0}}}, {"answers", {}}};
    survey_id_ = survey_data.at("survey_data").at("id");
    answer_id_ = 67;
    answers_["answer_data"]["survey_id"] = survey_id_;
    answers_["answer_data"]["answer_id"] = answer_id_;

    survey_loaded_ = true;
    save_answer_->setEnabled(true);
    id_input_->setEnabled(false);
    load_button_->setEnabled(false);
}

void MainWindow::build_questions(const nlohmann::json &in_file) {
    for (const auto &block : in_file.at("questions")) {
        const std::string block_type = block.at("type").get<std::string>();
        switch (survey::COMPARATOR.at(block_type)) {
            case survey::BlockType::Text: {
                questions_.push_back(new TextBlock(block, central_));
                break;
            }
            case survey::BlockType::Multiple: {
                questions_.push_back(new MultipleChoiceBlock(block, central_));
                break;
            }
            case survey::BlockType::Single: {
                questions_.push_back(new SingleChoiceBlock(block, central_));
                break;
            }
        }
        layout_->insertWidget(layout_->count() - 1, questions_.back());
    }
}

void MainWindow::clear_questions() {
    for (auto *question : questions_) {
        layout_->removeWidget(question);
        delete question;
    }
    questions_.clear();
}

void MainWindow::save_answer() {
    if (!survey_loaded_) {
        QMessageBox::warning(this, "Error", "Load a survey first.");
        return;
    }

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

    answers_.clear();
    answers_ = {
        {"answer_data", {{"survey_id", survey_id_}, {"answer_id", answer_id_}}},
        {"answers", {}}};
    for (auto question : questions_) {
        question->save_answer(answers_);
    }

    std::ofstream o("survey_answer.json");
    o << std::setw(4) << answers_ << std::endl;
    QMessageBox::information(
        this, "Save", "Your answers have been successfully saved."
    );
}
}  // namespace survey
