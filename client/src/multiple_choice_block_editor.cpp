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
#include "enums.hpp"
#include "server_interaction.hpp"

namespace survey {
MultipleChoiceBlockEditor::MultipleChoiceBlockEditor(
    SurveyType type,
    QWidget *parent
)
    : BlockEditor(type, parent) {
    auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Multiple choice", this);
    title_label->setObjectName("sectionLabel");

    auto *move_up_button = new QPushButton(this);
    move_up_button->setObjectName("moveUpButton");
    move_up_button->setFixedSize(36, 36);

    auto *move_down_button = new QPushButton(this);
    move_down_button->setObjectName("moveDownButton");
    move_down_button->setFixedSize(36, 36);

    auto *delete_label = new QLabel("Delete", this);
    delete_label->setObjectName("sectionLabel");

    auto *delete_block_button = new QPushButton(this);
    delete_block_button->setObjectName("dangerIconButton");
    delete_block_button->setFixedSize(36, 36);

    header_layout->addWidget(title_label);
    header_layout->addStretch();
    header_layout->addWidget(move_up_button);
    header_layout->addWidget(move_down_button);
    header_layout->addWidget(delete_label);
    header_layout->addWidget(delete_block_button);

    layout->addLayout(header_layout);

    connect(move_up_button, &QPushButton::clicked, this, [this]() {
        emit move_up_requested(this);
    });
    connect(move_down_button, &QPushButton::clicked, this, [this]() {
        emit move_down_requested(this);
    });
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

    if (type_ == SurveyType::Test) {
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

MultipleChoiceBlockEditor::MultipleChoiceBlockEditor(
    SurveyType type,
    const nlohmann::json &question_data,
    QWidget *parent
)
    : MultipleChoiceBlockEditor(type, parent) {
    question_->setText(
        QString::fromStdString(question_data.at("text").get<std::string>())
    );

    const auto &options_data = question_data.at("options");
    for (int i = 0; i < options_data.size() - 1; ++i) {
        options_.back()->setText(QString::fromStdString(options_data[i]));
        add_option();
    }
    options_.back()->setText(QString::fromStdString(options_data.back()));

    required_->setChecked(question_data.at("required").get<bool>());
    if (type_ == SurveyType::Test) {
        for (int index : question_data.at("answer")) {
            correct_answers_->buttons().at(index - 1)->setChecked(true);
        }
    }
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
    if (type_ == SurveyType::Test) {
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
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
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

    if (type_ == SurveyType::Test) {
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
                success(block);
            },
            failure
        );
        return;
    }
    success(block);
}
}  // namespace survey
