#pragma once

#include <QDialog>
#include <QObject>

namespace survey {
class ViewPassedSurveys : public QDialog {
    Q_OBJECT

public:
    explicit ViewPassedSurveys(
        QWidget *parent = nullptr
    );
};

class ViewSurveyResults : public QDialog {
    Q_OBJECT

public:
    explicit ViewSurveyResults(
        int survey_id,
        int session_id,
        QWidget *parent = nullptr
    );
};
}  // namespace survey
