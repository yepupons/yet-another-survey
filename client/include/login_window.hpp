#ifndef LOGIN_WINDOW_HPP_
#define LOGIN_WINDOW_HPP_

#include <QMainWindow>
#include <QString>

class QPushButton;
class QLabel;

namespace survey {
class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

public slots:
    void start_telegram_login();

private:
    QPushButton *login_button_;
    QLabel *status_label_;
    QString telegram_url_;
};
}  // namespace survey

#endif  // LOGIN_WINDOW_HPP_
