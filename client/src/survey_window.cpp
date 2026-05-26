#include "survey_window.hpp"
#include <qlabel.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "multiple_choice_block.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "single_choice_block.hpp"
#include "text_block.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}
};

SurveyWindow::SurveyWindow(
    const nlohmann::json &survey_data,
    nlohmann::json &answer_data,
    int section_id,
    bool preview_mode,
    QWidget *parent
)
    : QMainWindow(parent),
      preview_mode_(preview_mode),
      section_data_(survey_data.at("sections").at(section_id)),
      answer_data_(answer_data.at("sections").at(section_id)) {
    const std::string section_title = section_data_.value("title", "Section");
    setWindowTitle(QString::fromStdString(
        "[" + survey_data.at("title").get<std::string>() + "] " +
        section_title
    ));

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *central_layout = new QVBoxLayout(central);
    central_layout->setAlignment(Qt::AlignTop);
    central_layout->setContentsMargins(24, 24, 24, 24);
    central_layout->setSpacing(16);

    auto *title = new QWidget(this);
    title->setObjectName("questionCard");
    title->setFixedWidth(720);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *title_layout = new QVBoxLayout(title);
    title_layout->setAlignment(Qt::AlignTop);
    title_layout->setContentsMargins(24, 24, 24, 24);
    title_layout->setSpacing(16);

    auto *survey_title_label = new QLabel(
        QString::fromStdString(survey_data.at("title").get<std::string>()),
        title
    );
    survey_title_label->setObjectName("titleLabel");
    title_layout->addWidget(survey_title_label);

    auto *section_title_label = new QLabel(
        QString::fromStdString(section_title),
        title
    );
    section_title_label->setObjectName("subtitleLabel");
    title_layout->addWidget(section_title_label);

    central_layout->addWidget(title, 0, Qt::AlignHCenter);

    auto *scroll_area = new QScrollArea(central);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(16);

    for (const auto &block : section_data_.at("questions")) {
        const std::string block_type = block.at("type").get<std::string>();
        switch (COMPARATOR.at(block_type)) {
            case BlockType::Text: {
                questions_.push_back(new TextBlock(block, content));
                break;
            }
            case BlockType::Multiple: {
                questions_.push_back(new MultipleChoiceBlock(block, content));
                break;
            }
            case BlockType::Single: {
                questions_.push_back(new SingleChoiceBlock(block, content));
                break;
            }
        }
        questions_.back()->setObjectName("questionCard");
        questions_.back()->setMaximumWidth(760);
        questions_.back()->setSizePolicy(
            QSizePolicy::Expanding, QSizePolicy::Preferred
        );
        content_layout->addWidget(questions_.back(), 0, Qt::AlignHCenter);
    }
    content_layout->addStretch();
    content->setLayout(content_layout);

    scroll_area->setWidget(content);
    // scroll_area->setWidgetResizable(true);  // Deleted this line as per
    // instructions
    central_layout->addWidget(scroll_area);

    save_answer_button_ = new QPushButton("Save answers", central);
    save_answer_button_->setObjectName("primaryButton");
    central_layout->addWidget(save_answer_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

    if (preview_mode_) {
        save_answer_button_->setText("Next / finish preview");
    }

    connect(
        save_answer_button_, &QPushButton::clicked, this,
        &SurveyWindow::save_answer
    );
}

void SurveyWindow::closeEvent(QCloseEvent *event) {
    if (!preview_mode_ && !answer_saved) {
        emit closed_without_answer();
    }
    event->accept();
}

void SurveyWindow::save_answer() {
    for (auto question : questions_) {
        if (!question->is_valid()) {
            if (preview_mode_) {
                auto box = show_question_box(
                    this, QMessageBox::Question, "Continue?",
                    "Some required answers are missing. Are you sure you want "
                    "to continue?"
                );
            } else {
                show_message_box(
                    this, QMessageBox::Warning, "Error",
                    "Some required answers are missing. Please complete all "
                    "sections."
                );
                return;
            }
        }
    }

    bool all_answered = true;
    for (auto question : questions_) {
        if (!question->has_answer()) {
            all_answered = false;
            break;
        }
    }

    if (!preview_mode_) {
        QString message = all_answered ? "Are you sure you want to continue?"
                                       : "Some answers are missing. Are you "
                                         "sure you want to continue?";
        auto box =
            show_question_box(this, QMessageBox::Question, "Save?", message);

        connect(box, &QMessageBox::finished, this, [this](int want_to_save) {
            if (want_to_save == QMessageBox::No) {
                return;
            }
            for (auto question : questions_) {
                question->save_answer(answer_data_);
            }

            int next_section_id = section_data_.at("next_section_id");
            for (auto question : questions_) {
                if (question->next_section()) {
                    next_section_id = *(question->next_section());
                }
            }

            answer_saved = true;
            emit closed_with_answer(next_section_id);
            deleteLater();
        });
        return;
    }

    for (auto question : questions_) {
        question->save_answer(answer_data_);
    }

    int next_section_id = -1;
    try {
        next_section_id = section_data_.at("next_section_id").get<int>();
    } catch (const nlohmann::json::exception &) {}
    for (auto question : questions_) {
        if (question->next_section()) {
            next_section_id = *(question->next_section());
        }
    }

    answer_saved = true;
    emit closed_with_answer(next_section_id);
    deleteLater();
}
}  // namespace survey
