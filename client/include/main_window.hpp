#ifndef MAIN_WINDOW_HPP_
#define MAIN_WINDOW_HPP_

#include <QAction>
#include <QMainWindow>
#include "survey_taking.hpp"

class QAction;
class QLineEdit;
class QPushButton;

namespace survey {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void open_survey();
    void create_survey();
    void get_created_surveys();
    void get_passed_surveys();
    void show_trending();
    void update_auth_action();

private:
    QLineEdit *id_input_;
    QPushButton *open_survey_button_;
    QPushButton *create_survey_button_;
    QAction *auth_action_;
    SurveyTaking *opened_survey_ = nullptr;
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
