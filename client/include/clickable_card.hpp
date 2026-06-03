#pragma once

#include <QWidget>
#include <QMouseEvent>

namespace survey {
class ClickableCard : public QWidget {
public:
    explicit ClickableCard(std::function<void()> on_click, QWidget *parent = nullptr)
        : QWidget(parent), on_click_(std::move(on_click)) {
        setCursor(Qt::PointingHandCursor);
    }
protected:
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton) on_click_();
    }
private:
    std::function<void()> on_click_;
};
}  // namespace survey