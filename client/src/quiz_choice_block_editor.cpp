#include "quiz_choice_block_editor.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <algorithm>
#include "server_interaction.hpp"

namespace survey {
QuizChoiceBlockEditor::QuizChoiceBlockEditor(
    QStringListModel *outcomes_model,
    QWidget *parent
)
    : BlockEditor(QUIZ, parent), outcomes_model_(outcomes_model) {
    auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Single choice", this);
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
        &QuizChoiceBlockEditor::upload_image
    );

    image_preview_ = new QLabel(this);
    layout->addWidget(image_preview_);

    options_layout_ = new QVBoxLayout();
    layout->addLayout(options_layout_);

    add_option_button_ = new QPushButton("Add option", this);
    add_option_button_->setObjectName("secondaryButton");
    layout->addWidget(add_option_button_);

    add_option();

    connect(
        add_option_button_, &QPushButton::clicked, this,
        &QuizChoiceBlockEditor::add_option
    );

    required_ = new QCheckBox("Required", this);
    required_->setObjectName("requiredToggle");
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void QuizChoiceBlockEditor::add_option() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *option = new QLineEdit(row_widget);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    auto *outcome_selector = new QComboBox(row_widget);
    outcome_selector->setModel(outcomes_model_);
    outcome_selector->setObjectName("secondaryButton");
    outcome_selector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row_layout->addWidget(outcome_selector);
    outcome_selectors_.push_back(outcome_selector);

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    connect(
        delete_button, &QPushButton::clicked, this,
        [this, row_widget, option, outcome_selector]() {
            if (options_.size() <= 1) {
                return;
            }
            options_.erase(
                std::remove(options_.begin(), options_.end(), option),
                options_.end()
            );
            outcome_selectors_.erase(
                std::remove(
                    outcome_selectors_.begin(), outcome_selectors_.end(),
                    outcome_selector
                ),
                outcome_selectors_.end()
            );
            options_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );

    options_layout_->addWidget(row_widget);
}

void QuizChoiceBlockEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> callback
) const {
    nlohmann::json block;
    block["type"] = "single";
    block["text"] = question_->text().trimmed().toStdString();

    block["options"] = nlohmann::json::array();
    for (auto *option : options_) {
        block["options"].push_back(option->text().trimmed().toStdString());
    }

    block["scores"] = nlohmann::json::array();
    for (auto *selector : outcome_selectors_) {
        block["scores"].push_back(selector->currentText().toStdString());
    }

    block["required"] = required_->isChecked();

    if (!preview_mode && !image_data_.isEmpty()) {
        server().post_image(
            image_name_.toStdString(), image_data_,
            [=](const std::string &image_oid) mutable {
                block["image"] = image_oid;
                callback(block);
            },
            [](const std::string &) {}
        );
        return;
    }

    callback(block);
}

}  // namespace survey
