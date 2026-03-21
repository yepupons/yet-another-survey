#include <QApplication>
#include <QFile>
#include <nlohmann/json.hpp>
#include "main_window.hpp"
#include "session_id.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    QFont font = app.font();
    font.setPointSize(16);
    app.setFont(font);
    session_id = survey::SessionIdGenerator::generate_session_id();
    survey::MainWindow window;
    window.show();
    return app.exec();
}
