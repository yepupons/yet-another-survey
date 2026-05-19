#include "survey_builder_window.hpp"
#include <QrCodeGenerator.h>
#include <qglobal.h>
#include <qlineedit.h>
#include <qobject.h>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPixmap>
#include <chrono>
#include "pretty_view.hpp"
#include "section_editor.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "survey_taking.hpp"

namespace survey {
SurveyBuilderWindow::SurveyBuilderWindow(bool is_test, QWidget *parent)
    : QMainWindow(parent), is_test_(is_test) {
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");

    auto *central_layout = new QVBoxLayout();
    central_layout->setAlignment(Qt::AlignTop);
    central_layout->setContentsMargins(24, 24, 24, 24);
    central_layout->setSpacing(16);

    setWindowTitle(
        is_test_ ? "Survey Builder (Test)" : "Survey Builder (Survey)"
    );

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

    auto *bottom_row = new QHBoxLayout();
    bottom_row->addStretch();

    add_section_button_ = new QPushButton("New section", central);
    add_section_button_->setObjectName("primaryButton");
    add_section_button_->setFixedSize(180, 40);
    bottom_row->addWidget(add_section_button_);

    central_layout->addLayout(bottom_row);

    save_survey_button_ =
        new QPushButton(is_test_ ? "Save test" : "Save survey", central);
    save_survey_button_->setObjectName("primaryButton");
    central_layout->addWidget(save_survey_button_);

    central->setLayout(central_layout);
    setCentralWidget(central);

    connect(
        add_section_button_, &QPushButton::clicked, this,
        &SurveyBuilderWindow::add_section
    );
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

void SurveyBuilderWindow::add_section() {
    QStringList list = sections_list_->stringList();
    list.push_back("Go to section " + QString::number(list.size()));
    sections_list_->setStringList(list);

    sections_.push_back(new SectionEditor(is_test_, sections_list_, content_));
    sections_layout_->addWidget(sections_.back());
}

nlohmann::json SurveyBuilderWindow::build_survey_json(int id, bool preview_mode)
    const {
    nlohmann::json survey;
    survey["data"]["id"] = id;
    survey["data"]["creator_id"] = session().get_id();
    survey["data"]["type"] = is_test_ ? "test" : "survey";
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
    server().post_survey(
        build_survey_json(id, false),
        [&, id]() {
            QrCodeGenerator generator(this);
            const QImage qr_image =
                generator.generateQr(QString::number(id), 260, 4);
            show_message_box(
                parentWidget(), QPixmap::fromImage(qr_image), "Saved",
                "Survey has been saved.\nYour ID:\n" + QString::number(id)
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
}
}  // namespace survey
