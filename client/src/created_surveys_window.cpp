#include "created_surveys_window.hpp"
#include <QrCodeGenerator.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qstringview.h>
#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include <string>
#include "nlohmann/json_fwd.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "survey_window.hpp"

namespace survey {
CreatedSurveysWindow::CreatedSurveysWindow(QWidget *parent) : QDialog(parent) {
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

    auto *title_label = new QLabel(tr("Created surveys"), title_card);
    title_label->setObjectName("titleLabel");
    title_layout->addWidget(title_label);

    auto *subtitle_label = new QLabel(
        tr("Preview surveys, share QR codes, or view statistics."), title_card
    );
    subtitle_label->setObjectName("subtitleLabel");
    title_layout->addWidget(subtitle_label);

    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);

    server().get_created_surveys(
        session().get_id(),
        [=, this](const nlohmann::json &surveys) {
            if (surveys.empty()) {
                auto *empty_card = new QWidget(content);
                empty_card->setObjectName("questionCard");
                empty_card->setFixedWidth(720);
                auto *empty_layout = new QVBoxLayout(empty_card);
                empty_layout->setContentsMargins(24, 24, 24, 24);
                auto *empty_label =
                    new QLabel(tr("No created surveys yet."), empty_card);
                empty_label->setObjectName("titleLabel");
                empty_layout->addWidget(empty_label);
                content_layout->addWidget(empty_card, 0, Qt::AlignHCenter);
                return;
            }

            for (auto iter = surveys.rbegin(); iter != surveys.rend(); iter = std::next(iter)) {
                const auto &survey = *iter;
                const std::string id = survey["id"];
                const std::string title = survey["title"];

                auto *survey_card = new QWidget(content);
                survey_card->setObjectName("questionCard");
                survey_card->setFixedWidth(720);
                survey_card->setSizePolicy(
                    QSizePolicy::Expanding, QSizePolicy::Preferred
                );

                auto *card_layout = new QVBoxLayout(survey_card);
                card_layout->setContentsMargins(24, 24, 24, 24);
                card_layout->setSpacing(10);

                auto *survey_title = new QLabel(survey_card);
                survey_title->setObjectName("titleLabel");
                survey_title->setSizePolicy(
                    QSizePolicy::Expanding, QSizePolicy::Preferred
                );
                survey_title->setWordWrap(true);
                survey_title->setText(QString::fromStdString(title));
                card_layout->addWidget(survey_title);

                auto *actions_layout = new QHBoxLayout();
                actions_layout->setContentsMargins(0, 0, 0, 0);
                actions_layout->setSpacing(10);

                auto *show_qr_button =
                    new QPushButton(tr("Show QR"), survey_card);
                show_qr_button->setObjectName("primaryButton");
                actions_layout->addWidget(show_qr_button, 1);
                connect(
                    show_qr_button, &QPushButton::clicked, this,
                    [this, id]() { show_qr_code(id); }
                );

                auto *view_survey_button =
                    new QPushButton(tr("Preview"), survey_card);
                view_survey_button->setObjectName("primaryButton");
                actions_layout->addWidget(view_survey_button, 1);
                connect(
                    view_survey_button, &QPushButton::clicked, this,
                    [this, id]() {
                        server().get_survey(
                            id,
                            [=, this](const nlohmann::json &survey_data) {
                                show_survey_preview(this, survey_data);
                            },
                            [=, this](const std::string &error) {
                                show_message_box(
                                    parentWidget(), QMessageBox::Warning,
                                    "Error", QString::fromStdString(error)
                                );
                                deleteLater();
                            }
                        );
                    }
                );

                auto *view_stats_button =
                    new QPushButton(tr("View Stats"), survey_card);
                view_stats_button->setObjectName("primaryButton");
                actions_layout->addWidget(view_stats_button, 1);
                connect(
                    view_stats_button, &QPushButton::clicked, this,
                    [this, id]() {
                        auto *dialog = new ViewSurveyStats(id, this);
                        dialog->setAttribute(Qt::WA_DeleteOnClose);
                        dialog->showFullScreen();
                    }
                );

                card_layout->addLayout(actions_layout);
                content_layout->addWidget(survey_card, 0, Qt::AlignHCenter);
            }
        },
        [this](const std::string &error) {
            show_message_box(
                parentWidget(), QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
            deleteLater();
        }
    );
}

void CreatedSurveysWindow::show_survey_preview(
    QWidget *parent,
    const nlohmann::json &survey_data
) {
    auto *preview_answers = new nlohmann::json;
    (*preview_answers)["sections"] = nlohmann::json::array();
    for (size_t i = 0; i < survey_data.at("sections").size(); ++i) {
        (*preview_answers)["sections"].push_back(nlohmann::json::array());
    }

    SurveyWindow *preview_window =
        new SurveyWindow(survey_data, *preview_answers, 0, parent);
    preview_window->setAttribute(Qt::WA_DeleteOnClose);
    connect(preview_window, &QObject::destroyed, this, [preview_answers]() {
        delete preview_answers;
    });
    preview_window->showFullScreen();
}

void CreatedSurveysWindow::show_qr_code(const std::string &id) {
    QrCodeGenerator generator(this);
    const QString survey_id = QString::fromStdString(id);
    const QImage qr_image = generator.generateQr(survey_id, 260, 4);
    survey::show_qr_code(
        this, QPixmap::fromImage(qr_image), tr("QR code"), survey_id
    );
}

ViewSurveyStats::ViewSurveyStats(
    const std::string &survey_id,
    QWidget *parent
)
    : QDialog(parent), survey_id_(survey_id) {
    setWindowTitle(tr("Survey statistics"));

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
            server().get_survey_statistics(
                survey_id, "json",
                [=, this](const std::string &stats_str) {
                    const nlohmann::json stats =
                        nlohmann::json::parse(stats_str);

                    auto *title_card = new QWidget(content);
                    title_card->setObjectName("questionCard");
                    title_card->setFixedWidth(720);

                    auto *title_card_layout = new QVBoxLayout(title_card);
                    title_card_layout->setContentsMargins(24, 24, 24, 24);
                    title_card_layout->setSpacing(8);

                    auto *survey_title_label = new QLabel(
                        QString::fromStdString(survey.at("title")),
                        title_card
                    );
                    survey_title_label->setObjectName("titleLabel");
                    title_card_layout->addWidget(survey_title_label);

                    if (!survey.at("description").empty()) {
                        auto *desc_label = new QLabel(
                            QString::fromStdString(survey.at("description")),
                            title_card
                        );
                        desc_label->setObjectName("subtitleLabel");
                        desc_label->setWordWrap(true);
                        title_card_layout->addWidget(desc_label);
                    }

                    const int total = stats.value("total_answers", 0);
                    auto *total_label = new QLabel(
                        tr("Total responses: %1").arg(total),
                        title_card
                    );
                    total_label->setObjectName("descriptionLabel");
                    title_card_layout->addWidget(total_label);

                    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

                    const auto &sections = survey.at("sections");
                    for (int si = 0; si < (int)sections.size(); ++si) {
                        const auto &section = sections.at(si);

                        auto *section_card = new QWidget(content);
                        section_card->setObjectName("questionCard");
                        section_card->setFixedWidth(720);

                        auto *section_layout = new QVBoxLayout(section_card);
                        section_layout->setContentsMargins(24, 24, 24, 24);
                        section_layout->setSpacing(8);

                        auto *section_title_label = new QLabel(
                            QString::fromStdString(
                                section.value("title", "Section")
                            ),
                            section_card
                        );
                        section_title_label->setObjectName("titleLabel");
                        section_layout->addWidget(section_title_label);

                        content_layout->addWidget(
                            section_card, 0, Qt::AlignHCenter
                        );

                        const auto &questions = section.at("questions");
                        for (int qi = 0; qi < (int)questions.size(); ++qi) {
                            const auto &question = questions.at(qi);
                            const std::string type = question.at("type");

                            auto *q_card = new QWidget(content);
                            q_card->setObjectName("questionCard");
                            q_card->setFixedWidth(720);

                            auto *q_layout = new QVBoxLayout(q_card);
                            q_layout->setContentsMargins(24, 24, 24, 24);
                            q_layout->setSpacing(10);

                            auto *q_label = new QLabel(
                                QString::fromStdString(
                                    question.at("text").get<std::string>()
                                ),
                                q_card
                            );
                            q_label->setObjectName("questionTitle");
                            q_label->setWordWrap(true);
                            q_layout->addWidget(q_label);

                            const nlohmann::json &q_stats =
                                (stats.contains("sections") &&
                                 si < (int)stats["sections"].size() &&
                                 qi < (int)stats["sections"][si].size())
                                ? stats["sections"][si][qi]
                                : nlohmann::json::object();

                            if (type == "single" || type == "multiple") {
                                const auto &options = question.at("options");
                                for (int oi = 0; oi < (int)options.size(); ++oi) {
                                    const std::string key =
                                        std::to_string(oi + 1);
                                    const int count =
                                        q_stats.contains(key)
                                        ? q_stats[key].get<int>()
                                        : 0;

                                    auto *opt_row = new QWidget(q_card);
                                    auto *opt_layout =
                                        new QHBoxLayout(opt_row);
                                    opt_layout->setContentsMargins(0, 0, 0, 0);
                                    opt_layout->setSpacing(12);

                                    auto *opt_label = new QLabel(
                                        QString::fromStdString(
                                            options.at(oi)
                                        ),
                                        opt_row
                                    );
                                    opt_label->setObjectName("subtitleLabel");
                                    opt_label->setWordWrap(true);
                                    opt_label->setStyleSheet("color: black;");
                                    opt_layout->addWidget(opt_label, 1);

                                    auto *count_label = new QLabel(
                                        tr("%1 responses").arg(count), opt_row
                                    );
                                    count_label->setObjectName("descriptionLabel");
                                    count_label->setStyleSheet("color: black;");
                                    count_label->setAlignment(
                                        Qt::AlignRight | Qt::AlignVCenter
                                    );
                                    opt_layout->addWidget(count_label);

                                    q_layout->addWidget(opt_row);
                                }
                            } else {
                                for (const auto &[answer, cnt] :
                                     q_stats.items()) {
                                    auto *ans_row = new QWidget(q_card);
                                    auto *ans_layout =
                                        new QHBoxLayout(ans_row);
                                    ans_layout->setContentsMargins(0, 0, 0, 0);
                                    ans_layout->setSpacing(12);

                                    auto *ans_label = new QLabel(
                                        QString::fromStdString(answer),
                                        ans_row
                                    );
                                    ans_label->setObjectName("subtitleLabel");
                                    ans_label->setWordWrap(true);
                                    ans_label->setStyleSheet("color: black;");
                                    ans_layout->addWidget(ans_label, 1);

                                    auto *cnt_label = new QLabel(
                                        tr("%1 responses").arg(cnt.get<int>()),
                                        ans_row
                                    );
                                    cnt_label->setObjectName("descriptionLabel");
                                    cnt_label->setStyleSheet("color: black;");
                                    cnt_label->setAlignment(
                                        Qt::AlignRight | Qt::AlignVCenter
                                    );
                                    ans_layout->addWidget(cnt_label);

                                    q_layout->addWidget(ans_row);
                                }
                            }

                            content_layout->addWidget(
                                q_card, 0, Qt::AlignHCenter
                            );
                        }
                    }
                    content_layout->addStretch();
                },
                [=, this](const std::string &error) {
                    auto *err = new QLabel(
                        QString::fromStdString("Error: " + error), content
                    );
                    content_layout->addWidget(err);
                    content_layout->addStretch();
                }
            );
        },
        [=, this](const std::string &error) {
            auto *err = new QLabel(
                QString::fromStdString("Error: " + error), content
            );
            content_layout->addWidget(err);
            content_layout->addStretch();
        }
    );

    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);

    auto *btns_widget = new QWidget(this);
    btns_widget->setObjectName("statsFooter");
    btns_widget->setStyleSheet("QWidget#statsFooter { background-color: white; }");
    auto *btns_layout = new QHBoxLayout(btns_widget);
    btns_layout->setContentsMargins(24, 12, 24, 12);
    btns_layout->setSpacing(12);
    btns_layout->addStretch();

    auto *export_txt_btn = new QPushButton(tr("Export TXT"), btns_widget);
    export_txt_btn->setObjectName("primaryButton");
    connect(
        export_txt_btn, &QPushButton::clicked, this,
        [this]() { export_statistics(survey_id_, "txt"); }
    );
    btns_layout->addWidget(export_txt_btn);

    auto *export_jpg_btn = new QPushButton(tr("Export JPG"), btns_widget);
    export_jpg_btn->setObjectName("primaryButton");
    connect(
        export_jpg_btn, &QPushButton::clicked, this,
        [this]() { export_statistics(survey_id_, "jpg"); }
    );
    btns_layout->addWidget(export_jpg_btn);

    layout->addWidget(btns_widget);
    setLayout(layout);
}

void ViewSurveyStats::export_statistics(
    const std::string &survey_id,
    const std::string &file_format
) {
    server().get_survey_statistics(
        survey_id, file_format,
        [=](const std::string &file_data) {
            QFileDialog::saveFileContent(
                QByteArray::fromStdString(file_data),
                QString::fromStdString(survey_id) + '.' +
                    QString::fromStdString(file_format)
            );
        },
        [=, this](const std::string &error) {
            show_message_box(
                this, QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
        }
    );
}

}  // namespace survey
