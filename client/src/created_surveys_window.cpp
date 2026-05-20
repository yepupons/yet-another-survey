#include "created_surveys_window.hpp"
#include <QrCodeGenerator.h>
#include <qobject.h>
// #include <matplot/matplot.h>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <exception>
#include <fstream>
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

    auto *title_label = new QLabel("Created surveys", title_card);
    title_label->setObjectName("titleLabel");
    title_layout->addWidget(title_label);

    auto *subtitle_label = new QLabel(
        "Preview surveys, share QR codes, or export statistics.", title_card
    );
    subtitle_label->setObjectName("subtitleLabel");
    title_layout->addWidget(subtitle_label);

    content_layout->addWidget(title_card, 0, Qt::AlignHCenter);

    auto *surveys_card = new QWidget(content);
    surveys_card->setObjectName("questionCard");
    surveys_card->setFixedWidth(720);
    surveys_card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *surveys_layout = new QVBoxLayout(surveys_card);
    surveys_layout->setContentsMargins(24, 24, 24, 24);
    surveys_layout->setSpacing(12);

    server().get_created_surveys(
        session().get_id(),
        [=, this](const nlohmann::json &surveys_ids) {
            if (surveys_ids.empty()) {
                auto *empty_label =
                    new QLabel("No created surveys yet.", surveys_card);
                empty_label->setObjectName("titleLabel");
                surveys_layout->addWidget(empty_label);
                content_layout->addWidget(surveys_card, 0, Qt::AlignHCenter);
                content_layout->addStretch();
                scroll_area->setWidget(content);
                layout->addWidget(scroll_area);
                setLayout(layout);
                return;
            }

            for (int id : surveys_ids) {
                auto *row_widget = new QWidget(surveys_card);
                auto *row_layout = new QVBoxLayout(row_widget);
                row_layout->setContentsMargins(0, 0, 0, 0);
                row_layout->setSpacing(10);

                auto *survey_title = new QLabel(row_widget);
                survey_title->setObjectName("titleLabel");
                survey_title->setSizePolicy(
                    QSizePolicy::Expanding, QSizePolicy::Preferred
                );
                survey_title->setWordWrap(true);
                server().get_survey(
                    id,
                    [=, this](const nlohmann::json &survey_data) {
                        auto title = survey_data.at("title").get<std::string>();
                        survey_title->setText(
                            QString::fromStdString(title) +
                            " (id: " + QString::number(id) + ")"
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
                row_layout->addWidget(survey_title);

                auto *actions_layout = new QHBoxLayout();
                actions_layout->setContentsMargins(0, 0, 0, 0);
                actions_layout->setSpacing(10);

                auto *show_qr_button = new QPushButton("Show QR", row_widget);
                show_qr_button->setObjectName("primaryButton");
                actions_layout->addWidget(show_qr_button, 1);
                connect(
                    show_qr_button, &QPushButton::clicked, this,
                    [this, id]() { show_qr_code(id); }
                );

                auto *view_survey_button =
                    new QPushButton("Preview", row_widget);
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

                auto *txt_export_button =
                    new QPushButton("Export TXT", row_widget);
                txt_export_button->setObjectName("primaryButton");
                actions_layout->addWidget(txt_export_button, 1);
                connect(
                    txt_export_button, &QPushButton::clicked, this,
                    [this, id]() { export_statistics_txt(id); }
                );

                auto *jpg_export_button =
                    new QPushButton("Export JPG", row_widget);
                jpg_export_button->setObjectName("primaryButton");
                actions_layout->addWidget(jpg_export_button, 1);
                connect(
                    jpg_export_button, &QPushButton::clicked, this,
                    [this, id]() { export_statistics_jpg(id); }
                );

                row_layout->addLayout(actions_layout);
                surveys_layout->addWidget(row_widget);
            }
        },
        [this](const std::string &error) {
            show_message_box(
                parentWidget(), QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
            deleteLater();
            return;
        }
    );

    content_layout->addWidget(surveys_card, 0, Qt::AlignHCenter);
    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}

void CreatedSurveysWindow::export_statistics_txt(int survey_id) {
    /*
    std::ofstream file(std::to_string(survey_id) + ".txt");
    if (!file.is_open()) {
        show_message_box(
            this, QMessageBox::Warning, "Error", "Unable to create file"
        );
        return;
    }
    server().get_survey(
        survey_id,
        [=, this](const nlohmann::json &survey) {
            server().get_survey_statistics(
                survey_id,
                [=, this](const nlohmann::json &stats) {
                    file << "Survey: \"" << survey["title"] << "\"\n\n";
                    file << "Total number of answers: " <<
    stats["total_answers"] << "\n\n"; file << "Statistic by sections:\n\n"; for
    (int i = 0; i < survey["sections"].size(); ++i) { file << "Section " <<
    survey["sections"][i]["title"] << ":\n\n"; for (int j = 0; j <
    survey["sections"][i]["questions"].size(); ++j) { const auto &question =
    survey["sections"][i]["questions"][j]; file << "Question " <<
    question["text"] << ":\n"; if (question["type"] == "single" ||
                                question["type"] == "multiple") {
                                for (int k = 1; k <= question["options"].size();
    ++k) { file << question["options"][k - 1] << ": "; if
    (stats["sections"][i][j].contains(std::to_string(k))) { file <<
    stats["sections"][i][j][std::to_string(k)]; } else { file << 0;
                                    }
                                    file << " answer(s)\n";
                                }
                            } else if (question["type"] == "text") {
                                for (const auto &answer_count :
                                    stats["sections"][i][j].items()) {
                                    file << '\"' << answer_count.key()
                                        << "\": " << answer_count.value() << "
    answer(s)\n";
                                }
                            }
                            file << '\n';
                        }
                    }
                    file.close();
                },
                [=, this](const std::string &error) {
                    show_message_box(this, QMessageBox::Warning, "Error",
    QString::fromStdString(error));
                }
            )
        },
        [=, this](const std::string &error) {
            show_message_box(this, QMessageBox::Warning, "Error",
    QString::fromStdString(error));
        }
    );
    */
}

void CreatedSurveysWindow::export_statistics_jpg(int survey_id) {
    /*
    using namespace matplot;

    nlohmann::json survey = ServerInteraction::get_survey(survey_id);
    nlohmann::json stats = ServerInteraction::get_survey_statistics(survey_id);

    int total_rows = 0;
    for (const auto &section : survey["sections"]) {
        total_rows += section["questions"].size();
    }

    auto f = figure(true);
    f->size(1200, 300 * total_rows);

    int plot_index = 0;
    for (int i = 0; i < survey["sections"].size(); ++i) {
        const auto &section = survey["sections"][i];
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            subplot(total_rows, 1, plot_index++);
            title(
                "[Section: " + section["title"].get<std::string>() + "] " +
                "Question: " + question["text"].get<std::string>()
            );

            std::vector<double> values;
            std::vector<std::string> labels;
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    std::string label = question["options"][k - 1];
                    if (label.size() > 20) {
                        label = label.substr(0, 17) + "...";
                    }
                    labels.push_back(label);

                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        values.push_back(
                            stats["sections"][i][j][std::to_string(k)]
                        );
                    } else {
                        values.push_back(0);
                    }
                }
            } else if (question["type"] == "text") {
                if (stats["sections"][i][j].empty()) {
                    labels.push_back("No answers yet");
                    values.push_back(0);
                } else {
                    std::vector<std::pair<std::string, int>> answers;
                    for (const auto &item : stats["sections"][i][j].items()) {
                        answers.emplace_back(item.key(), item.value());
                    }
                    std::sort(
                        answers.begin(), answers.end(),
                        [](auto &a, auto &b) { return a.second > b.second; }
                    );

                    for (int k = 0; k < std::min(10UL, answers.size()); ++k) {
                        std::string label = answers[k].first;
                        if (label.size() > 20) {
                            label = label.substr(0, 17) + "...";
                        }
                        labels.push_back(label);
                        values.push_back(answers[k].second);
                    }
                }
            }
            bar(values);

            xticks(iota(1, labels.size()));
            xticklabels(labels);

            int max_value = *std::max_element(values.begin(), values.end());
            yticks(iota(0, max_value + 1));
            ylim({0, static_cast<double>(max_value + 1)});
        }
    }
    if (!save(std::to_string(survey_id) + ".jpg")) {
        show_message_box(
            this, QMessageBox::Warning, "Error", "Unable to create file"
        );
    };
    close();
    */
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
    preview_window->show();
}

void CreatedSurveysWindow::show_qr_code(int id) {
    QrCodeGenerator generator(this);
    const QImage qr_image = generator.generateQr(QString::number(id), 260, 4);
    show_message_box(
        this, QPixmap::fromImage(qr_image), "QR code",
        "Survey ID:\n" + QString::number(id)
    );
}

}  // namespace survey
