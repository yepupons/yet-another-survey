#pragma once

#include <QDialog>
#include <nlohmann/json.hpp>

namespace survey {
class TopSurveysWindow : public QDialog {
    Q_OBJECT

public:
    explicit TopSurveysWindow(
        const nlohmann::json &surveys, QWidget *parent = nullptr
    );
};
}  // namespace survey
