#include <QApplication>
#include <nlohmann/json.hpp>
#include "main_window.hpp"

int main(int argc, char *argv[]) {
    nlohmann::json answer_data = {
        {"answer_data", {{"survey_id", 0}, {"answer_id", 0}}}, {"answers", {}}};

    QApplication a(argc, argv);
    survey::MainWindow w(answer_data);
    w.show();
    return a.exec();
}
