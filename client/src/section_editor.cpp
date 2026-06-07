#include "section_editor.hpp"
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <functional>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include "abstract_block_editor.hpp"
#include "enums.hpp"
#include "multiple_choice_block_editor.hpp"
#include "quiz_choice_block_editor.hpp"
#include "server_interaction.hpp"
#include "single_choice_block_editor.hpp"
#include "text_block_editor.hpp"

namespace survey {
SectionEditor::SectionEditor(
    SurveyType type,
    QStringListModel *sections_list,
    QWidget *parent
)
    : QWidget(parent), type_(type), sections_list_(sections_list) {
    setObjectName("questionCard");
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    if (type_ != SurveyType::Quiz) {
        layout->addWidget(new QLabel(
            tr("Section %1").arg(sections_list->stringList().size() - 1),
            this
        ));

        title_ = new QLineEdit(this);
        title_->setPlaceholderText(tr("Write section title here"));
        layout->addWidget(title_);

        layout->addWidget(new QLabel(tr("Questions"), this));
    }

    if (type_ == SurveyType::Quiz) {
        outcomes_model_ = new QStringListModel(this);
        auto *outcomes_label_ = new QLabel(tr("Outcomes"), this);
        outcomes_label_->setObjectName("sectionLabel");
        layout->addWidget(outcomes_label_);

        outcomes_layout_ = new QVBoxLayout();
        layout->addLayout(outcomes_layout_);

        auto *add_outcome_button = new QPushButton(tr("Add outcome"), this);
        add_outcome_button->setObjectName("secondaryButton");
        layout->addWidget(add_outcome_button);

        add_outcome();

        connect(
            add_outcome_button, &QPushButton::clicked, this,
            &SectionEditor::add_outcome
        );

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

    if (type_ != SurveyType::Quiz) {
        next_section_ = new QComboBox(this);
        next_section_->setModel(sections_list);
        bottom_row->addWidget(next_section_);
    }
    
    bottom_row->addStretch();
    
    if (type_ != SurveyType::Quiz) {
        use_AI_ = new QCheckBox(tr("Use AI"), this);
        use_AI_->setObjectName("requiredToggle");
        bottom_row->addWidget(use_AI_);
    }

    add_block_button_ = new QPushButton(tr("Add question"), this);
    bottom_row->addWidget(add_block_button_);

    connect(
        add_block_button_, &QPushButton::clicked, this,
        &SectionEditor::add_block
    );

    setLayout(layout);
}

void SectionEditor::setup_block_actions(BlockEditor *block) {
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
        if (idx <= 0) {
            return;
        }
        std::swap(questions_[idx], questions_[idx - 1]);
        questions_layout_->removeWidget(block);
        questions_layout_->insertWidget(idx - 1, block);
    });
    connect(block, &BlockEditor::move_down_requested, this, [this, block]() {
        int idx = questions_.indexOf(block);
        if (idx < 0 || idx >= questions_.size() - 1) {
            return;
        }
        std::swap(questions_[idx], questions_[idx + 1]);
        questions_layout_->removeWidget(block);
        questions_layout_->insertWidget(idx + 1, block);
    });
}

