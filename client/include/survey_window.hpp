#ifndef SURVEY_WINDOW_HPP_
#define SURVEY_WINDOW_HPP_

#include <QMainWindow>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

class QPushButton;

namespace survey {
class SurveyWindow : public QMainWindow {
    Q_OBJECT

public:
    SurveyWindow(const nlohmann::json &survey_data, QWidget *parent = nullptr);

public slots:
    void save_answer();

private:
    int survey_id_;
    QList<QuestionBlock *> questions_;
    QPushButton *save_answer_button_;
};
}  // namespace survey
#endif  // SURVEY_WINDOW_HPP_
