#ifndef SURVEY_WINDOW_HPP_
#define SURVEY_WINDOW_HPP_

#include <QMainWindow>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"
#include <QCloseEvent>

class QPushButton;

namespace survey {
class SurveyWindow : public QMainWindow {
    Q_OBJECT

public:
    SurveyWindow(const nlohmann::json &survey_data, nlohmann::json &answer_data, int section_id, QWidget *parent = nullptr);

public slots:
    void save_answer();

signals:
    void closed_with_answer(int next_section_id);
    void closed_without_answer();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    const nlohmann::json &section_data_;
    nlohmann::json &answer_data_;
    bool answer_saved;
    
    QList<QuestionBlock *> questions_;
    QPushButton *save_answer_button_;
};
}  // namespace survey
#endif  // SURVEY_WINDOW_HPP_
