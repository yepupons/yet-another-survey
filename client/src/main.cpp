#include <QApplication>
#include <nlohmann/json.hpp>
#include "main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font = app.font();
    font.setPointSize(16);
    app.setFont(font);

    survey::MainWindow window;
    window.show();
    return app.exec();
}
