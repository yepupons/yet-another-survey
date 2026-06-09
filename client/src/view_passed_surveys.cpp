#include "view_passed_surveys.hpp"
#include <qnamespace.h>
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
#include "clickable_card.hpp"

namespace survey {
ViewPassedSurveys::ViewPassedSurveys(QWidget *parent) : QDialog(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(make_back_header(this, this));

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(0, 24, 0, 24);
    content_layout->setSpacing(16);

    auto *title_card = new QWidget(content);
    title_card->setObjectName("questionCard");
    title_card->setFixedWidth(720);

    auto *title_layout = new QVBoxLayout(title_card);
    title_layout->setContentsMargins(24, 24, 24, 24);
    title_layout->setSpacing(8);

    auto *title_label = new QLabel(tr("Passed surveys"), title_card);
    title_label->setObjectName("titleLabel");
    title_layout->addWidget(title_label);

    auto *subtitle_label =
        new QLabel(tr("Select a survey to view your answers."), title_card);
    subtitle_label->setObjectName("subtitleLabel");
    title_layout->addWidget(subtitle_label);

    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);

    server().get_passed_surveys(
        session().get_id(),
        [=, this](const nlohmann::json &surveys) {
            if (surveys.empty()) {
                auto *empty_card = new QWidget(content);
                empty_card->setObjectName("questionCard");
                empty_card->setFixedWidth(720);
                auto *empty_layout = new QVBoxLayout(empty_card);
                empty_layout->setContentsMargins(24, 24, 24, 24);
                auto *empty_label =
                    new QLabel(tr("No passed surveys yet."), empty_card);
                empty_label->setObjectName("subtitleLabel");
                empty_layout->addWidget(empty_label);
                content_layout->addWidget(empty_card, 0, Qt::AlignHCenter);
                return;
            }

            for (auto iter = surveys.rbegin(); iter != surveys.rend(); iter = std::next(iter)) {
                const auto &item = *iter;
                const std::string survey_id =
                    item.at("survey_id").get<std::string>();
                const std::string title =
                    item.at("survey_title").get<std::string>();
                const std::string description =
                    item.at("survey_description").get<std::string>();
                const std::string answer_id =
                    item.at("answer_id").get<std::string>();
                const std::string completed_at =
                    item.at("completed_at").get<std::string>();
                const std::string user_rate =
                    item.at("user_rate").get<std::string>();
                const bool already_rated = !user_rate.empty();

                auto *row = new QWidget(content);
                row->setFixedWidth(720);
                auto *row_layout = new QHBoxLayout(row);
                row_layout->setContentsMargins(0, 0, 0, 0);
                row_layout->setSpacing(8);

                auto *vote_widget = new QWidget(row);
                vote_widget->setFixedWidth(52);
                auto *vote_layout = new QVBoxLayout(vote_widget);
                vote_layout->setContentsMargins(0, 0, 0, 0);
                vote_layout->setSpacing(4);
                vote_layout->setAlignment(Qt::AlignCenter);

                auto *like_btn = new QPushButton(vote_widget);
                like_btn->setObjectName("likeButton");
                like_btn->setCheckable(true);
                like_btn->setChecked(user_rate == "like");
                like_btn->setDisabled(answer_id.empty() || already_rated);

                auto *dislike_btn = new QPushButton(vote_widget);
                dislike_btn->setObjectName("dislikeButton");
                dislike_btn->setCheckable(true);
                dislike_btn->setChecked(user_rate == "dislike");
                dislike_btn->setDisabled(answer_id.empty() || already_rated);

                vote_layout->addWidget(like_btn);
                vote_layout->addWidget(dislike_btn);

                connect(like_btn, &QPushButton::clicked, this, [=]() {
                    server().post_rate(
                        survey_id, answer_id, true,
                        [like_btn, dislike_btn](const nlohmann::json &) {
                            like_btn->setChecked(true);
                            like_btn->setDisabled(true);
                            dislike_btn->setDisabled(true);
                        },
                        [](const std::string &) {}
                    );
                });
                connect(dislike_btn, &QPushButton::clicked, this, [=]() {
                    server().post_rate(
                        survey_id, answer_id, false,
                        [like_btn, dislike_btn](const nlohmann::json &) {
                            dislike_btn->setChecked(true);
                            like_btn->setDisabled(true);
                            dislike_btn->setDisabled(true);
                        },
                        [](const std::string &) {}
                    );
                });

                auto *card = new ClickableCard(
                    [survey_id, answer_id, this]() {
                        auto *dialog = new ViewSurveyResults(
                            survey_id, answer_id, this
                        );
                        dialog->setAttribute(Qt::WA_DeleteOnClose);
                        dialog->showFullScreen();
                    },
                    row
                );
                card->setObjectName("surveyCard");
                card->setSizePolicy(
                    QSizePolicy::Expanding, QSizePolicy::Preferred
                );

                auto *card_layout = new QVBoxLayout(card);
                card_layout->setContentsMargins(24, 20, 24, 20);
                card_layout->setSpacing(8);

                auto *survey_title =
                    new QLabel(QString::fromStdString(title + " (" + completed_at + ")"), card);
                survey_title->setObjectName("titleLabel");
                survey_title->setWordWrap(true);
                survey_title->setAttribute(
                    Qt::WA_TransparentForMouseEvents
                );
                card_layout->addWidget(survey_title);

                if (!description.empty()) {
                    auto *desc_label = new QLabel(
                        QString::fromStdString(description), card
                    );
                    desc_label->setObjectName("descriptionLabel");
                    desc_label->setWordWrap(true);
                    desc_label->setAttribute(
                        Qt::WA_TransparentForMouseEvents
                    );
                    card_layout->addWidget(desc_label);
                }

                row_layout->addWidget(vote_widget);
                row_layout->addWidget(card);

                content_layout->addWidget(row, 0, Qt::AlignHCenter);
            }
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
    const std::string &answer_id,
    QWidget *parent
)
    : QDialog(parent) {
    setWindowTitle(tr("Survey results"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(make_back_header(this, this));

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
                answer_id,
                [=, this](const nlohmann::json &results) {
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
                            tr("Section %1").arg(section_index + 1),
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
                            if (sections.is_array() &&
                                section_index < sections.size()) {
                                auto &answers =
                                    sections.at(section_index);
                                if (answers.is_array() &&
                                    question_index < answers.size()) {
                                    block->set_answer(answers.at(question_index).at("answer"));
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
