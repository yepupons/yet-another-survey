#include "single_choice_block_editor.hpp"
#include <qradiobutton.h>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>

namespace survey {
SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    bool is_test,
    QStringListModel *sections_list,
    QWidget *parent
)
    : BlockEditor(is_test, parent), sections_list_(sections_list) {
    auto *layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Single choice", this));

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Write your question here");
    layout->addWidget(question_);

    if (is_test_) {
        layout->addWidget(new QLabel("Mark the correct answer", this));
        correct_answers_ = new QButtonGroup(this);
    }

    options_layout_ = new QVBoxLayout();
    layout->addLayout(options_layout_);

    add_option_button_ = new QPushButton("Add option", this);
    layout->addWidget(add_option_button_);

    add_option();

    connect(add_option_button_, &QPushButton::clicked, this, &SingleChoiceBlockEditor::add_option);

    required_ = new QCheckBox("Required", this);
    required_->setChecked(true);
    layout->addWidget(required_);

    setLayout(layout);
}

void SingleChoiceBlockEditor::add_option() {
    auto *row_layout = new QHBoxLayout();

    auto *option = new QLineEdit(this);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    auto *link_enabling = new QRadioButton("After answer:");
    link_enabling->setAutoExclusive(false);
    row_layout->addWidget(link_enabling);
    link_enablings_.push_back(link_enabling);

    auto *link = new QComboBox(this);
    link->setModel(sections_list_);
    link->setEnabled(false);
    row_layout->addWidget(link);
    links_.push_back(link);

    connect(link_enabling, &QRadioButton::toggled, link, [link](bool checked) { link->setEnabled(checked); });

    if (is_test_) {
        auto *correct = new QRadioButton("Correct", this);
        row_layout->addWidget(correct);
        correct_answers_ ->addButton(correct, correct_answers_->buttons().size());
    }

    options_layout_->addLayout(row_layout);
}

nlohmann::json SingleChoiceBlockEditor::to_json() const {
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

    if (is_test_) {
        block["answer"] = correct_answers_->checkedId() + 1;
    }

    return block;
}

}  // namespace survey