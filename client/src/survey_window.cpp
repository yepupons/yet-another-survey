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
#include "server_interaction.hpp"
#include "session_id.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}};

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
        {"answer_data",
         {{"survey_id", survey_id_},
          {"answer_id", 67},
          {"user_id", std::to_string(session_id)}}},
        {"answers", {}}};
    for (auto question : questions_) {
        question->save_answer(answers);
    }
    // TODO: MAKE COOLDOWN, OUR SERVER CAN BE DDOSED BY THIS BUTTON
    try {
        ServerInteraction::post_answers(answers, survey_id_);
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error", e.what());
        return;
    }

    QMessageBox::information(
        this, "Saved", "Your answers have been successfully saved."
    );
}
}  // namespace survey
