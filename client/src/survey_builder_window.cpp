#include "survey_builder_window.hpp"
#include <qglobal.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <chrono>
#include "section_editor.hpp"
#include "server_interaction.hpp"
#include "session.hpp"

namespace survey {
SurveyBuilderWindow::SurveyBuilderWindow(bool is_test, QWidget *parent)
    : QMainWindow(parent), is_test_(is_test) {
    setWindowTitle(
        is_test_ ? "Survey Builder (Test)" : "Survey Builder (Survey)"
    );

    auto *central = new QWidget(this);
    auto *central_layout = new QVBoxLayout();
    central_layout->setAlignment(Qt::AlignTop);

    auto *title = new QLabel("Survey Builder", central);
    central_layout->addWidget(title);

    auto *scroll_area = new QScrollArea(central);
    content_ = new QWidget();
    scroll_area->setWidget(content_);

    sections_layout_ = new QVBoxLayout();
    sections_layout_->setAlignment(Qt::AlignTop);
    content_->setLayout(sections_layout_);

    sections_list_ = new QStringListModel(this);
    QStringList list = sections_list_->stringList();
    sections_list_->setStringList(list << "Save answers");

    add_section();

    scroll_area->setWidget(content_);
    scroll_area->setWidgetResizable(true);
    central_layout->addWidget(scroll_area);

    auto *bottom_row = new QHBoxLayout();
    bottom_row->addStretch();

    add_section_button_ = new QPushButton("+", central);
    add_section_button_->setFixedSize(40, 40);
    bottom_row->addWidget(add_section_button_);

    central_layout->addLayout(bottom_row);

    save_survey_button_ = new QPushButton("Save survey", central);
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
}

void SurveyBuilderWindow::add_section() {
    QStringList list = sections_list_->stringList();
    list.push_back("Go to section " + QString::number(list.size()));
    sections_list_->setStringList(list);

    sections_.push_back(new SectionEditor(is_test_, sections_list_, content_));
    sections_layout_->addWidget(sections_.back());
}

nlohmann::json SurveyBuilderWindow::build_survey_json(int id) const {
    nlohmann::json survey;
    survey["data"] = {
        {"id", id},
        {"creator_id", session().get_id()},
        {"type", is_test_ ? "test" : "survey"}
    };
    survey["sections"] = nlohmann::json::array();

    for (auto *section : sections_) {
        survey["sections"].push_back(section->to_json());
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
    nlohmann::json survey = build_survey_json(id);

    try {
        ServerInteraction::post_survey(id, survey);
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "Error", e.what());
        return;
    }

    QMessageBox::information(
        this, "Saved",
        "Survey has been saved.\nYour ID:\n" + QString::number(id)
    );
}
}  // namespace survey
