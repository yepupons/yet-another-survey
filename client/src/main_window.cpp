#include <curl/curl.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "multiple_choice_question.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"
#include "main_window.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QList>

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}
};

static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

MainWindow::MainWindow(
    const nlohmann::json& in_file,
    nlohmann::json& out_file, 
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

    connect(save_answer_, &QPushButton::clicked, this, &MainWindow::save_answer);
}

void MainWindow::save_answer() {
    for (auto question : questions_) {
        question->save_answer(answers_);
    }
    std::ofstream o("test_answer.json");
    o << std::setw(4) << answers_ << std::endl;
}
}  // namespace survey