void SectionEditor::add_outcome() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *outcome_edit = new QLineEdit(row_widget);
    outcome_edit->setPlaceholderText(tr("Write outcome here"));
    row_layout->addWidget(outcome_edit);
    outcomes_.push_back(outcome_edit);

    connect(
        outcome_edit, &QLineEdit::textChanged, this,
        [this](const QString &) {
            QStringList list;
            for (auto *edit : outcomes_) {
                list.append(edit->text().trimmed());
            }
            outcomes_model_->setStringList(list);
        }
    );

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    connect(
        delete_button, &QPushButton::clicked, this,
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

    if (type_ == SurveyType::Quiz) {
        block = new QuizChoiceBlockEditor(outcomes_model_, this);
        questions_.push_back(block);
        questions_layout_->addWidget(questions_.back());
        setup_block_actions(block);
    } else {
        auto *menu = new QMenu(this);
        menu->setMinimumWidth(add_block_button_->width());
        menu->setStyleSheet(styleSheet());
        auto *text = menu->addAction(tr("Text"));
        text->setData(static_cast<int>(BlockType::Text));
        auto *single = menu->addAction(tr("Single Choice"));
        single->setData(static_cast<int>(BlockType::Single));
        auto *multiple = menu->addAction(tr("Multiple Choice"));
        multiple->setData(static_cast<int>(BlockType::Multiple));

        connect(menu, &QMenu::triggered, this, [this](QAction *chosen) {
            if (!chosen) {
                return;
            }
            auto block_type = static_cast<BlockType>(chosen->data().toInt());
            if (!use_AI_->isChecked()) {
                BlockEditor *block = nullptr;
                switch (block_type) {
                    case BlockType::Text:
                        block = new TextBlockEditor(type_, this);
                        break;
                    case BlockType::Single:
                        block = new SingleChoiceBlockEditor(
                            type_, sections_list_, this
                        );
                        break;
                    case BlockType::Multiple:
                        block = new MultipleChoiceBlockEditor(type_, this);
                        break;
                }
                questions_.push_back(block);
                questions_layout_->addWidget(questions_.back());
                setup_block_actions(block);
            } else {
                add_block_button_->setEnabled(false);
                to_json(
                    true,
                    [=, this](const nlohmann::json &section_data) {
                        server().generate_question(
                            section_data, type_, block_type,
                            [=, this](const nlohmann::json &question_data) {
                                BlockEditor *block = nullptr;
                                switch (block_type) {
                                    case BlockType::Text:
                                        block = new TextBlockEditor(
                                            type_, question_data, this
                                        );
                                        break;
                                    case BlockType::Single:
                                        block = new SingleChoiceBlockEditor(
                                            type_, question_data,
                                            sections_list_, this
                                        );
                                        break;
                                    case BlockType::Multiple:
                                        block = new MultipleChoiceBlockEditor(
                                            type_, question_data, this
                                        );
                                        break;
                                }
                                questions_.push_back(block);
                                questions_layout_->addWidget(questions_.back());
                                setup_block_actions(block);
                                add_block_button_->setEnabled(true);
                            },
                            [=, this](const std::string &error) {
                                show_message_box(
                                    this, QMessageBox::Warning, tr("Error"),
                                    QString::fromStdString(error)
                                );
                                add_block_button_->setEnabled(true);
                            }
                        );
                    },
                    [=, this](const std::string &error) {
                        show_message_box(
                            this, QMessageBox::Warning, "Error",
                            QString::fromStdString(error)
                        );
                        add_block_button_->setEnabled(true);
                    }
                );
            }
        });
        menu->popup(add_block_button_->mapToGlobal(
            QPoint(0, add_block_button_->height())
        ));
    }
}

void SectionEditor::build_questions_json(
    bool preview_mode,
    std::shared_ptr<nlohmann::json> section,
    int current_question,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) const {
    questions_[current_question]->to_json(
        preview_mode,
        [=, this](const nlohmann::json &question) {
            (*section)["questions"].push_back(question);
            if (current_question == questions_.size() - 1) {
                success(*section);
                return;
            }
            build_questions_json(
                preview_mode, section, current_question + 1, success, failure
            );
        },
        failure
    );
}

void SectionEditor::to_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> success,
    std::function<void(const std::string &)> failure
) const {
    auto section = std::make_shared<nlohmann::json>();

    if (type_ != SurveyType::Quiz) {
        (*section)["title"] =
            title_ ? title_->text().trimmed().toStdString() : "";
        (*section)["next_section_id"] =
            next_section_ ? next_section_->currentIndex() - 1 : -1;
    } else {
        (*section)["title"] = "Quiz";
        (*section)["next_section_id"] = -1;
    }

    (*section)["questions"] = nlohmann::json::array();
    if (questions_.empty()) {
        success(*section);
    } else {
        build_questions_json(preview_mode, section, 0, success, failure);
    }
}
}  // namespace survey
