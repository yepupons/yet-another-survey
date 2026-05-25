#pragma once

#include <QDialog>
#include <QObject>
#include <nlohmann/json.hpp>

namespace survey {
class ViewTestResults : public QDialog {
    Q_OBJECT
public:
    explicit ViewTestResults(
        const nlohmann::json &results,
        QWidget *parent = nullptr
    );
};

}  // namespace survey
