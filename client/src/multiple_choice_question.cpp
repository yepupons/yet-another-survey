#include "multiple_choice_question.hpp"
#include <nlohmann/json.hpp>
#include <QLabel>
#include <QButtonGroup>
#include <QCheckBox>
#include <QString>
#include <QVBoxLayout>
#include "abstract_question.hpp"
#include <QList>

namespace survey {
MultipleChoiceBlock::MultipleChoiceBlock(
    const nlohmann::json &block,
    // std::optional<int> correct_answer,
    QWidget *parent
) : QuestionBlock(parent) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    options_ = new QButtonGroup(this);
    for (const std::string &option : block.at("options")) {
        options_->addButton(new QCheckBox(QString::fromStdString(option), this));
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(question_);
    for (const auto &option : options_->buttons()) {
        layout->addWidget(option);
    }
    setLayout(layout);
}

void MultipleChoiceBlock::save_answer(nlohmann::json &answer_data) const {
    QList<int> answer;
    for (const auto &option : options_->buttons()) {
        if (option->isChecked()) {
            answer.push_back(options_->id(option));
        }
    }
    answer_data["answers"].push_back(answer);
}
}  // namespace survey
