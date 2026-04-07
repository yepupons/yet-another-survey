#include "view_passed_surveys.hpp"
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "multiple_choice_block.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "single_choice_block.hpp"
#include "text_block.hpp"

namespace survey {
ViewPassedSurveys::ViewPassedSurveys(
    const QStringList &survey_ids,
    QWidget *parent
)
    : QDialog(parent) {
    setWindowTitle("Passed surveys");

    auto *layout = new QVBoxLayout(this);
    if (survey_ids.isEmpty()) {
        layout->addWidget(new QLabel("No passed surveys yet.", this));
        setLayout(layout);
        return;
    }

    for (const auto &id : survey_ids) {
        auto *button = new QPushButton(id, this);
        layout->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, id]() {
            int survey_id = id.toInt();
            int session_id = session().get_id();
            auto *dialog = new ViewSurveyResults(survey_id, session_id, this);
            dialog->exec();
        });
    }

    setLayout(layout);
}

ViewSurveyResults::ViewSurveyResults(
    int survey_id,
    int session_id,
    QWidget *parent
)
    : QDialog(parent) {
    setWindowTitle("Survey results");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(16);

    try {
        nlohmann::json survey = ServerInteraction::get_survey(survey_id);
        nlohmann::json results =
            ServerInteraction::get_survey_results(session_id, survey_id);

        nlohmann::json answers_sections = nlohmann::json::array();
        if (results.is_array() && !results.empty()) {
            answers_sections = results.at(0);
        }

        auto &sections = survey.at("sections");
        for (int section_index = 0; section_index < sections.size();
             section_index++) {
            auto &section = sections.at(section_index);
            auto *section_title = new QLabel(
                QString::fromStdString(section.value("title", "Section")),
                content
            );
            section_title->setObjectName("titleLabel");
            content_layout->addWidget(section_title);

            const auto &questions = section.at("questions");
            for (int question_index = 0; question_index < questions.size();
                 question_index++) {
                auto &question = questions.at(question_index);
                std::string type = question.at("type");

                Block *block = nullptr;
                if (type == "text") {
                    block = new TextBlock(question, content);
                } else if (type == "multiple") {
                    block = new MultipleChoiceBlock(question, content);
                } else if (type == "single") {
                    block = new SingleChoiceBlock(question, content);
                } else {
                    continue;
                }

                if (answers_sections.is_array() &&
                    section_index < answers_sections.size()) {
                    auto &answers = answers_sections.at(section_index);
                    if (answers.is_array() && question_index < answers.size()) {
                        block->set_answer(answers.at(question_index));
                    }
                }
                block->set_read_only(true);
                content_layout->addWidget(block);
            }
        }
    } catch (const std::exception &e) {
        auto *error = new QLabel(QString("Error: ") + e.what(), content);
        content_layout->addWidget(error);
    }

    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}
}  // namespace survey
