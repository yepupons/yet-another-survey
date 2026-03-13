#include "multiple_choice_question.hpp"
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

namespace survey {
MultipleChoiceBlock::MultipleChoiceBlock(
    const nlohmann::json &block,
    // std::optional<int> correct_answer,
    QWidget *parent
)
    : QuestionBlock(parent, block.value("required", false)) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    options_ = new QButtonGroup(this);
    options_->setExclusive(false);
    int id = 0;
    for (const std::string &option : block.at("options")) {
        auto *button = new QCheckBox(QString::fromStdString(option), this);
        options_->addButton(button, ++id);
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
    answer_data.push_back(answer);
}

bool MultipleChoiceBlock::has_answer() const {
    return options_->checkedId() != -1;
}
}  // namespace survey
