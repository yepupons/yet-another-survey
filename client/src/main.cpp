#include <QApplication>
#include <nlohmann/json.hpp>
#include "main_window.hpp"
#include "session_id.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFont font = app.font();
    font.setPointSize(16);
    app.setFont(font);
    session_id = survey::SessionIdGenerator::generate_session_id();
    survey::MainWindow window;
    window.show();
    return app.exec();
}
