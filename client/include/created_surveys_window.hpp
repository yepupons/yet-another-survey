#ifndef CREATED_SURVEYS_WINDOW_HPP_
#define CREATED_SURVEYS_WINDOW_HPP_

#include <QDialog>
#include <QLabel>
#include <QList>
#include <nlohmann/json.hpp>
#include <string>

namespace survey {
class CreatedSurveysWindow : public QDialog {
    Q_OBJECT

public:
    explicit CreatedSurveysWindow(QWidget *parent = nullptr);

public slots:
    void show_qr_code(const std::string &id);
    void
    show_survey_preview(QWidget *parent, const nlohmann::json &survey_data);
    void export_statistics(const std::string &survey_id, const std::string &file_format);
};
}  // namespace survey

#endif
