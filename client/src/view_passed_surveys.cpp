#include "view_passed_surveys.hpp"
#include <qnamespace.h>
#include <qobject.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "multiple_choice_block.hpp"
#include "nlohmann/json_fwd.hpp"
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
    auto *header = new QWidget(this);
    header->setObjectName("topHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setMinimumWidth(500);

    auto *header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(24, 4, 24, 4);
    header_layout->setSpacing(12);

    auto *back_button = new QPushButton("←", header);
    back_button->setObjectName("headerNavButton");
    header_layout->addWidget(back_button);
    header_layout->addStretch();

    connect(back_button, &QPushButton::clicked, this, &ViewPassedSurveys::close);

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

    server().get_passed_surveys(
        session().get_id(),
        [=, this](const nlohmann::json &surveys_ids) {
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

                auto *empty_label =
                    new QLabel("No passed surveys yet.", surveys_card);
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
            surveys_card->setSizePolicy(
                QSizePolicy::Expanding, QSizePolicy::Preferred
            );

            auto *surveys_layout = new QVBoxLayout(surveys_card);
            surveys_layout->setContentsMargins(24, 24, 24, 24);
            surveys_layout->setSpacing(12);

            for (const auto &id_json : surveys_ids) {
                const std::string id = id_json.get<std::string>();
                auto *row_widget = new QWidget(surveys_card);
                auto *row_layout = new QHBoxLayout(row_widget);
                row_layout->setContentsMargins(0, 0, 0, 0);
                row_layout->setSpacing(12);

                server().get_survey(
                    id,
                    [=, this](const nlohmann::json &survey_data) {
                        auto title = survey_data.at("title").get<std::string>();
                        auto button = new QPushButton(
                            QString::fromStdString(title), row_widget
                        );
                        button->setObjectName("primaryButton");
                        button->setSizePolicy(
                            QSizePolicy::Expanding, QSizePolicy::Preferred
                        );
                        row_layout->addWidget(button);
                        surveys_layout->addWidget(row_widget);

                        connect(
                            button, &QPushButton::clicked, this,
                            [this, id]() {
                                std::string session_id = session().get_id();
                                auto *dialog = new ViewSurveyResults(
                                    id, session_id, this
                                );
                                dialog->setAttribute(Qt::WA_DeleteOnClose);
                                dialog->showFullScreen();
                            }
                        );
                    },
                    [=, this](const std::string &error) {
                        show_message_box(
                            parentWidget(), QMessageBox::Warning, "Error",
                            QString::fromStdString(error)
                        );
                        deleteLater();
                    }
                );
            }

            content_layout->addWidget(surveys_card, 0, Qt::AlignHCenter);
            content_layout->addStretch();
            scroll_area->setWidget(content);
            layout->addWidget(header);
            layout->addWidget(scroll_area);
            setLayout(layout);
        },
        [=, this](const std::string &error) {
            show_message_box(
                parentWidget(), QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
            deleteLater();
        }
    );
}

ViewSurveyResults::ViewSurveyResults(
    const std::string &survey_id,
    const std::string &session_id,
    QWidget *parent
)
    : QDialog(parent) {
    setWindowTitle("Survey results");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *header = new QWidget(this);
    header->setObjectName("topHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setMinimumWidth(500);

    auto *header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(24, 4, 24, 4);
    header_layout->setSpacing(12);

    auto *back_button = new QPushButton("←", header);
    back_button->setObjectName("headerNavButton");
    header_layout->addWidget(back_button);
    header_layout->addStretch();

    connect(back_button, &QPushButton::clicked, this, &ViewSurveyResults::close);

    layout->addWidget(header);

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(16);

    server().get_survey(
        survey_id,
        [=, this](const nlohmann::json &survey) {
            server().get_survey_results(
                session_id, survey_id,
                [=, this](const nlohmann::json &results) {
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
                        title->setSizePolicy(
                            QSizePolicy::Expanding, QSizePolicy::Preferred
                        );

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
                            QString::fromStdString(
                                section.value("title", "Section")
                            ),
                            content
                        );

                        survey_title_label->setObjectName("subtitleLabel");
                        title_layout->addWidget(survey_title_label);

                        content_layout->addWidget(title, 0, Qt::AlignHCenter);

                        const auto &questions = section.at("questions");
                        for (int question_index = 0;
                             question_index < questions.size();
                             question_index++) {
                            auto &question = questions.at(question_index);
                            std::string type = question.at("type");

                            Block *block = nullptr;
                            if (type == "text") {
                                block = new TextBlock(question, content);
                            } else if (type == "multiple") {
                                block =
                                    new MultipleChoiceBlock(question, content);
                            } else if (type == "single") {
                                block =
                                    new SingleChoiceBlock(question, content);
                            } else {
                                continue;
                            }

                            if (answers_sections.is_array() &&
                                section_index < answers_sections.size()) {
                                auto &answers =
                                    answers_sections.at(section_index);
                                if (answers.is_array() &&
                                    question_index < answers.size()) {
                                    block->set_answer(extract_saved_answer(
                                        answers.at(question_index)
                                    ));
                                }
                            }
                            block->set_read_only(true);
                            content_layout->addWidget(block);
                        }
                    }
                    content_layout->addStretch();
                },
                [=, this](const std::string &error) {
                    auto *error_label = new QLabel(
                        QString::fromStdString("Error: " + error), content
                    );
                    content_layout->addWidget(error_label);
                    content_layout->addStretch();
                }
            );
        },
        [=, this](const std::string &error) {
            auto *error_label =
                new QLabel(QString::fromStdString("Error: " + error), content);
            content_layout->addWidget(error_label);
            content_layout->addStretch();
        }
    );
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}
}  // namespace survey
