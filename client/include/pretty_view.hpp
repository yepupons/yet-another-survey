#ifndef PRETTY_VIEW_HPP_
#define PRETTY_VIEW_HPP_

#include <qnamespace.h>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>

namespace survey {
inline void show_message_box(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text
) {
    auto *box = new QMessageBox(parent);
    box->setIcon(icon);
    box->setWindowTitle(title);
    box->setInformativeText(text);
    box->setTextFormat(Qt::RichText);
    box->setStandardButtons(QMessageBox::Ok);
    box->setStyleSheet(parent ? parent->styleSheet() : QString());
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->show();
}

inline void show_message_box(
    QWidget *parent,
    const QPixmap &pixmap,
    const QString &title,
    const QString &text
) {
    auto dialog = new QDialog(parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    dialog->setWindowTitle(title);

    auto *root = new QVBoxLayout(dialog);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->setSizeConstraint(QLayout::SetFixedSize);

    auto *content = new QWidget(dialog);
    content->setObjectName("centralWidget");

    auto *content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(18);

    auto *message_row = new QHBoxLayout();
    message_row->setSpacing(24);

    auto *qr_label = new QLabel(content);
    qr_label->setPixmap(pixmap);
    qr_label->setFixedSize(pixmap.size());
    qr_label->setScaledContents(false);

    auto *text_label = new QLabel(text, content);
    text_label->setWordWrap(true);

    message_row->addWidget(qr_label);
    message_row->addWidget(text_label);

    auto *ok_button = new QPushButton("OK", content);
    QBoxLayout::connect(
        ok_button, &QPushButton::clicked, dialog, &QDialog::accept
    );

    auto *button_row = new QHBoxLayout();
    button_row->addStretch();
    button_row->addWidget(ok_button);

    content_layout->addLayout(message_row);
    content_layout->addLayout(button_row);
    root->addWidget(content);

    dialog->show();
}

inline QMessageBox *show_question_box(
    QWidget *parent,
    QMessageBox::Icon icon,
    const QString &title,
    const QString &text,
    QMessageBox::StandardButton accept_button = QMessageBox::Yes,
    QMessageBox::StandardButton reject_button = QMessageBox::No,
    QMessageBox::StandardButton default_button = QMessageBox::No
) {
    auto *box = new QMessageBox(parent);
    box->setIcon(icon);
    box->setWindowTitle(title);
    box->setInformativeText(text);
    box->setTextFormat(Qt::RichText);
    box->setStandardButtons(accept_button | reject_button);
    box->setDefaultButton(default_button);
    box->setStyleSheet(parent ? parent->styleSheet() : QString());
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->show();
    return box;
}
}  // namespace survey

#endif
