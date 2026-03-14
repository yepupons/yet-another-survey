#include "multiple_choice_block_editor.hpp"
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <vector>

namespace survey {
MultipleChoiceBlockEditor::MultipleChoiceBlockEditor(
    bool is_test,
    QWidget *parent
)
    : BlockEditor(is_test, parent) {
    auto *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Multiple choice", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Write your question here");
    layout->addWidget(question_);

    if (is_test_) {
        layout->addWidget(new QLabel("Mark the correct answer(s)", this));
        correct_answers_ = new QButtonGroup(this);
        correct_answers_->setExclusive(false);
    }

    options_layout_ = new QVBoxLayout();
    layout->addLayout(options_layout_);

    add_option();

    add_option_button_ = new QPushButton("Add option", this);
    layout->addWidget(add_option_button_);

    connect(
        add_option_button_, &QPushButton::clicked, this,
        &MultipleChoiceBlockEditor::add_option
    );

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void MultipleChoiceBlockEditor::add_option() {
    auto *row_layout = new QHBoxLayout();

    auto *option = new QLineEdit(this);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    if (is_test_) {
        auto *correct = new QRadioButton("Correct", this);
        row_layout->addWidget(correct);
        correct_answers_->addButton(
            correct, correct_answers_->buttons().size()
        );
    }

    options_layout_->addLayout(row_layout);
}

nlohmann::json MultipleChoiceBlockEditor::to_json() const {
    nlohmann::json block;
    block["type"] = "multiple";
    block["text"] = question_->text().trimmed().toStdString();
    block["options"] = nlohmann::json::array();

    for (auto *option : options_) {
        const std::string option_text = option->text().trimmed().toStdString();
        block["options"].push_back(option_text);
    }

    block["required"] = required_->isChecked();

    if (is_test_) {
        std::vector<int> answers;
        for (const auto answer : correct_answers_->buttons()) {
            if (answer->isChecked()) {
                answers.push_back(correct_answers_->id(answer) + 1);
            }
        }
        block["answer"] = answers;
    }

    return block;
}
}  // namespace survey
