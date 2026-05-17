#ifndef LOGIN_WINDOW_HPP_
#define LOGIN_WINDOW_HPP_

#include <QMainWindow>
#include <QString>

class QPushButton;
class QLabel;
class QTimer;

namespace survey {
class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

public slots:
    void start_telegram_login();

signals:
    void login_completed();

private:
    QPushButton *login_button_;
    QLabel *status_label_;

    QString telegram_url_;
    int expires_in_ = 0;
    QString challenge_id_;

    QTimer *countdown_timer_;
    QTimer *poll_timer_;
};
}  // namespace survey

#endif  // LOGIN_WINDOW_HPP_
