#include "survey_builder_window.hpp"
#include "survey_taking.hpp"
#include <QrCodeGenerator.h>
#include <qglobal.h>
#include <qlineedit.h>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <algorithm>
#include <chrono>
#include <QString>
#include "pretty_view.hpp"
#include "section_editor.hpp"
#include "server_interaction.hpp"
#include "session.hpp"

namespace survey {
SurveyBuilderWindow::SurveyBuilderWindow(Created_Type type, QWidget *parent)
    : QMainWindow(parent), type_(type) {
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *central_layout = new QVBoxLayout();
    central_layout->setAlignment(Qt::AlignTop);
    central_layout->setContentsMargins(24, 24, 24, 24);
    central_layout->setSpacing(16);
    
    setWindowTitle("Survey Builder (" + write_type(type) + ")");

    auto *top_row = new QHBoxLayout();

    auto *title_label = new QLabel("Survey Builder", central);
    title_label->setObjectName("titleLabel");

    auto *preview_button = new QPushButton("Preview", central);
    preview_button->setObjectName("primaryButton");
    preview_button->setFixedSize(140, 40);

    top_row->addWidget(title_label);
    top_row->addStretch();
    top_row->addWidget(preview_button);

    central_layout->addLayout(top_row);

    title_ = new QLineEdit(this);
    title_->setPlaceholderText("Write survey title here");
    central_layout->addWidget(title_);

    if (type_ == QUIZ) {
        outcomes_layout_ = new QVBoxLayout();
        central_layout->addLayout(outcomes_layout_);

        outcomes_model_ = new QStringListModel(this);

        auto *add_outcome_button = new QPushButton("Add outcome", central);
        add_outcome_button->setObjectName("secondaryButton");
        central_layout->addWidget(add_outcome_button);

        connect(add_outcome_button, &QPushButton::clicked, this,
            &SurveyBuilderWindow::add_outcome);
    }

    auto *scroll_area = new QScrollArea(central);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    content_ = new QWidget();
    content_->setObjectName("centralWidget");
    scroll_area->setWidget(content_);

    sections_layout_ = new QVBoxLayout();
    sections_layout_->setAlignment(Qt::AlignTop);
    sections_layout_->setContentsMargins(0, 0, 0, 0);
    sections_layout_->setSpacing(16);
    content_->setLayout(sections_layout_);

    sections_list_ = new QStringListModel(this);
    QStringList list = sections_list_->stringList();
    sections_list_->setStringList(list << "Save answers");

    add_section();

    central_layout->addWidget(scroll_area);

    if (type_ != QUIZ) {
        auto *bottom_row = new QHBoxLayout();
        bottom_row->addStretch();

        add_section_button_ = new QPushButton("New section", central);
        add_section_button_->setObjectName("primaryButton");
        add_section_button_->setFixedSize(180, 40);
        bottom_row->addWidget(add_section_button_);

        central_layout->addLayout(bottom_row);
    }

    save_survey_button_ = new QPushButton("Save " + write_type(type), central);
    save_survey_button_->setObjectName("primaryButton");
    central_layout->addWidget(save_survey_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

    if (add_section_button_) {
        connect(
            add_section_button_, &QPushButton::clicked, this,
            &SurveyBuilderWindow::add_section
        );
    }
    connect(
        save_survey_button_, &QPushButton::clicked, this,
        &SurveyBuilderWindow::save_survey
    );
    connect(preview_button, &QPushButton::clicked, this, [this]() {
        const int preview_id = generate_survey_id();
        nlohmann::json survey = build_survey_json(preview_id, true);

        auto *preview = new SurveyTaking(survey, true, this);
        preview->setAttribute(Qt::WA_DeleteOnClose);
        preview->setWindowTitle("Preview");
        preview->show();
    });
}

void SurveyBuilderWindow::add_outcome() {
    auto *row_widget = new QWidget(this);
    auto *row_layout = new QHBoxLayout(row_widget);
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(8);

    auto *outcome_edit = new QLineEdit(row_widget);
    outcome_edit->setPlaceholderText("Write outcome here");
    row_layout->addWidget(outcome_edit);
    outcomes_.push_back(outcome_edit);

    connect(outcome_edit, &QLineEdit::textChanged, this, [this](const QString &) {
        QStringList list;
        for (auto *edit : outcomes_) {
            list.append(edit->text().trimmed());
        }
        outcomes_model_->setStringList(list);
    });

    auto *delete_button = new QPushButton(row_widget);
    delete_button->setObjectName("dangerIconButton");
    delete_button->setFixedSize(36, 36);
    row_layout->addWidget(delete_button);

    connect(delete_button, &QPushButton::clicked, this,
        [this, row_widget, outcome_edit]() {
            outcomes_.erase(
                std::remove(outcomes_.begin(), outcomes_.end(), outcome_edit),
                outcomes_.end()
            );
            QStringList list;
            for (auto *edit : outcomes_) {
                list.append(edit->text().trimmed());
            }
            outcomes_model_->setStringList(list);
            outcomes_layout_->removeWidget(row_widget);
            row_widget->deleteLater();
        }
    );

    outcomes_layout_->addWidget(row_widget);
}

void SurveyBuilderWindow::add_section() {
    QStringList list = sections_list_->stringList();
    list.push_back("Go to section " + QString::number(list.size()));
    sections_list_->setStringList(list);

    sections_.push_back(new SectionEditor(type_, sections_list_, outcomes_model_, content_));
    sections_layout_->addWidget(sections_.back());
}

nlohmann::json SurveyBuilderWindow::build_survey_json(int id, bool preview_mode) const {
    nlohmann::json survey;
    survey["data"]["id"] = id;
    survey["data"]["creator_id"] = session().get_id();
    survey["data"]["type"] = write_type(type_).toStdString();
    survey["title"] = title_->text().trimmed().isEmpty()
                          ? "Unnamed"
                          : title_->text().trimmed().toStdString();
    survey["sections"] = nlohmann::json::array();
    for (auto *section : sections_) {
        survey["sections"].push_back(section->to_json(preview_mode));
    }
    return survey;
}

int SurveyBuilderWindow::generate_survey_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
    )
                  .count();

    return static_cast<int>(ms % 1000000000);
}

void SurveyBuilderWindow::save_survey() {
    const int id = generate_survey_id();
    nlohmann::json survey = build_survey_json(id, false);

    try {
        ServerInteraction::post_survey(survey);
    } catch (const std::exception &e) {
        show_message_box(this, QMessageBox::Warning, "Error", e.what());
        return;
    }

    QrCodeGenerator generator(this);
    const QImage qr_image = generator.generateQr(QString::number(id), 260, 4);
    show_message_box(
        this, QPixmap::fromImage(qr_image), "Saved",
        "Survey has been saved.\nYour ID:\n" + QString::number(id)
    );
    deleteLater();
}

const QString SurveyBuilderWindow::write_type(Created_Type type){
    switch (type){
        case SURVEY:
            return "survey";
        case TEST:
            return "test";
        case QUIZ:
            return "quiz";
    }
}
}  // namespace survey
