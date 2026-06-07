#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QTranslator>
#include <nlohmann/json.hpp>
#include "main_window.hpp"
#include "russian_hotkeys_handler.hpp"

#ifdef Q_OS_WASM
#include <emscripten/val.h>
#include <QUrlQuery>
#endif

int main(int argc, char *argv[]) {
    int exit_code = 67;
    while (exit_code == 67) {
        QApplication app(argc, argv);

        QSettings settings("yet-another-survey", "yet-another-survey");
        const QString lang = settings.value("language", "ru").toString();

        QTranslator translator;
        if (translator.load(":/translations/app_" + lang + ".qm")) {
            app.installTranslator(&translator);
        } else {
            qWarning() << "Failed to load translation for:" << lang;
        }

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
#ifdef Q_OS_WASM
        emscripten::val location = emscripten::val::global("window")["location"];
        QString query_str = QString::fromStdString(location["search"].as<std::string>()); 
        QUrlQuery query(query_str);
        if (query.hasQueryItem("?survey-id")) {
            window.set_survey_id(query.queryItemValue("?survey-id"));
        }
#endif
        window.showFullScreen();
        exit_code = app.exec();
    }
    return exit_code;
}
