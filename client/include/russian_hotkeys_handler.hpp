#ifndef RUSSIAN_HOTKEYS_HANDLER_HPP_
#define RUSSIAN_HOTKEYS_HANDLER_HPP_

#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMetaObject>
#include <QObject>

class RussianHotkeysHandler : public QObject {
    Q_OBJECT
public:
    explicit RussianHotkeysHandler(QObject *parent = nullptr)
        : QObject(parent) {
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                QString text = keyEvent->text().toLower();
                QByteArray slot_name;

                if (text == "с") {
                    slot_name = "copy";
                } else if (text == "м") {
                    slot_name = "paste";
                } else if (text == "ч") {
                    slot_name = "cut";
                } else if (text == "ф") {
                    slot_name = "selectAll";
                }

                if (!slot_name.isEmpty()) {
                    QObject *focus_object = QGuiApplication::focusObject();
                    if (focus_object) {
                        bool success = QMetaObject::invokeMethod(
                            focus_object, slot_name.constData()
                        );
                        if (success) {
                            return true;
                        }
                    }
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

#endif  // RUSSIAN_HOTKEYS_HANDLER_HPP_