#ifndef SURVEY_TAKING_HPP_
#define SURVEY_TAKING_HPP_

#include <nlohmann/json.hpp>
#include <string>
#include "survey_window.hpp"

namespace survey {
class SurveyTaking : public QWidget {
    Q_OBJECT

public:
    explicit SurveyTaking(
        const std::string &survey_id,
        QWidget *parent = nullptr
    );
    explicit SurveyTaking(
        const nlohmann::json &survey_data,
        bool preview_mode,
        QWidget *parent = nullptr
    );

public slots:
    void open_next_section(int next_section_id);

private:
    bool preview_mode_ = false;
    nlohmann::json survey_data_;
    nlohmann::json answer_data_;
    nlohmann::json submission_result_;
    SurveyWindow *current_section_ = nullptr;  
};
}  // namespace survey

#endif
