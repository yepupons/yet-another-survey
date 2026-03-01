#include "text_question_block_editor.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace survey {
TextBlockEditor::TextBlockEditor(QWidget *parent) : BlockEditor(parent) {
    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Text block", this));

    text_ = new QTextEdit(this);
    text_->setPlaceholderText("Write your question here");
    layout->addWidget(text_);

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    save_ = new QPushButton("Save block", this);
    layout->addWidget(save_);

    connect(save_, &QPushButton::clicked, this, &TextBlockEditor::on_save);

    setStyleSheet("TextBlockEditor { border: 1px solid #aaa; border-radius: 8px; padding: 8px; }");
}

void TextBlockEditor::on_save() {
    if (text_->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Text is required.");
        return;
    }
    saved_ = true;
    required_->setEnabled(false);
    text_->setEnabled(false);
    save_->setEnabled(false);
}

nlohmann::json TextBlockEditor::to_json() const {
    nlohmann::json j;
    j["type"] = "text";
    j["text"]  = text_->toPlainText().toStdString();
    j["required"] = required_->isChecked();
    return j;
}
}