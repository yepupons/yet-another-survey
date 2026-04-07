#include "single_choice_block.hpp"
#include <QButtonGroup>
#include <QLabel>
#include <QRadioButton>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>

namespace survey {
SingleChoiceBlock::SingleChoiceBlock(
    const nlohmann::json &block,
    // std::optional<int> correct_answer,
    QWidget *parent
)
    : Block(parent, block.value("required", false)) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    options_ = new QButtonGroup(this);
    if (block.contains("links")) {
        for (const auto &link : block.at("links")) {
            links_[link.at("condition")] = link.at("section_id");
        }
    }

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
    answer_data.push_back(options_->checkedId());
}

std::optional<int> SingleChoiceBlock::next_section() const {
    try {
        return links_.at(options_->checkedId());
    } catch (const std::out_of_range &) {
        return std::nullopt;
    }
}

bool SingleChoiceBlock::has_answer() const {
    return options_->checkedId() != -1;
}
}  // namespace survey