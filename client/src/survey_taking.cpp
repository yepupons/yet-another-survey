#include "survey_taking.hpp"
#include <QMessageBox>
#include <QWidget>
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "test_results.hpp"

namespace survey {
SurveyTaking::SurveyTaking(int survey_id, QWidget *parent) : QWidget(parent) {
    try {
        survey_data_ = ServerInteraction::get_survey(survey_id);
    } catch (const std::exception &e) {
        show_message_box(parent, QMessageBox::Warning, "Error", e.what());
        deleteLater();
        return;
    }
    answer_data_["data"]["id"] = 67;  // when it will be valid...
    answer_data_["data"]["survey_id"] = survey_id;
    answer_data_["data"]["respondent_id"] = session().get_id();
    answer_data_["sections"] = nlohmann::json::array();
    for (int i = 0; i < survey_data_.at("sections").size(); ++i) {
        answer_data_["sections"].push_back(nlohmann::json::array());
    }
    open_next_section(0);
}

void SurveyTaking::open_next_section(int next_section_id) {
    if (next_section_id == -1) {
        try {
            if (survey_data_.at("data").at("type") == "test") {
                auto result = ServerInteraction::check_answer(answer_data_);
                auto *view = new ViewTestResults(result, this);
                view->setAttribute(Qt::WA_DeleteOnClose);
                view->show();
                connect(view, &QDialog::finished, this, [this]() {
                    deleteLater();
                });
                return;
            } else {
                ServerInteraction::post_answer(answer_data_);
                show_message_box(
                    this, QMessageBox::Information, "Saved",
                    "Your answers have been successfully saved."
                );
            }
        } catch (const std::exception &e) {
            show_message_box(this, QMessageBox::Warning, "Error", e.what());
        }
        deleteLater();
        return;
    }
    current_section_ =
        new SurveyWindow(survey_data_, answer_data_, next_section_id, this);
    current_section_->setAttribute(Qt::WA_DeleteOnClose);
    current_section_->showMaximized();
    current_section_->raise();
    current_section_->activateWindow();
    connect(
        current_section_, &SurveyWindow::closed_with_answer, this,
        &SurveyTaking::open_next_section
    );
    connect(
        current_section_, &SurveyWindow::closed_without_answer, this,
        [this]() {
            show_message_box(
                this, QMessageBox::Warning, "Error",
                "Your answers haven't been saved"
            );
            deleteLater();
        }
    );
}
}  // namespace survey
