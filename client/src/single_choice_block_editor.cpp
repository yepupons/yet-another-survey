#include "single_choice_block_editor.hpp"
#include <qradiobutton.h>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QSignalBlocker>

namespace survey {
SingleChoiceBlockEditor::SingleChoiceBlockEditor(
    bool is_test,
    QStringListModel *sections_list,
    QWidget *parent
)
    : BlockEditor(is_test, parent), sections_list_(sections_list) {
    auto *layout = new QVBoxLayout();
    auto *title_label = new QLabel("Single choice", this);
    title_label->setObjectName("sectionLabel");
    layout->addWidget(title_label);

    question_ = new QLineEdit(this);
    question_->setPlaceholderText("Write your question here");
    layout->addWidget(question_);

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
    auto *row_layout = new QHBoxLayout();

    auto *option = new QLineEdit(this);
    option->setPlaceholderText("Write option text here");
    row_layout->addWidget(option);
    options_.push_back(option);

    auto *link_enabling = new QRadioButton("After answer:");
    link_enabling->setAutoExclusive(false);
    link_enabling->setObjectName("sectionLabel");
    row_layout->addWidget(link_enabling);
    link_enablings_.push_back(link_enabling);

    auto *action_label = new QLabel("Save answer", this);
    action_label->setObjectName("actionValueLabel");
    row_layout->addWidget(action_label);

    auto *link = new QComboBox(this);
    link->setObjectName("secondaryButton");
    link->setVisible(false);
    row_layout->addWidget(link);
    links_.push_back(link);

    auto rebuild_actions = [this, link]() {
        QSignalBlocker blocker(link);
        const QVariant current_action = link->currentData();

        link->clear();
        link->addItem("Save answer", -1);
        link->insertSeparator(1);

        const QStringList sections = sections_list_->stringList();
        for (int i = 0; i < sections.size(); ++i) {
            const QString section_name = sections[i].trimmed();
            if (section_name.compare("Save answer", Qt::CaseInsensitive) == 0 ||
                section_name.compare("Save answers", Qt::CaseInsensitive) ==
                    0) {
                continue;
            }
            link->addItem("Go to section: " + section_name, i);
        }

        const int current_index = link->findData(current_action);
        if (current_index >= 0) {
            link->setCurrentIndex(current_index);
        } else {
            link->setCurrentIndex(link->findData(-1));
        }
    };

    rebuild_actions();
    link->setCurrentIndex(link->findData(-1));
    action_label->setText(link->currentText());

    connect(
        sections_list_, &QAbstractItemModel::dataChanged, this,
        [rebuild_actions](const QModelIndex &, const QModelIndex &, const QList<int> &) {
            rebuild_actions();
        }
    );
    connect(
        sections_list_, &QAbstractItemModel::rowsInserted, this,
        [rebuild_actions](const QModelIndex &, int, int) { rebuild_actions(); }
    );
    connect(
        sections_list_, &QAbstractItemModel::rowsRemoved, this,
        [rebuild_actions](const QModelIndex &, int, int) { rebuild_actions(); }
    );
    connect(
        sections_list_, &QAbstractItemModel::modelReset, this,
        [rebuild_actions]() { rebuild_actions(); }
    );

    connect(
        link_enabling, &QRadioButton::toggled, this,
        [link, action_label](bool checked) {
            link->setVisible(checked);
            action_label->setVisible(!checked);
        }
    );

    connect(
        link, &QComboBox::currentTextChanged, this,
        [action_label](const QString &text) { action_label->setText(text); }
    );

    if (is_test_) {
        auto *correct = new QRadioButton("Correct", this);
        correct->setObjectName("sectionLabel");
        row_layout->addWidget(correct);
        correct_answers_->addButton(
            correct, correct_answers_->buttons().size()
        );
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
            link["section_id"] = links_[i]->currentData().toInt();
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