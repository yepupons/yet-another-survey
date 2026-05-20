#include "survey_taking.hpp"
#include <qobject.h>
#include <QMessageBox>
#include <QWidget>
#include "nlohmann/json_fwd.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "test_results.hpp"

namespace survey {
SurveyTaking::SurveyTaking(int survey_id, QWidget *parent)
    : QWidget(parent), preview_mode_(false) {
    server().get_survey(
        survey_id,
        [=, this](const nlohmann::json &survey_data) {
            survey_data_ = survey_data;
            answer_data_["data"]["id"] = 67;  // when it will be valid...
            answer_data_["data"]["survey_id"] = survey_id;
            answer_data_["data"]["respondent_id"] = session().get_id();
            answer_data_["sections"] = nlohmann::json::array();
            for (int i = 0; i < survey_data_.at("sections").size(); ++i) {
                answer_data_["sections"].push_back(nlohmann::json::array());
            }
            open_next_section(0);
        },
        [=, this](const std::string &error) {
            show_message_box(
                parent, QMessageBox::Warning, "Error",
                QString::fromStdString(error)
            );
            deleteLater();
        }
    );
}

SurveyTaking::SurveyTaking(
    const nlohmann::json &survey_data,
    bool preview_mode,
    QWidget *parent
)
    : QWidget(parent), preview_mode_(preview_mode), survey_data_(survey_data) {
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
                parentWidget(), QMessageBox::Information, "Exit",
                "Preview finished."
            );
            deleteLater();
            return;
        }
        if (survey_data_.at("data").at("type") == "test") {
            server().check_answer(
                answer_data_,
                [=, this](const nlohmann::json &result) {
                    auto *view = new ViewTestResults(result, this);
                    view->setAttribute(Qt::WA_DeleteOnClose);
                    view->show();
                },
                [=, this](const std::string &error) {
                    show_message_box(
                        parentWidget(), QMessageBox::Warning, "Error",
                        QString::fromStdString(error)
                    );
                    deleteLater();
                }
            );
        } else {
            server().post_answer(
                answer_data_,
                [=, this]() {
                    show_message_box(
                        parentWidget(), QMessageBox::Information, "Saved",
                        "Your answers have been successfully saved."
                    );
                    deleteLater();
                },
                [=, this](const std::string &error) {
                    show_message_box(
                        parentWidget(), QMessageBox::Warning, "Error",
                        QString::fromStdString(error)
                    );
                    deleteLater();
                }
            );
        }
    } else {
        current_section_ = new SurveyWindow(
            survey_data_, answer_data_, next_section_id, preview_mode_, this
        );
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
                    parentWidget(), QMessageBox::Warning, "Error",
                    "Your answers haven't been saved"
                );
                deleteLater();
            }
        );
    }
}
}  // namespace survey
