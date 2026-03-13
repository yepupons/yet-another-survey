#include "text_question_block_editor.hpp"
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

namespace survey {
TextBlockEditor::TextBlockEditor(BuilderMode mode, QWidget *parent) : BlockEditor(mode, parent) {
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Text block", this));

    text_ = new QTextEdit(this);
    text_->setPlaceholderText("Write your question here");
    layout->addWidget(text_);

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    if (is_test_mode()) {
        correctAnswersLabel_ = new QLabel("Correct answers", this);
        layout->addWidget(correctAnswersLabel_);

        correctAnswers_ = new QTextEdit(this);
        correctAnswers_->setPlaceholderText("Write one correct answer per line");
        layout->addWidget(correctAnswers_);
    }

    save_ = new QPushButton("Save block", this);
    layout->addWidget(save_);

    connect(save_, &QPushButton::clicked, this, &TextBlockEditor::on_save);

    setStyleSheet(
        "TextBlockEditor { border: 1px solid #aaa; border-radius: 8px; "
        "padding: 8px; }"
    );
}

void TextBlockEditor::on_save() {
    if (text_->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Text is required.");
        return;
    }

    if (is_test_mode()) {
        if (correctAnswers_ == nullptr ||
            correctAnswers_->toPlainText().trimmed().isEmpty()) {
            QMessageBox::warning(
                this, "Error", "At least one correct answer is required."
            );
            return;
        }
    }

    saved_ = true;
    required_->setEnabled(false);
    text_->setEnabled(false);

    if (correctAnswers_ != nullptr) {
        correctAnswers_->setEnabled(false);
    }

    save_->setEnabled(false);
}

nlohmann::json TextBlockEditor::to_json() const {
    nlohmann::json j;
    j["type"] = "text";
    j["text"] = text_->toPlainText().toStdString();
    j["required"] = required_->isChecked();

    if (is_test_mode()) {
        std::vector<std::string> answers;
        const QStringList lines = correctAnswers_->toPlainText().split('\n');

        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                answers.push_back(trimmed.toStdString());
            }
        }

        if (answers.size() == 1) {
            j["answer"] = answers[0];
        } else {
            j["answer"] = answers;
        }
    }
    return j;
}
}  // namespace survey