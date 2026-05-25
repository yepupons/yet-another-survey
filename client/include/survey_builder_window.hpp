#ifndef SURVEY_BUILDER_WINDOW_HPP_
#define SURVEY_BUILDER_WINDOW_HPP_
#include <qtmetamacros.h>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QString>
#include <nlohmann/json.hpp>
#include "section_editor.hpp"

namespace survey {
class SurveyBuilderWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit SurveyBuilderWindow(Created_Type type, QWidget *parent = nullptr);

private slots:
    void add_section();
    void save_survey();

private:
    Created_Type type_;

    QWidget *content_ = nullptr;

    QLineEdit *title_ = nullptr;
    QList<SectionEditor *> sections_;
    QVBoxLayout *sections_layout_ = nullptr;
    QStringListModel *sections_list_ = nullptr;

    QPushButton *add_section_button_ = nullptr;
    QPushButton *save_survey_button_ = nullptr;

    nlohmann::json build_survey_json(int id, bool preview_mode) const;
    static int generate_survey_id();
    static const QString write_type(Created_Type type);
};
}  // namespace survey

#endif
