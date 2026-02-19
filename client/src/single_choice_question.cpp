#include "single_choice_question.hpp"
#include <nlohmann/json.hpp>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QString>
#include <QVBoxLayout>
#include "abstract_question.hpp"

namespace survey {
SingleChoiceBlock::SingleChoiceBlock(
    const nlohmann::json &block,
    // std::optional<int> correct_answer,
    QWidget *parent
) : QuestionBlock(parent) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    options_ = new QButtonGroup(this);
    for (const std::string &option : block.at("options")) {
        options_->addButton(new QRadioButton(QString::fromStdString(option), this));
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(question_);
    for (const auto &option : options_->buttons()) {
        layout->addWidget(option);
    }
    setLayout(layout);
}

void SingleChoiceBlock::save_answer(nlohmann::json &answer_data) const {
    answer_data["answers"].push_back(options_->checkedId());
}
}  // namespace survey