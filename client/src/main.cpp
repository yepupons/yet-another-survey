#include <QApplication>
#include <QFile>
#include <nlohmann/json.hpp>
#include "main_window.hpp"
#include "russian_hotkeys_handler.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }
    QFont font = app.font();
    font.setPointSize(14);
    app.setFont(font);
    app.installEventFilter(new RussianHotkeysHandler(&app));
    survey::MainWindow window;
    window.showMaximized();
    return app.exec();
}
