#include "single_choice_block_editor.hpp"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace survey {

SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    BuilderMode mode,
    QWidget *parent
)
    : BlockEditor(mode, parent) {
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Single choice", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Question");
    layout->addWidget(question_);

    if (is_test_mode()) {
        auto *correctLabel = new QLabel("Mark the correct answer", this);
        layout->addWidget(correctLabel);

        correctGroup_ = new QButtonGroup(this);
        correctGroup_->setExclusive(true);
    }

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
        save_, &QPushButton::clicked, this,
        &SingleChoiceBlockEditor::on_save
    );

    on_add_option();
    on_add_option();

    setStyleSheet(
        "SingleChoiceBlockEditor { border: 1px solid #aaa; border-radius: 8px; "
        "padding: 8px; }"
    );
}

void SingleChoiceBlockEditor::on_add_option() {
    auto *row = new QWidget(this);
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);

    auto *opt = new QLineEdit(row);
    opt->setPlaceholderText("Option text");
    rowLayout->addWidget(opt);
    optionEdits_.push_back(opt);

    if (is_test_mode()) {
        auto *correct = new QRadioButton("Correct", row);
        rowLayout->addWidget(correct);

        if (correctGroup_ != nullptr) {
            correctGroup_->addButton(correct);
        }
    }

    optionsLayout_->addWidget(row);
}

void SingleChoiceBlockEditor::on_save() {
    if (question_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Question is required.");
        return;
    }

    int nonEmpty = 0;
    for (auto *e : optionEdits_) {
        if (!e->text().trimmed().isEmpty()) {
            ++nonEmpty;
        }
    }

    if (nonEmpty < 2) {
        QMessageBox::warning(
            this, "Error", "Need at least 2 non-empty options."
        );
        return;
    }

    if (is_test_mode()) {
        int correctCount = 0;
        const auto radios = findChildren<QRadioButton *>();
        for (auto *radio : radios) {
            if (radio->isChecked()) {
                ++correctCount;
            }
        }

        if (correctCount != 1) {
            QMessageBox::warning(
                this, "Error", "Choose exactly 1 correct answer."
            );
            return;
        }
    }

    saved_ = true;
    question_->setEnabled(false);
    required_->setEnabled(false);

    for (auto *e : optionEdits_) {
        e->setEnabled(false);
    }

    const auto radios = findChildren<QRadioButton *>();
    for (auto *radio : radios) {
        radio->setEnabled(false);
    }

    addOption_->setEnabled(false);
    save_->setEnabled(false);
}

nlohmann::json SingleChoiceBlockEditor::to_json() const {
    nlohmann::json j;
    j["type"] = "single";
    j["text"] = question_->text().trimmed().toStdString();
    j["options"] = nlohmann::json::array();

    int correctIndex = -1;
    int currentIndex = 0;

    for (auto *e : optionEdits_) {
        const auto s = e->text().trimmed().toStdString();
        if (s.empty()) {
            continue;
        }

        j["options"].push_back(s);

        if (is_test_mode()) {
            auto *row = e->parentWidget();
            if (row != nullptr) {
                auto *radio = row->findChild<QRadioButton *>();
                if (radio != nullptr && radio->isChecked()) {
                    correctIndex = currentIndex;
                }
            }
        }

        ++currentIndex;
    }

    j["required"] = required_->isChecked();

    if (is_test_mode()) {
        j["answer"] = correctIndex;
    }

    return j;
}

}  // namespace survey