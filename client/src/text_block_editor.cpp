#include "text_block_editor.hpp"
#include <string>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

namespace survey {
TextBlockEditor::TextBlockEditor(bool is_test, QWidget *parent) : BlockEditor(is_test, parent) {
    auto *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Text", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Write your question here");
    layout->addWidget(question_);

    if (is_test_) {
        layout->addWidget(new QLabel("Correct answers", this));

        correct_answers_layout_ = new QVBoxLayout();
        layout->addLayout(correct_answers_layout_);

        add_correct_answer_button_ = new QPushButton("Add answer", this);
        layout->addWidget(add_correct_answer_button_);

        connect(add_correct_answer_button_, &QPushButton::clicked, this, &TextBlockEditor::add_correct_answer);

        add_correct_answer();
    }

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void TextBlockEditor::add_correct_answer() {
    correct_answers_.push_back(new QLineEdit(this));
    correct_answers_.back()->setPlaceholderText("Write correct answer here");
    correct_answers_layout_->addWidget(correct_answers_.back());
}

nlohmann::json TextBlockEditor::to_json() const {
    nlohmann::json block;
    block["type"] = "text";
    block["text"] = question_->text().trimmed().toStdString();
    block["required"] = required_->isChecked();

    if (is_test_) {
        std::vector<std::string> answers;
        for (QLineEdit *answer: correct_answers_) {
            const std::string answer_text = answer->text().trimmed().toStdString();
            if (!answer_text.empty()) {
                answers.push_back(answer_text);
            }
        }
        block["answer"] = answers;
    }
    return block;
}
}  // namespace survey