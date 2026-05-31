#include "text_block_editor.hpp"
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <string>
#include "server_interaction.hpp"

namespace survey {
TextBlockEditor::TextBlockEditor(SurveyType type, QWidget *parent)
    : BlockEditor(type, parent) {
    auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Text", this);
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
        &TextBlockEditor::upload_image
    );

    image_preview_ = new QLabel(this);
    layout->addWidget(image_preview_);

    if (type_ == SurveyType::Test) {
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

    connect(
        delete_button, &QPushButton::clicked, this,
        [this, row_widget, answer_edit_]() {
            if (correct_answers_.size() <= 1) {
                return;
            }

            correct_answers_.erase(
                std::remove(
                    correct_answers_.begin(), correct_answers_.end(),
                    answer_edit_
                ),
                correct_answers_.end()
            );

            correct_answers_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );
}

void TextBlockEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) const {
    nlohmann::json block;
    block["type"] = "text";
    block["text"] = question_->text().trimmed().toStdString();

    block["required"] = required_->isChecked();

    if (type_ == SurveyType::Test) {
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