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

        auto *card = new ClickableCard([id]() {
            auto *taking = new SurveyTaking(id);
            taking->setAttribute(Qt::WA_DeleteOnClose);
        }, content);
        card->setObjectName("surveyCard");
        card->setFixedWidth(720);

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

        const QString stats = tr("Score: %1  ·  %2 likes  ·  %3 dislikes  ·  %4 ratings")
                                  .arg(score).arg(likes).arg(dislikes).arg(total);
        auto *stats_label = new QLabel(stats, card);
        stats_label->setObjectName("subtitleLabel");
        stats_label->setAttribute(Qt::WA_TransparentForMouseEvents);
        card_layout->addWidget(stats_label);

        content_layout->addWidget(card, 0, Qt::AlignHCenter);
    }

    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}

}  // namespace survey
