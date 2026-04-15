#include "single_choice_block_editor.hpp"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>
#include <QFileDialog>
#include "server_interaction.hpp"
#include <algorithm>

namespace survey {
SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    bool is_test,
    QStringListModel *sections_list,
    QWidget *parent
)
    : BlockEditor(is_test, parent), sections_list_(sections_list) {
    auto *layout = new QVBoxLayout();

    auto *header_layout = new QHBoxLayout();
    auto *title_label = new QLabel("Single choice", this);
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
        &SingleChoiceBlockEditor::upload_image
    );
    
    image_preview_ = new QLabel(this);
    layout->addWidget(image_preview_);

    if (is_test_) {
        auto *correct_label = new QLabel("Mark the correct answer", this);
        correct_label->setObjectName("sectionLabel");
        layout->addWidget(correct_label);
        correct_answers_ = new QButtonGroup(this);
    }

    options_layout_ = new QVBoxLayout();
    layout->addLayout(options_layout_);

    add_option_button_ = new QPushButton("Add option", this);
    add_option_button_->setObjectName("secondaryButton");
    layout->addWidget(add_option_button_);

    add_option();

    connect(
        add_option_button_, &QPushButton::clicked, this,
        &SingleChoiceBlockEditor::add_option
    );

    required_ = new QCheckBox("Required", this);
    required_->setObjectName("requiredToggle");
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void SingleChoiceBlockEditor::add_option() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *option = new QLineEdit(row_widget);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    auto *link_enabling = new QRadioButton("After answer:");
    link_enabling->setAutoExclusive(false);
    link_enabling->setObjectName("sectionLabel");
    row_layout->addWidget(link_enabling);
    link_enablings_.push_back(link_enabling);

    auto *link = new QComboBox(row_widget);
    link->setModel(sections_list_);
    link->setObjectName("secondaryButton");
    link->setEnabled(false);
    row_layout->addWidget(link);
    links_.push_back(link);

    connect(link_enabling, &QRadioButton::toggled, link, [link](bool checked) {
        link->setEnabled(checked);
    });

    QAbstractButton *correct_button = nullptr;
    if (is_test_) {
        auto *correct = new QRadioButton("Correct", row_widget);
        correct->setObjectName("sectionLabel");
        row_layout->addWidget(correct);
        correct_answers_->addButton(correct);
        correct_button = correct;
    }

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    connect(delete_button, &QPushButton::clicked, this,
        [this, row_widget, option, link_enabling, link, correct_button]() {
            if (options_.size() <= 1) {
                return;
            }

            options_.erase(
                std::remove(options_.begin(), options_.end(), option),
                options_.end()
            );
            link_enablings_.erase(
                std::remove(link_enablings_.begin(), link_enablings_.end(), link_enabling),
                link_enablings_.end()
            );
            links_.erase(
                std::remove(links_.begin(), links_.end(), link),
                links_.end()
            );
            if (correct_button) {
                correct_answers_->removeButton(correct_button);
            }
            options_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );

    options_layout_->addWidget(row_widget);
}

nlohmann::json SingleChoiceBlockEditor::to_json() const {
    nlohmann::json block;
    block["type"] = "single";
    block["text"] = question_->text().trimmed().toStdString();
    if (!image_path_.isEmpty()) {
        block["image"] = ServerInteraction::post_image(image_path_.toStdString());
    }

    block["options"] = nlohmann::json::array();
    for (auto *option : options_) {
        const std::string option_text = option->text().trimmed().toStdString();
        block["options"].push_back(option_text);
    }

    block["links"] = nlohmann::json::array();
    for (int i = 0; i < links_.size(); ++i) {
        if (link_enablings_[i]->isChecked()) {
            nlohmann::json link;
            link["condition"] = i + 1;
            link["section_id"] = links_[i]->currentIndex() - 1;
            block["links"].push_back(link);
        }
    }

    block["required"] = required_->isChecked();

    if (is_test_) {
        int answer_index = 0;
        for (int i = 0; i < options_layout_->count(); ++i) {
            auto *item = options_layout_->itemAt(i);
            if (!item || !item->widget()) {
                continue;
            }

            auto *answer = item->widget()->findChild<QRadioButton *>();
            if (answer && answer->text() == "Correct" && answer->isChecked()) {
                answer_index = i + 1;
                break;
            }
        }
        block["answer"] = answer_index;
    }

    return block;
}

}  // namespace survey