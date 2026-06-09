#pragma once

#include <QDialog>
#include <QObject>
#include <string>

namespace survey {
class ViewPassedSurveys : public QDialog {
    Q_OBJECT

public:
    explicit ViewPassedSurveys(QWidget *parent = nullptr);
};

class ViewSurveyResults : public QDialog {
    Q_OBJECT

public:
    explicit ViewSurveyResults(
        const std::string &survey_id,
        const std::string &answer_id,
        QWidget *parent = nullptr
    );
};
}  // namespace survey
