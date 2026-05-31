#include "section_editor.hpp"
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <functional>
#include <memory>
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
    QWidget *parent
)
    : QWidget(parent), type_(type), sections_list_(sections_list) {
    setObjectName("questionCard");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

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

    if (type_ == QUIZ) {
        outcomes_model_ = new QStringListModel(this);
        auto *outcomes_label_ = new QLabel("Outcomes", this);
        outcomes_label_->setObjectName("sectionLabel");
        layout->addWidget(outcomes_label_);

        outcomes_layout_ = new QVBoxLayout();
        layout->addLayout(outcomes_layout_);

        auto *add_outcome_button = new QPushButton("Add outcome", this);
        add_outcome_button->setObjectName("secondaryButton");
        layout->addWidget(add_outcome_button);

        add_outcome();
        
        connect(add_outcome_button, &QPushButton::clicked, this,
            &SectionEditor::add_outcome);

        auto *divider = new QFrame(this);
        divider->setFrameShape(QFrame::HLine);
        divider->setFrameShadow(QFrame::Plain);
        divider->setObjectName("dividerLine");
        layout->addWidget(divider);
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

void SectionEditor::add_outcome() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *outcome_edit = new QLineEdit(row_widget);
    outcome_edit->setPlaceholderText("Write outcome here");
    row_layout->addWidget(outcome_edit);
    outcomes_.push_back(outcome_edit);

    connect(outcome_edit, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList list;
        for (auto *edit : outcomes_) {
            list.append(edit->text().trimmed());
        }
        outcomes_model_->setStringList(list);
    });

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    connect(delete_button, &QPushButton::clicked, this,
        [this, row_widget, outcome_edit]() {
            if (outcomes_.size() <= 1) {
                return;
            }
            outcomes_.erase(
                std::remove(outcomes_.begin(), outcomes_.end(), outcome_edit),
                outcomes_.end()
            );
            QStringList list;
            for (auto *edit : outcomes_) {
                list.append(edit->text().trimmed());
            }
            outcomes_model_->setStringList(list);
            outcomes_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );

    outcomes_layout_->addWidget(row_widget);
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

        connect(
            menu, &QMenu::triggered, this,
            [this, single, multiple, text](QAction *chosen) {
                if (!chosen) {
                    return;
                }

                BlockEditor *block = nullptr;
                if (chosen == single) {
                    block =
                        new SingleChoiceBlockEditor(type_, sections_list_, this);
                } else if (chosen == multiple) {
                    block = new MultipleChoiceBlockEditor(type_, this);
                } else if (chosen == text) {
                    block = new TextBlockEditor(type_, this);
                }

                if (!block) {
                    return;
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
                connect(block, &BlockEditor::move_up_requested, this, [this, block]() {
                    int idx = questions_.indexOf(block);
                    if (idx <= 0) return;
                    std::swap(questions_[idx], questions_[idx - 1]);
                    questions_layout_->removeWidget(block);
                    questions_layout_->insertWidget(idx - 1, block);
                });
                connect(block, &BlockEditor::move_down_requested, this, [this, block]() {
                    int idx = questions_.indexOf(block);
                    if (idx < 0 || idx >= questions_.size() - 1) return;
                    std::swap(questions_[idx], questions_[idx + 1]);
                    questions_layout_->removeWidget(block);
                    questions_layout_->insertWidget(idx + 1, block);
                });
            }
        );
        menu->popup(
            add_block_button_->mapToGlobal(QPoint(0, add_block_button_->height()))
        );
        return;
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
    connect(block, &BlockEditor::move_up_requested, this, [this, block]() {
        int idx = questions_.indexOf(block);
        if (idx <= 0) return;
        std::swap(questions_[idx], questions_[idx - 1]);
        questions_layout_->removeWidget(block);
        questions_layout_->insertWidget(idx - 1, block);
    });
    connect(block, &BlockEditor::move_down_requested, this, [this, block]() {
        int idx = questions_.indexOf(block);
        if (idx >= questions_.size() - 1) return;
        std::swap(questions_[idx], questions_[idx + 1]);
        questions_layout_->removeWidget(block);
        questions_layout_->insertWidget(idx + 1, block);
    });
}

void SectionEditor::build_questions_json(
    bool preview_mode,
    std::shared_ptr<nlohmann::json> section,
    int current_question,
    std::function<void(const nlohmann::json &)> callback
) const {
    questions_[current_question]->to_json(
        preview_mode,
        [=, this](const nlohmann::json &question) {
            (*section)["questions"].push_back(question);
            if (current_question == questions_.size() - 1) {
                callback(*section);
                return;
            }
            build_questions_json(
                preview_mode, section, current_question + 1, callback
            );
        }
    );
}

void SectionEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> callback
) const {
    auto section = std::make_shared<nlohmann::json>();

    if (type_ != QUIZ) {
        (*section)["title"] = title_ ? title_->text().trimmed().toStdString() : "";
        (*section)["next_section_id"] = next_section_
                                            ? next_section_->currentIndex() - 1
                                            : -1;
    }

    (*section)["questions"] = nlohmann::json::array();
    if (questions_.empty()) {
        callback({});
    } else {
        build_questions_json(preview_mode, section, 0, callback);
    }
}
}  // namespace survey
