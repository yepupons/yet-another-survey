#include "survey_builder_window.hpp"
#include <QrCodeGenerator.h>
#include <qglobal.h>
#include <qlineedit.h>
#include <qobject.h>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <QTextEdit>
#include <algorithm>
#include <chrono>
#include <memory>
#include "nlohmann/json_fwd.hpp"
#include <QString>
#include "pretty_view.hpp"
#include "section_editor.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "survey_taking.hpp"

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

    auto *meta_card = new QWidget(this);
    meta_card->setObjectName("card");

    auto *meta_layout = new QVBoxLayout(meta_card);
    meta_layout->setContentsMargins(16, 10, 16, 12);
    meta_layout->setSpacing(4);

    auto *title_field_label = new QLabel("Title", meta_card);
    title_field_label->setObjectName("surveyMetaLabel");
    meta_layout->addWidget(title_field_label);

    title_ = new QLineEdit(meta_card);
    title_->setObjectName("surveyTitleInput");
    title_->setPlaceholderText("Untitled survey");
    meta_layout->addWidget(title_);

    auto *divider = new QFrame(meta_card);
    divider->setObjectName("dividerLine");
    divider->setFrameShape(QFrame::HLine);
    meta_layout->addSpacing(3);
    meta_layout->addWidget(divider);
    meta_layout->addSpacing(3);

    auto *desc_field_label = new QLabel("Description", meta_card);
    desc_field_label->setObjectName("surveyMetaLabel");
    meta_layout->addWidget(desc_field_label);

    description_ = new QTextEdit(meta_card);
    description_->setObjectName("surveyDescriptionInput");
    description_->setPlaceholderText("Add a description (optional)");
    description_->setFixedHeight(56);
    description_->setFrameShape(QFrame::NoFrame);
    meta_layout->addWidget(description_);

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

    sections_layout_->addWidget(meta_card);

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
        // const int preview_id = generate_survey_id();
        build_survey_json(
             true,
            [=, this](const nlohmann::json &survey) {
                auto *preview = new SurveyTaking(survey, true, this);
                preview->setAttribute(Qt::WA_DeleteOnClose);
                preview->setWindowTitle("Preview");
                preview->show();
            }
        );
    });
}

void SurveyBuilderWindow::add_section() {
    QStringList list = sections_list_->stringList();
    list.push_back("Go to section " + QString::number(list.size()));
    sections_list_->setStringList(list);

    sections_.push_back(new SectionEditor(type_, sections_list_, content_));
    sections_layout_->addWidget(sections_.back());
}

void SurveyBuilderWindow::build_sections_json(
    bool preview_mode,
    std::shared_ptr<nlohmann::json> survey,
    int current_section,
    std::function<void(const nlohmann::json &)> callback
) const {
    sections_[current_section]->to_json(
        preview_mode,
        [=, this](const nlohmann::json &section) {
            (*survey)["sections"].push_back(section);
            if (current_section == sections_.size() - 1) {
                callback(*survey);
                return;
            }
            build_sections_json(
                preview_mode, survey, current_section + 1, callback
            );
        }
    );
}

void SurveyBuilderWindow::build_survey_json(
    bool preview_mode,
    std::function<void(const nlohmann::json &)> callback
) const {
    auto survey = std::make_shared<nlohmann::json>();
    (*survey)["data"]["creator_id"] = session().get_id();
    (*survey)["data"]["type"] = write_type(type_).toStdString();
    (*survey)["title"] = title_->text().trimmed().isEmpty()
                             ? "Unnamed"
                             : title_->text().trimmed().toStdString();
    (*survey)["description"] = description_->toPlainText().trimmed().toStdString();

    (*survey)["sections"] = nlohmann::json::array();
    build_sections_json(preview_mode, survey, 0, callback);
}


void SurveyBuilderWindow::save_survey() {
    build_survey_json(false, [=, this](const nlohmann::json &survey_data) {
    server().post_survey(
        survey_data,
        [=, this](const nlohmann::json &response) {
            const QString id = QString::fromStdString(response.at("survey_id").get<std::string>());
            QrCodeGenerator generator(this);
            const QImage qr_image =
                generator.generateQr(id, 260, 4);

            show_message_box(
                parentWidget(), QPixmap::fromImage(qr_image), "Saved",
                "Survey has been saved.\nYour ID:\n" + id
            );

            deleteLater();
        },
            [=, this](const std::string &error) {
                show_message_box(
                    this, QMessageBox::Warning, "Error",
                    QString::fromStdString(error)
                );
            }
        );
    });
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
