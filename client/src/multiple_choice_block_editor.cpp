#include "multiple_choice_block_editor.hpp"
#include <QAbstractButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <algorithm>
#include <vector>
#include "server_interaction.hpp"

namespace survey {
MultipleChoiceBlockEditor::MultipleChoiceBlockEditor(
    Created_Type type,
    QWidget *parent
)
    : BlockEditor(type, parent) {
    auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Multiple choice", this);
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

    upload_image_button_ = new QPushButton("Upload Image", this);
    layout->addWidget(upload_image_button_);

    connect(
        upload_image_button_, &QPushButton::clicked, this,
        &MultipleChoiceBlockEditor::upload_image
    );

    image_preview_ = new QLabel(this);
    layout->addWidget(image_preview_);

    if (type_ == TEST) {
        auto *correct_label = new QLabel("Mark the correct answer(s)", this);
        correct_label->setObjectName("sectionLabel");
        layout->addWidget(correct_label);
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
    if (type_ == TEST) {
        auto *correct = new QCheckBox("Correct", row_widget);
        correct->setObjectName("correctOptionToggle");
        row_layout->addWidget(correct);
        correct_answers_->addButton(correct);
        correct_button = correct;
    }

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    options_layout_->addWidget(row_widget);

    connect(
        delete_button, &QPushButton::clicked, this,
        [this, row_widget, option, correct_button]() {
            if (options_.size() <= 1) {
                return;
            }
            options_.erase(
                std::remove(options_.begin(), options_.end(), option),
                options_.end()
            );
            if (correct_button) {
                correct_answers_->removeButton(correct_button);
            }
            options_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );
}

void MultipleChoiceBlockEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> callback
) const {
    nlohmann::json block;
    block["type"] = "multiple";
    block["text"] = question_->text().trimmed().toStdString();

    block["options"] = nlohmann::json::array();

    for (auto *option : options_) {
        const std::string option_text = option->text().trimmed().toStdString();
        block["options"].push_back(option_text);
    }

    block["required"] = required_->isChecked();

    if (type_ == TEST) {
        std::vector<int> answers;
        for (int i = 0; i < options_layout_->count(); ++i) {
            auto *item = options_layout_->itemAt(i);
            if (!item || !item->widget()) {
                continue;
            }

            auto *answer = item->widget()->findChild<QAbstractButton *>(
                "correctOptionToggle"
            );
            if (answer && answer->isChecked()) {
                answers.push_back(i + 1);
            }
        }
        block["answer"] = answers;
    }

    // NOT WORK

    // if (preview_mode && !image_path_.isEmpty()) {
    //     block["image_path"] = image_path_.toStdString();
    // }

    if (!preview_mode && !image_data_.isEmpty()) {
        server().post_image(
            image_name_.toStdString(), image_data_,
            [=](const std::string &image_oid) mutable {
                block["image"] = image_oid;
                callback(block);
            },
            [](const std::string &) {}
        );
    }
}
}  // namespace survey
