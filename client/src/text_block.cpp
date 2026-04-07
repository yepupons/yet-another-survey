#include "text_block.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>

namespace survey {
TextBlock::TextBlock(
    const nlohmann::json &block,
    // std::optional<std::string> correct_answer,
    QWidget *parent
)
    : Block(parent, block.value("required", false)) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    answer_ = new QLineEdit(this);
    answer_->setPlaceholderText("Input your answer here:");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(question_);
    layout->addWidget(answer_);
    setLayout(layout);
}

void TextBlock::save_answer(nlohmann::json &answer_data) const {
    answer_data.push_back(answer_->text().toStdString());
}

bool TextBlock::has_answer() const {
    return !answer_->text().trimmed().isEmpty();
}
}  // namespace survey
