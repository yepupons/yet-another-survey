#ifndef MAIN_WINDOW_HPP_
#define MAIN_WINDOW_HPP_

#include <QList>
#include <QMainWindow>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

class QPushButton;
class QLineEdit;
class QVBoxLayout;
class QWidget;

namespace survey {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(
        nlohmann::json &out_file,
        QWidget *parent = nullptr
    );

public slots:
    void load_survey();
    void save_answer();

private:
    void build_questions(const nlohmann::json &in_file);
    void clear_questions();

    QList<QuestionBlock *> questions_{};
    nlohmann::json answers_;
    QWidget *central_;
    QVBoxLayout *layout_;
    QLineEdit *id_input_;
    QPushButton *load_button_;
    QPushButton *save_answer_;
    bool survey_loaded_{false};
    int requested_id_{0};
    int survey_id_{0};
    int answer_id_{0};
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
