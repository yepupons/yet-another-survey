#include "multiple_choice_block_editor.hpp"
#include <QLabel>
#include <QMessageBox>

namespace survey {
MultipleChoiceBlockEditor::MultipleChoiceBlockEditor(BuilderMode mode, QWidget *parent)
    : BlockEditor(mode, parent) {
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Multiple choice", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Question");
    layout->addWidget(question_);

    if (is_test_mode()) {
        auto *correctLabel = new QLabel("Mark the correct answer(s)", this);
        layout->addWidget(correctLabel);
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
        &MultipleChoiceBlockEditor::on_add_option
    );
    connect(
        save_, &QPushButton::clicked, this, &MultipleChoiceBlockEditor::on_save
    );

    on_add_option();

    setStyleSheet(
        "SingleChoiceBlockEditor { border: 1px solid #aaa; border-radius: 8px; "
        "padding: 8px; }"
    );
}

void MultipleChoiceBlockEditor::on_add_option() {
    auto *row = new QWidget(this);
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);

    auto *opt = new QLineEdit(this);
    opt->setPlaceholderText("Option text");
    optionsLayout_->addWidget(opt);
    optionEdits_.push_back(opt);

    if (is_test_mode()) {
        auto *correct = new QRadioButton("Correct", row);
        rowLayout->addWidget(correct);
    }

    optionsLayout_->addWidget(row);
}

void MultipleChoiceBlockEditor::on_save() {
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
    if (nonEmpty < 2) {
        QMessageBox::warning(
            this, "Error", "Need at least 2 non-empty options."
        );
        return;
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

nlohmann::json MultipleChoiceBlockEditor::to_json() const {
    nlohmann::json j;
    j["type"] = "multiple";
    j["text"] = question_->text().trimmed().toStdString();
    j["options"] = nlohmann::json::array();

    std::vector<int> correctIndexes;
    int currentIndex = 0;

    for (auto *e : optionEdits_) {
        auto s = e->text().trimmed().toStdString();
        if (s.empty()) {
            continue;
        }

        j["options"].push_back(s);

        if (is_test_mode()) {
            auto *row = e->parentWidget();
            if (row != nullptr) {
                auto *radio = row->findChild<QRadioButton *>();
                if (radio != nullptr && radio->isChecked()) {
                    correctIndexes.push_back(currentIndex);
                }
            }
        }

        ++currentIndex;
    }

    j["required"] = required_->isChecked();

    if (correctIndexes.size() == 1) {
        j["answer"] = correctIndexes[0];
    } else {
        j["answer"] = correctIndexes;
    }

    return j;
}
}  // namespace survey
