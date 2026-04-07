#ifndef MAIN_WINDOW_HPP_
#define MAIN_WINDOW_HPP_

#include <QMainWindow>
#include "survey_window.hpp"

class QLineEdit;
class QPushButton;

namespace survey {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void open_survey();
    void open_next_section(int next_section_id);
    void create_survey();
    void change_session_id();
    void get_passed_surveys();

private:
    nlohmann::json opened_survey_data_;
    nlohmann::json opened_answer_data_;

    QLineEdit *id_input_;
    QPushButton *open_survey_button_;
    QPushButton *create_survey_button_;
    SurveyWindow *opened_survey_ = nullptr;
    QPushButton *change_session_id_button_;
    QPushButton *get_passed_surveys_button_;
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
