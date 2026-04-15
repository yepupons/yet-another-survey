#include "multiple_choice_block_editor.hpp"
#include "pretty_view.hpp"
#include <QAbstractButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <algorithm>
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
    options_layout_->setSpacing(10);
    layout->addLayout(options_layout_);

    add_option();

    add_option_button_ = new QPushButton("Add option", this);
    layout->addWidget(add_option_button_);

    connect(
        add_option_button_, &QPushButton::clicked, this,
        &MultipleChoiceBlockEditor::add_option
    );

    required_ = new QCheckBox("Required", this);
    required_->setObjectName("requiredToggle");
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void MultipleChoiceBlockEditor::add_option() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *option = new QLineEdit(row_widget);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    QAbstractButton *correct_button = nullptr;
    if (is_test_) {
        auto *correct = new QCheckBox("Correct", row_widget);
        correct->setObjectName("correctOptionToggle");
        row_layout->addWidget(correct);
        correct_answers_->addButton(correct);
        correct_button = correct;
    }

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setToolTip("Delete option");
    delete_button->setCursor(Qt::PointingHandCursor);
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    options_layout_->addWidget(row_widget);

    connect(delete_button, &QPushButton::clicked, this, [this, row_widget, option, correct_button]() {

        options_.erase(
            std::remove(options_.begin(), options_.end(), option),
            options_.end()
        );

        if (correct_button) {
            correct_answers_->removeButton(correct_button);
        }

        options_layout_->removeWidget(row_widget);
        row_widget->deleteLater();
    });
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
        for (int i = 0; i < options_layout_->count(); ++i) {
            auto *item = options_layout_->itemAt(i);
            if (!item || !item->widget()) {
                continue;
            }

            auto *answer =
                item->widget()->findChild<QAbstractButton *>("correctOptionToggle");
            if (answer && answer->isChecked()) {
                answers.push_back(i + 1);
            }
        }
        block["answer"] = answers;
    }

    return block;
}
}  // namespace survey
