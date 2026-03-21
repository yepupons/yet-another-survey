#include "survey_window.hpp"
#include <curl/curl.h>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "multiple_choice_question.hpp"
#include "nlohmann/json_fwd.hpp"
#include "server_interaction.hpp"
#include "session_id.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}};

SurveyWindow::SurveyWindow(
    const nlohmann::json &survey_data,
    nlohmann::json &answer_data,
    int section_id,
    QWidget *parent
)
    : QMainWindow(parent),
      section_data_(survey_data.at("sections").at(section_id)),
      answer_data_(answer_data.at("sections").at(section_id)) {
    setWindowTitle(
        QString::fromStdString(section_data_.at("title").get<std::string>())
    );

    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *central_layout = new QVBoxLayout(central);
    central_layout->setAlignment(Qt::AlignTop);
    central_layout->setContentsMargins(24, 24, 24, 24);
    central_layout->setSpacing(16);

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
        questions_.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        content_layout->addWidget(questions_.back(), 0, Qt::AlignHCenter);
    }
    content_layout->addStretch();
    content->setLayout(content_layout);

    scroll_area->setWidget(content);
    // scroll_area->setWidgetResizable(true);  // Deleted this line as per instructions
    central_layout->addWidget(scroll_area);

    save_answer_button_ = new QPushButton("Save answers", central);
    save_answer_button_->setObjectName("primaryButton");
    central_layout->addWidget(save_answer_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

    connect(
        save_answer_button_, &QPushButton::clicked, this,
        &SurveyWindow::save_answer
    );
}

void SurveyWindow::closeEvent(QCloseEvent *event) {
    if (!answer_saved) {
        emit closed_without_answer();
    }
    event->accept();
}

void SurveyWindow::save_answer() {
    for (auto question : questions_) {
        if (!question->is_valid()) {
            QMessageBox::warning(
                this, "Error",
                "Some answers are missing. Please complete all sections."
            );
            return;
        }
    }

    bool all_answered = true;
    for (auto question : questions_) {
        if (!question->has_answer()) {
            all_answered = false;
            break;
        }
    }

    QString message =
        all_answered
            ? "Are you sure you want to continue?"
            : "Some answers sre missing. Are you sure you want to continue?";
    auto want_to_save = QMessageBox::question(
        this, "Save?", message, QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

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
}
}  // namespace survey
