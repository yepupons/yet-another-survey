#ifndef PRETTY_VIEW_HPP_
#define PRETTY_VIEW_HPP_

#include <QMessageBox>
#include <QString>
#include <QWidget>

namespace survey {
inline void show_message_box(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text
) {
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setInformativeText(QString("<div style='max-width: 220px; white-space: "
                                   "normal; word-wrap: break-word;'>%1</div>")
                               .arg(text.toHtmlEscaped()));
    box.setTextFormat(Qt::RichText);
    box.setStandardButtons(QMessageBox::Ok);
    box.setStyleSheet(parent ? parent->styleSheet() : QString());
    box.exec();
}

inline QMessageBox::StandardButton show_question_box(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButton accept_button,
    QMessageBox::StandardButton reject_button,
    QMessageBox::StandardButton default_button = QMessageBox::No
) {
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setInformativeText(QString("<div style='max-width: 220px; white-space: "
                                   "normal; word-wrap: break-word;'>%1</div>")
                               .arg(text.toHtmlEscaped()));
    box.setTextFormat(Qt::RichText);
    box.setStandardButtons(accept_button | reject_button);
    box.setDefaultButton(default_button);
    box.setStyleSheet(parent ? parent->styleSheet() : QString());
    return static_cast<QMessageBox::StandardButton>(box.exec());
}
}  // namespace survey

#endif