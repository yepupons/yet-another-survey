#include "created_surveys_window.hpp"
#include <QrCodeGenerator.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qstringview.h>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QPushButton>
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

            for (const auto &id_json : surveys_ids) {
                const std::string id = id_json.get<std::string>();
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
                            " (id: " + QString::fromStdString(id) + ")"
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
                    [this, id]() { export_statistics(id, "txt"); }
                );

                auto *jpg_export_button =
                    new QPushButton("Export JPG", row_widget);
                jpg_export_button->setObjectName("primaryButton");
                actions_layout->addWidget(jpg_export_button, 1);
                connect(
                    jpg_export_button, &QPushButton::clicked, this,
                    [this, id]() { export_statistics(id, "jpg"); }
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

void CreatedSurveysWindow::export_statistics(const std::string &survey_id, const std::string &file_format) {
    server().get_survey_statistics(
        survey_id,
        file_format,
        [=](const std::string &file_data) {
            QFileDialog::saveFileContent(QByteArray::fromStdString(file_data), QString::fromStdString(survey_id) + '.' + QString::fromStdString(file_format));
        },
        [=, this](const std::string &error) {
            show_message_box(this, QMessageBox::Warning, "Error", QString::fromStdString(error));
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
    preview_window->show();
}

void CreatedSurveysWindow::show_qr_code(const std::string &id) {
    QrCodeGenerator generator(this);
    const QString survey_id = QString::fromStdString(id);
    const QImage qr_image = generator.generateQr(survey_id, 260, 4);
    show_message_box(
        this, QPixmap::fromImage(qr_image), "QR code",
        "Survey ID:\n" + survey_id
    );
}

}  // namespace survey
