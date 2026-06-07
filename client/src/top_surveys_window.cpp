#include "top_surveys_window.hpp"
#include <qboxlayout.h>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "survey_taking.hpp"
#include "clickable_card.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"

namespace survey {

TopSurveysWindow::TopSurveysWindow(
    const nlohmann::json &surveys, QWidget *parent
) : QDialog(parent) {
    setWindowTitle(tr("Trending surveys"));

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
    content_layout->setContentsMargins(0, 24, 0, 24);
    content_layout->setSpacing(16);

    auto *title_card = new QWidget(content);
    title_card->setObjectName("questionCard");
    title_card->setFixedWidth(720);

    auto *title_layout = new QVBoxLayout(title_card);
    title_layout->setContentsMargins(24, 24, 24, 24);
    title_layout->setSpacing(8);

    auto *title_label = new QLabel(tr("Trending surveys"), title_card);
    title_label->setObjectName("titleLabel");
    title_layout->addWidget(title_label);

    auto *subtitle_label = new QLabel(
        tr("Top 10 surveys rated by the community."), title_card
    );
    subtitle_label->setObjectName("subtitleLabel");
    title_layout->addWidget(subtitle_label);

    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

    if (surveys.empty()) {
        auto *empty_card = new QWidget(content);
        empty_card->setObjectName("questionCard");
        empty_card->setFixedWidth(720);
        auto *empty_layout = new QVBoxLayout(empty_card);
        empty_layout->setContentsMargins(24, 24, 24, 24);
        auto *empty_label = new QLabel(tr("No surveys yet."), empty_card);
        empty_label->setObjectName("titleLabel");
        empty_layout->addWidget(empty_label);
        content_layout->addWidget(empty_card, 0, Qt::AlignHCenter);
    }

    for (const auto &survey : surveys) {
        const std::string id = survey.at("id").get<std::string>();
        const std::string title = survey.at("title").get<std::string>();
        const std::string description = survey.at("description").get<std::string>();
        const int likes = survey.value("likes_count", 0);
        const int dislikes = survey.value("dislikes_count", 0);
        const int total = survey.value("ratings_count", 0);
        const int score = survey.value("rating_score", 0);
        const int complitions = survey.value("answers_count", 0);
        const std::string answer_id = survey.value("answer_id", "");
        const std::string user_rate = survey.value("user_rate", "");
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

        if (!answer_id.empty()) {
            connect(like_btn, &QPushButton::clicked, [id, answer_id, like_btn, dislike_btn]() {
                server().post_rate(
                    id, answer_id, true,
                    [like_btn, dislike_btn](const nlohmann::json &) {
                        like_btn->setChecked(true);
                        like_btn->setDisabled(true);
                        dislike_btn->setDisabled(true);
                    },
                    [](const std::string &) {}
                );
            });
            connect(dislike_btn, &QPushButton::clicked, [id, answer_id, like_btn, dislike_btn]() {
                server().post_rate(
                    id, answer_id, false,
                    [like_btn, dislike_btn](const nlohmann::json &) {
                        dislike_btn->setChecked(true);
                        like_btn->setDisabled(true);
                        dislike_btn->setDisabled(true);
                    },
                    [](const std::string &) {}
                );
            });
        }

        auto *card = new ClickableCard([id]() {
            auto *taking = new SurveyTaking(id);
            taking->setAttribute(Qt::WA_DeleteOnClose);
        }, row);
        card->setObjectName("surveyCard");
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        auto *card_layout = new QVBoxLayout(card);
        card_layout->setContentsMargins(24, 20, 24, 20);
        card_layout->setSpacing(8);

        auto *survey_title = new QLabel(QString::fromStdString(title), card);
        survey_title->setObjectName("titleLabel");
        survey_title->setWordWrap(true);
        survey_title->setAttribute(Qt::WA_TransparentForMouseEvents);
        card_layout->addWidget(survey_title);

        if (!description.empty()) {
            auto *survey_description = new QLabel(QString::fromStdString(description), card);
            survey_description->setObjectName("descriptionLabel");
            survey_description->setWordWrap(true);
            survey_description->setAttribute(Qt::WA_TransparentForMouseEvents);
            card_layout->addWidget(survey_description);
        }

        const QString stats = tr("Score: %1  ·  %2 likes  ·  %3 dislikes  ·  %4 ratings  ·  %5 complitions")
                                  .arg(score).arg(likes).arg(dislikes).arg(total).arg(complitions);
        auto *stats_label = new QLabel(stats, card);
        stats_label->setObjectName("subtitleLabel");
        stats_label->setAttribute(Qt::WA_TransparentForMouseEvents);
        card_layout->addWidget(stats_label);

        row_layout->addWidget(vote_widget);
        row_layout->addWidget(card);

        content_layout->addWidget(row, 0, Qt::AlignHCenter);
    }

    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}

}  // namespace survey
