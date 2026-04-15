#include "text_block_editor.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <string>

namespace survey {
TextBlockEditor::TextBlockEditor(bool is_test, QWidget *parent)
    : BlockEditor(is_test, parent) {
        auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Text", this);
    title_label->setObjectName("sectionLabel");

    auto *delete_label = new QLabel("Delete", this);
    delete_label->setObjectName("sectionLabel");

    auto *delete_block_button = new QPushButton(this);
    delete_block_button->setObjectName("dangerIconButton");
    delete_block_button->setFixedSize(36, 36);

    header_layout->addWidget(title_label);
    header_layout->addStretch();
    header_layout->addWidget(delete_label);
    header_layout->addWidget(delete_block_button);

    layout->addLayout(header_layout);

    connect(delete_block_button, &QPushButton::clicked, this, [this]() {
        emit remove_requested(this);
    });

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Write your question here");
    layout->addWidget(question_);

    if (is_test_) {
        auto *correct_label = new QLabel("Correct answer(s)", this);
        correct_label->setObjectName("sectionLabel");
        layout->addWidget(correct_label);

        correct_answers_layout_ = new QVBoxLayout();
        layout->addLayout(correct_answers_layout_);

        add_correct_answer_button_ = new QPushButton("Add answer", this);
        layout->addWidget(add_correct_answer_button_);

        connect(
            add_correct_answer_button_, &QPushButton::clicked, this,
            &TextBlockEditor::add_correct_answer
        );

        add_correct_answer();
    }

    required_ = new QCheckBox("Required", this);
    required_->setObjectName("requiredToggle");
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void TextBlockEditor::add_correct_answer() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto answer_edit_ = new QLineEdit(row_widget);
    answer_edit_->setPlaceholderText("Write correct answer here");
    row_layout->addWidget(answer_edit_);

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    correct_answers_.push_back(answer_edit_);
    correct_answers_layout_->addWidget(row_widget);

    connect(delete_button, &QPushButton::clicked, this, [this, row_widget, answer_edit_]() {
        if (correct_answers_.size() <= 1) {
            return;
        }

        correct_answers_.erase(
            std::remove(correct_answers_.begin(), correct_answers_.end(), answer_edit_),
            correct_answers_.end()
        );

        correct_answers_layout_->removeWidget(row_widget);
        row_widget->deleteLater();
    });
}

nlohmann::json TextBlockEditor::to_json() const {
    nlohmann::json block;
    block["type"] = "text";
    block["text"] = question_->text().trimmed().toStdString();
    block["required"] = required_->isChecked();

    if (is_test_) {
        std::vector<std::string> answers;
        for (QLineEdit *answer : correct_answers_) {
            const std::string answer_text =
                answer->text().trimmed().toStdString();
            if (!answer_text.empty()) {
                answers.push_back(answer_text);
            }
        }
        block["answer"] = answers;
    }
    return block;
}
}  // namespace survey