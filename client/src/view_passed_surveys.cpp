#include "view_passed_surveys.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "multiple_choice_block.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "single_choice_block.hpp"
#include "text_block.hpp"

namespace survey {
nlohmann::json extract_saved_answer(nlohmann::json &saved_answer) {
    if (saved_answer.is_object() && saved_answer.contains("answer")) {
        return saved_answer.at("answer");
    }
    return saved_answer;
}

ViewPassedSurveys::ViewPassedSurveys(QWidget *parent) : QDialog(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(0, 24, 0, 0);
    content_layout->setSpacing(16);

    auto *title_card = new QWidget(content);
    title_card->setObjectName("questionCard");
    title_card->setFixedWidth(720);
    title_card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *title_layout = new QVBoxLayout(title_card);
    title_layout->setAlignment(Qt::AlignTop);
    title_layout->setContentsMargins(24, 24, 24, 24);
    title_layout->setSpacing(8);

    auto *title_label = new QLabel("Passed surveys", title_card);
    title_label->setObjectName("titleLabel");
    title_layout->addWidget(title_label);

    auto *subtitle_label =
        new QLabel("Select a survey to view your answers.", title_card);
    subtitle_label->setObjectName("subtitleLabel");
    title_layout->addWidget(subtitle_label);

    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

    nlohmann::json surveys_ids;
    try {
        surveys_ids = ServerInteraction::get_passed_surveys(session().get_id());
    } catch (const std::exception &e) {
        show_message_box(this, QMessageBox::Warning, "Error", e.what());
        deleteLater();
        return;
    }
    if (surveys_ids.empty()) {
        auto *surveys_card = new QWidget(content);
        surveys_card->setObjectName("questionCard");
        surveys_card->setFixedWidth(720);
        surveys_card->setSizePolicy(
            QSizePolicy::Expanding, QSizePolicy::Preferred
        );

        auto *surveys_layout = new QVBoxLayout(surveys_card);
        surveys_layout->setContentsMargins(24, 24, 24, 24);
        surveys_layout->setSpacing(12);

        auto *empty_label = new QLabel("No passed surveys yet.", surveys_card);
        empty_label->setObjectName("subtitleLabel");
        surveys_layout->addWidget(empty_label);

        content_layout->addWidget(surveys_card, 0, Qt::AlignHCenter);
        content_layout->addStretch();
        scroll_area->setWidget(content);
        layout->addWidget(scroll_area);
        setLayout(layout);
        return;
    }

    auto *surveys_card = new QWidget(content);
    surveys_card->setObjectName("questionCard");
    surveys_card->setFixedWidth(720);
    surveys_card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *surveys_layout = new QVBoxLayout(surveys_card);
    surveys_layout->setContentsMargins(24, 24, 24, 24);
    surveys_layout->setSpacing(12);

    for (int id : surveys_ids) {
        auto *row_widget = new QWidget(surveys_card);
        auto *row_layout = new QHBoxLayout(row_widget);
        row_layout->setContentsMargins(0, 0, 0, 0);
        row_layout->setSpacing(12);

        QPushButton *button = nullptr;
        try {
            auto title =
                ServerInteraction::get_survey(id).at("title").get<std::string>(
                );
            button = new QPushButton(QString::fromStdString(title), row_widget);
        } catch (const std::exception &e) {
            show_message_box(this, QMessageBox::Warning, "Error", e.what());
            deleteLater();
            return;
        }

        button->setObjectName("primaryButton");
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        row_layout->addWidget(button);
        surveys_layout->addWidget(row_widget);

        connect(button, &QPushButton::clicked, this, [this, id]() {
            int survey_id = id;
            std::string session_id = session().get_id();
            auto *dialog = new ViewSurveyResults(survey_id, session_id, this);
            dialog->showMaximized();
            dialog->exec();
        });
    }

    content_layout->addWidget(surveys_card, 0, Qt::AlignHCenter);
    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}

ViewSurveyResults::ViewSurveyResults(
    int survey_id,
    const std::string &session_id,
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
            auto *title = new QWidget(this);
            title->setObjectName("questionCard");
            title->setFixedWidth(720);
            title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            auto *title_layout = new QVBoxLayout(title);
            title_layout->setAlignment(Qt::AlignTop);
            title_layout->setContentsMargins(24, 24, 24, 24);
            title_layout->setSpacing(16);

            auto *section_number_label = new QLabel(
                "Section " + QString::number(section_index + 1),
                content
            );

            section_number_label->setObjectName("titleLabel");
            title_layout->addWidget(section_number_label);

            auto *survey_title_label = new QLabel(
                QString::fromStdString(section.value("title", "Section")),
                content
            );

            survey_title_label->setObjectName("subtitleLabel");
            title_layout->addWidget(survey_title_label);

            content_layout->addWidget(title, 0, Qt::AlignHCenter);

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
                        block->set_answer(
                            extract_saved_answer(answers.at(question_index))
                        );
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
