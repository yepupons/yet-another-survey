#include "single_choice_question.hpp"
#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

namespace survey {
SingleChoiceBlock::SingleChoiceBlock(
    const nlohmann::json &block,
    // std::optional<int> correct_answer,
    QWidget *parent
)
    : QuestionBlock(parent, block.value("required", false)) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    options_ = new QButtonGroup(this);
    int id = 0;
    for (const std::string &option : block.at("options")) {
        auto *radio_button =
            new QRadioButton(QString::fromStdString(option), this);
        options_->addButton(radio_button, ++id);
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

bool SingleChoiceBlock::has_answer() const {
    if (options_->checkedId() == -1) {
        return false;
    }
    return true;
}
}  // namespace survey