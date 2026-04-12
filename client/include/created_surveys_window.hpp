#ifndef CREATED_SURVEYS_WINDOW_HPP_
#define CREATED_SURVEYS_WINDOW_HPP_

#include <QDialog>
#include <QLabel>
#include <QList>

namespace survey {
class CreatedSurveysWindow : public QDialog {
    Q_OBJECT

public:
    explicit CreatedSurveysWindow(QWidget *parent = nullptr);

public slots:
    void import_statistics_txt(int survey_id);
    void import_statistics_jpg(int survey_id);
};
}  // namespace survey

#endif