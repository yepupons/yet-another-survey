#ifndef MAIN_WINDOW_HPP_
#define MAIN_WINDOW_HPP_

#include <qobject.h>
#include <QAction>
#include <QLabel>
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
    void set_survey_id(QString survey_id);

public slots:
    void open_survey();
    void create_survey();
    void get_created_surveys();
    void get_passed_surveys();
    void show_trending();
    void update_auth_action();
    void load_global_stats();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void update_header_title();

    QLabel *header_title_ = nullptr;
    QLineEdit *id_input_;
    QPushButton *open_survey_button_;
    QPushButton *create_survey_button_;
    QAction *auth_action_;
    SurveyTaking *opened_survey_ = nullptr;

    QLabel *surveys_stat_label_ = nullptr;
    QLabel *answers_stat_label_  = nullptr;
    QLabel *ratings_stat_label_  = nullptr;
    QLabel *users_stat_label_    = nullptr;
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
