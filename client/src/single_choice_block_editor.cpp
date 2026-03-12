#include "single_choice_block_editor.hpp"
#include <QLabel>
#include <QMessageBox>

namespace survey {
SingleChoiceBlockEditor::SingleChoiceBlockEditor(QWidget *parent)
    : BlockEditor(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Single choice", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Question");
    layout->addWidget(question_);

    optionsLayout_ = new QVBoxLayout();
    layout->addLayout(optionsLayout_);

    addOption_ = new QPushButton("+ option", this);
    layout->addWidget(addOption_);

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    save_ = new QPushButton("Save block", this);
    layout->addWidget(save_);

    connect(
        addOption_, &QPushButton::clicked, this,
        &SingleChoiceBlockEditor::on_add_option
    );
    connect(
        save_, &QPushButton::clicked, this, &SingleChoiceBlockEditor::on_save
    );

    on_add_option();

    setStyleSheet(
        "SingleChoiceBlockEditor { border: 1px solid #aaa; border-radius: 8px; "
        "padding: 8px; }"
    );
}

void SingleChoiceBlockEditor::on_add_option() {
    auto *opt = new QLineEdit(this);
    opt->setPlaceholderText("Option text");
    optionsLayout_->addWidget(opt);
    optionEdits_.push_back(opt);
}

void SingleChoiceBlockEditor::on_save() {
    if (question_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Question is required.");
        return;
    }

    int nonEmpty = 0;
    for (auto *e : optionEdits_) {
        if (!e->text().trimmed().isEmpty()) {
            nonEmpty++;
        }
    }
    if (nonEmpty < 1) {
        QMessageBox::warning(
            this, "Error", "Need at least 1 non-empty options."
        );
        return;
    }

    saved_ = true;
    question_->setEnabled(false);
    required_->setEnabled(false);
    for (auto *e : optionEdits_) {
        e->setEnabled(false);
    }
    addOption_->setEnabled(false);
    save_->setEnabled(false);
}

nlohmann::json SingleChoiceBlockEditor::to_json() const {
    nlohmann::json j;
    j["type"] = "single";
    j["text"] = question_->text().trimmed().toStdString();
    j["options"] = nlohmann::json::array();

    for (auto *e : optionEdits_) {
        auto s = e->text().trimmed().toStdString();
        if (!s.empty()) {
            j["options"].push_back(s);
        }
    }
    j["required"] = required_->isChecked();
    return j;
}
}  // namespace survey