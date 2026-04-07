#include "section_editor.hpp"
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <nlohmann/json_fwd.hpp>
#include "abstract_block_editor.hpp"
#include "multiple_choice_block_editor.hpp"
#include "single_choice_block_editor.hpp"
#include "text_block_editor.hpp"

namespace survey {
SectionEditor::SectionEditor(
    bool is_test,
    QStringListModel *sections_list,
    QWidget *parent
)
    : QWidget(parent), is_test_(is_test), sections_list_(sections_list) {
    auto *layout = new QVBoxLayout();
    layout->addWidget(new QLabel(
        "Section " + QString::number(sections_list->stringList().size() - 1),
        this
    ));

    title_ = new QLineEdit(this);
    title_->setPlaceholderText("Write section title here");
    layout->addWidget(title_);

    layout->addWidget(new QLabel("Questions", this));

    questions_layout_ = new QVBoxLayout();
    layout->addLayout(questions_layout_);

    auto *bottom_row = new QHBoxLayout();
    layout->addLayout(bottom_row);

    next_section_ = new QComboBox(this);
    next_section_->setModel(sections_list);
    bottom_row->addWidget(next_section_);

    add_block_button_ = new QPushButton("Add question", this);
    bottom_row->addWidget(add_block_button_);

    connect(
        add_block_button_, &QPushButton::clicked, this,
        &SectionEditor::add_block
    );

    setLayout(layout);
}

void SectionEditor::add_block() {
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
        questions_.push_back(
            new SingleChoiceBlockEditor(is_test_, sections_list_, this)
        );
    } else if (chosen == multiple) {
        questions_.push_back(new MultipleChoiceBlockEditor(is_test_, this));
    } else if (chosen == text) {
        questions_.push_back(new TextBlockEditor(is_test_, this));
    }
    questions_layout_->addWidget(questions_.back());
}

nlohmann::json SectionEditor::to_json() const {
    nlohmann::json section;
    section["title"] = title_->text().trimmed().toStdString();

    section["questions"] = nlohmann::json::array();
    for (auto *block : questions_) {
        section["questions"].push_back(block->to_json());
    }

    section["next_section_id"] = next_section_->currentIndex() - 1;

    return section;
}
}  // namespace survey