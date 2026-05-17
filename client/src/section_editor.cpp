#include "section_editor.hpp"
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <nlohmann/json_fwd.hpp>
#include "abstract_block_editor.hpp"
#include "multiple_choice_block_editor.hpp"
#include "single_choice_block_editor.hpp"
#include "text_block_editor.hpp"
#include "quiz_choice_block_editor.hpp"

namespace survey {
SectionEditor::SectionEditor(
    Created_Type type,
    QStringListModel *sections_list,
    QStringListModel *outcomes_model,
    QWidget *parent
)
    : QWidget(parent), type_(type), sections_list_(sections_list),
      outcomes_model_(outcomes_model) {
    auto *layout = new QVBoxLayout();

    if (type_ != QUIZ) {
        layout->addWidget(new QLabel(
            "Section " + QString::number(sections_list->stringList().size() - 1),
            this
        ));

        title_ = new QLineEdit(this);
        title_->setPlaceholderText("Write section title here");
        layout->addWidget(title_);

        layout->addWidget(new QLabel("Questions", this));
    }

    questions_layout_ = new QVBoxLayout();
    layout->addLayout(questions_layout_);

    auto *bottom_row = new QHBoxLayout();
    layout->addLayout(bottom_row);

    if (type_ != QUIZ) {
        next_section_ = new QComboBox(this);
        next_section_->setModel(sections_list);
        bottom_row->addWidget(next_section_);
    }

    add_block_button_ = new QPushButton("Add question", this);
    bottom_row->addWidget(add_block_button_);

    connect(
        add_block_button_, &QPushButton::clicked, this,
        &SectionEditor::add_block
    );

    setLayout(layout);
}

void SectionEditor::add_block() {
    BlockEditor *block = nullptr;

    if (type_ == QUIZ) {
        block = new QuizChoiceBlockEditor(outcomes_model_, this);
    } else {
        auto *menu = new QMenu(this);
        menu->setMinimumWidth(add_block_button_->width());
        menu->setStyleSheet(styleSheet());
        auto *single = menu->addAction("Single Choice");
        auto *multiple = menu->addAction("Multiple Choice");
        auto *text = menu->addAction("Text");

        QAction *chosen = menu->exec(
            add_block_button_->mapToGlobal(QPoint(0, add_block_button_->height()))
        );

        if (!chosen) {
            return;
        }

        if (chosen == single) {
            block = new SingleChoiceBlockEditor(type_, sections_list_, this);
        } else if (chosen == multiple) {
            block = new MultipleChoiceBlockEditor(type_, this);
        } else if (chosen == text) {
            block = new TextBlockEditor(type_, this);
        }
    }

    questions_.push_back(block);
    questions_layout_->addWidget(questions_.back());

    connect(block, &BlockEditor::remove_requested, this, [this, block]() {
        questions_.erase(
            std::remove(questions_.begin(), questions_.end(), block),
            questions_.end()
        );
        questions_layout_->removeWidget(block);
        block->deleteLater();
    });
}

nlohmann::json SectionEditor::to_json(bool preview_mode) const {
    nlohmann::json section;
    section["title"] = title_ ? title_->text().trimmed().toStdString() : "";

    section["questions"] = nlohmann::json::array();
    for (auto *block : questions_) {
        section["questions"].push_back(block->to_json(preview_mode));
    }

    section["next_section_id"] = next_section_
                                     ? next_section_->currentIndex() - 1
                                     : -1;

    return section;
}
}  // namespace survey