#include "survey_taking.hpp"
#include <qobject.h>
#include <QMessageBox>
#include <QWidget>
#include <map>
#include "enums.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"
#include "test_results.hpp"

namespace survey {
SurveyTaking::SurveyTaking(const std::string &survey_id, QWidget *parent)
    : QWidget(parent), preview_mode_(false), survey_id_(survey_id) {
    server().get_survey(
        survey_id,
        [=, this](const nlohmann::json &survey_data) {
            survey_data_ = survey_data;
            answer_data_["data"]["survey_id"] = survey_id;
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
    std::string survey_id = survey_data_.at("data").value("id", "preview");
    survey_id_ = survey_id;
    answer_data_["data"]["survey_id"] = survey_id;
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
        
        // maybe we should delete this???

        // auto show_rating = [=, this](const std::string &answer_id) {
        //     auto *box = show_question_box(
        //         parentWidget(), QMessageBox::Question,
        //         tr("Rate this survey"), tr("Did you like this survey?"),
        //         QMessageBox::Yes, QMessageBox::No
        //     );
        //     connect(box, &QMessageBox::finished, this, [=, this](int result) {
        //         if (result == QMessageBox::Yes || result == QMessageBox::No) {
        //             server().post_rate(
        //                 survey_id_, answer_id, result == QMessageBox::Yes,
        //                 [](const nlohmann::json &) {}, [](const std::string &) {}
        //             );
        //         }
        //         deleteLater();
        //     });
        // };

        auto type =
            SURVEY_TYPE.at(survey_data_.at("data").at("type").get<std::string>()
            );
        switch (type) {
            case SurveyType::Survey: {
                server().post_answer(
                    answer_data_, survey_id_,
                    [=, this](const nlohmann::json &submission_result) {
                        submission_result_ = submission_result;
                        const std::string answer_id =
                            submission_result.at("answer_id").get<std::string>();
                        // show_rating(answer_id);
                    },
                    [=, this](const std::string &error) {
                        show_message_box(
                            parentWidget(), QMessageBox::Warning, "Error",
                            QString::fromStdString(error)
                        );
                        deleteLater();
                    }
                );
                break;
            }
            case SurveyType::Test: {
                server().check_answer(
                    answer_data_,
                    [=, this](const nlohmann::json &result) {
                        auto *view = new ViewTestResults(result, this);
                        view->setAttribute(Qt::WA_DeleteOnClose);
                        view->show();
                        // show_rating(
                        //     result.at("data").at("answer_id").get<std::string>()
                        // );
                    },
                    [=, this](const std::string &error) {
                        show_message_box(
                            parentWidget(), QMessageBox::Warning, "Error",
                            QString::fromStdString(error)
                        );
                        deleteLater();
                    }
                );
                break;
            }
            case SurveyType::Quiz: {
                std::map<std::string, int> scores;

                for (size_t i = 0; i < answer_data_["sections"].size(); ++i) {
                    const auto &answers = answer_data_["sections"][i];
                    const auto &questions =
                        survey_data_["sections"][i]["questions"];

                    for (size_t j = 0; j < answers.size(); ++j) {
                        int answer_idx = answers[j]["answer"].get<int>();
                        if (answer_idx <= 0) {
                            continue;
                        }

                        const auto &scores_arr = questions[j]["scores"];
                        if (static_cast<size_t>(answer_idx - 1) <
                            scores_arr.size()) {
                            scores[scores_arr[answer_idx - 1].get<std::string>(
                            )]++;
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
                server().post_answer(
                    answer_data_, survey_id_,
                    [=, this](const nlohmann::json &submission_result) {
                        const std::string answer_id =
                            submission_result.at("answer_id").get<std::string>();
                        // show_rating(answer_id);
                    },
                    [=, this](const std::string &error) {
                        show_message_box(
                            parentWidget(), QMessageBox::Warning, "Error",
                            QString::fromStdString(error)
                        );
                        deleteLater();
                    }
                );
                break;
            }
        }

    } else {
        current_section_ = new SurveyWindow(
            survey_data_, answer_data_, next_section_id, preview_mode_, this
        );
        current_section_->setAttribute(Qt::WA_DeleteOnClose);
        current_section_->showFullScreen();
        current_section_->raise();
        current_section_->activateWindow();
        connect(
            current_section_, &SurveyWindow::closed_with_answer, this,
            &SurveyTaking::open_next_section, Qt::DirectConnection
        );
        connect(
            current_section_, &SurveyWindow::closed_without_answer, this,
            [this]() {
                show_message_box(
                    parentWidget(), QMessageBox::Warning, "Error",
                    "Your answers haven't been saved"
                );
                deleteLater();
            },
            Qt::DirectConnection
        );
    }
}
}  // namespace survey
