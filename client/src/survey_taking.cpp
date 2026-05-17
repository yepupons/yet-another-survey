#include "survey_taking.hpp"
#include <map>
#include <QMessageBox>
#include <QWidget>
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "test_results.hpp"

namespace survey {
SurveyTaking::SurveyTaking(int survey_id, QWidget *parent)
    : QWidget(parent), preview_mode_(false) {
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

SurveyTaking::SurveyTaking(
    const nlohmann::json& survey_data,
    bool preview_mode,
    QWidget *parent
)
    : QWidget(parent),
      preview_mode_(preview_mode),
      survey_data_(survey_data) {
    answer_data_["data"]["id"] = -1;
    answer_data_["data"]["survey_id"] = survey_data_.at("data").at("id");
    answer_data_["data"]["respondent_id"] = session().get_id();
    answer_data_["sections"] = nlohmann::json::array();

    for (std::size_t i = 0; i < survey_data_.at("sections").size(); ++i) {
        answer_data_["sections"].push_back(nlohmann::json::array());
    }

    open_next_section(0);
}

void SurveyTaking::open_next_section(int next_section_id) {
    if (next_section_id == -1) {
        if (preview_mode_) {
            show_message_box(
                    this, QMessageBox::Information, "Exit",
                    "Preview finished."
            );
            deleteLater();
            return;
        }
        try {
            const std::string type = survey_data_.at("data").at("type");
            if (type == "test") {
                auto result = ServerInteraction::check_answer(answer_data_);
                auto *view = new ViewTestResults(result, this);
                view->setAttribute(Qt::WA_DeleteOnClose);
                view->show();
                connect(view, &QDialog::finished, this, [this]() {
                    deleteLater();
                });
                return;
            } else if (type == "quiz") {
                std::map<std::string, int> scores;

                for (size_t i = 0; i < answer_data_["sections"].size(); ++i) {
                    const auto &answers = answer_data_["sections"][i];
                    const auto &questions = survey_data_["sections"][i]["questions"];

                    for (size_t j = 0; j < answers.size(); ++j) {
                        int answer_idx = answers[j]["answer"].get<int>();
                        if (answer_idx <= 0) continue;

                        const auto &scores_arr = questions[j]["scores"];
                        if (static_cast<size_t>(answer_idx - 1) < scores_arr.size()) {
                            scores[scores_arr[answer_idx - 1].get<std::string>()]++;
                        }
                    }
                }

                std::string winner;
                int max_score = -1;
                for (const auto &[name, score] : scores) {
                    if (score > max_score) {
                        max_score = score;
                        winner = name;
                    }
                }

                show_message_box(
                    this, QMessageBox::Information, "Result",
                    "You are: " + QString::fromStdString(winner)
                );
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
        new SurveyWindow(survey_data_, answer_data_, next_section_id, preview_mode_, this);
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
