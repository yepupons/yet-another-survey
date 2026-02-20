#ifndef MAIN_WINDOW_HPP_
#define MAIN_WINDOW_HPP_

#include <QList>
#include <QMainWindow>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

class QPushButton;

namespace survey {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(
        const nlohmann::json &in_file,
        nlohmann::json &out_file,
        QWidget *parent = nullptr
    );

public slots:
    void save_answer();

private:
    QList<QuestionBlock *> questions_{};
    nlohmann::json answers_;
    QPushButton *save_answer_;
};
}  // namespace survey
#endif  // MAIN_WINDOW_HPP_
