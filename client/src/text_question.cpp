#include "text_question.hpp"
#include <nlohmann/json.hpp>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>
#include "abstract_question.hpp"

namespace survey {
TextBlock::TextBlock(
    const nlohmann::json &block,
    // std::optional<std::string> correct_answer,
    QWidget *parent
) : QuestionBlock(parent) {
    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    answer_ = new QLineEdit("Input your answer here:", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(question_);
    layout->addWidget(answer_);
    setLayout(layout);
}

void TextBlock::save_answer(nlohmann::json &answer_data) const {
    answer_data["answers"].push_back(answer_->text().toStdString());
}
}  // namespace survey
