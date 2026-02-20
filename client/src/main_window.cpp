#include "main_window.hpp"
#include <curl/curl.h>
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

MainWindow::MainWindow(
    const nlohmann::json &in_file,
    nlohmann::json &out_file,
    QWidget *parent
)
    : QMainWindow(parent), answers_(out_file) {
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    for (const auto &block : in_file.at("questions")) {
        const std::string block_type = block.at("type").get<std::string>();
        switch (survey::COMPARATOR.at(block_type)) {
            case survey::BlockType::Text: {
                questions_.push_back(new TextBlock(block, central));
                break;
            }
            case survey::BlockType::Multiple: {
                questions_.push_back(new MultipleChoiceBlock(block, central));
                break;
            }
            case survey::BlockType::Single: {
                questions_.push_back(new SingleChoiceBlock(block, central));
                break;
            }
        }
        layout->addWidget(questions_.back());
    }

    save_answer_ = new QPushButton("Save answers", central);
    layout->addWidget(save_answer_);

    central->setLayout(layout);
    setCentralWidget(central);

    connect(
        save_answer_, &QPushButton::clicked, this, &MainWindow::save_answer
    );
}

void MainWindow::save_answer() {
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