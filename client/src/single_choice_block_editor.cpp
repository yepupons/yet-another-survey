#include "single_choice_block_editor.hpp"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>
#include <algorithm>
#include "server_interaction.hpp"

namespace survey {
SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    SurveyType type,
    QStringListModel *sections_list,
    QWidget *parent
)
    : BlockEditor(type, parent), sections_list_(sections_list) {
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
        &SingleChoiceBlockEditor::upload_image
    );

    image_preview_ = new QLabel(this);
    layout->addWidget(image_preview_);

    if (type_ == SurveyType::Test) {
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

SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    SurveyType type,
    const nlohmann::json &question_data,
    QStringListModel *sections_list,
    QWidget *parent
)
    : SingleChoiceBlockEditor(type, sections_list, parent) {
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
        correct_answers_->buttons()
            .at(question_data.at("answer").get<int>() - 1)
            ->setChecked(true);
    }
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
    link->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row_layout->addWidget(link);
    links_.push_back(link);

    connect(link_enabling, &QRadioButton::toggled, link, [link](bool checked) {
        link->setEnabled(checked);
    });

    QAbstractButton *correct_button = nullptr;
    if (type_ == SurveyType::Test) {
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

    connect(
        delete_button, &QPushButton::clicked, this,
        [this, row_widget, option, link_enabling, link, correct_button]() {
            if (options_.size() <= 1) {
                return;
            }

            options_.erase(
                std::remove(options_.begin(), options_.end(), option),
                options_.end()
            );
            link_enablings_.erase(
                std::remove(
                    link_enablings_.begin(), link_enablings_.end(),
                    link_enabling
                ),
                link_enablings_.end()
            );
            links_.erase(
                std::remove(links_.begin(), links_.end(), link), links_.end()
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

void SingleChoiceBlockEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) const {
    nlohmann::json block;
    block["type"] = "single";
    block["text"] = question_->text().trimmed().toStdString();

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

    if (type_ == SurveyType::Test) {
        int answer_index = 0;
        if (auto *checked = correct_answers_->checkedButton()) {
            const auto buttons = correct_answers_->buttons();
            const auto answer_it =
                std::find(buttons.begin(), buttons.end(), checked);
            if (answer_it != buttons.end()) {
                answer_index =
                    static_cast<int>(std::distance(buttons.begin(), answer_it)
                    ) +
                    1;
            }
        }
        block["answer"] = answer_index;
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
