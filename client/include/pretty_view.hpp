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
}  // namespace survey

#endif