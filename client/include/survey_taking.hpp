#ifndef SURVEY_TAKING_HPP_
#define SURVEY_TAKING_HPP_

#include <nlohmann/json.hpp>
#include "survey_window.hpp"

namespace survey {
class SurveyTaking : public QWidget {
    Q_OBJECT

public:
    SurveyTaking(int survey_id, QWidget *parent = nullptr);

public slots:
    void open_next_section(int next_section_id);

private:
    nlohmann::json survey_data_;
    nlohmann::json answer_data_;
    SurveyWindow *current_section_ = nullptr;
};
}  // namespace survey

#endif