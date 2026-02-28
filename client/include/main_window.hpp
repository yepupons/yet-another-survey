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
    void create_survey();

private:
    QLineEdit *id_input_;
    QPushButton *open_survey_button_;
    QPushButton *create_survey_button_;
    SurveyWindow *opened_survey_ = nullptr;
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
