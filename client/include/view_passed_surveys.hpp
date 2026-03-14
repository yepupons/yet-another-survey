#pragma once

#include <QDialog>
#include <QStringList>

namespace survey {
class ViewPassedSurveys : public QDialog {
    Q_OBJECT

public:
    explicit ViewPassedSurveys(
        const QStringList &survey_ids,
        QWidget *parent = nullptr
    );
};
}  // namespace survey
