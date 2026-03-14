#ifndef SURVEY_BUILDER_WINDOW_HPP_
#define SURVEY_BUILDER_WINDOW_HPP_
#include <qtmetamacros.h>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QList>
#include <QStringListModel>
#include <nlohmann/json.hpp>
#include "section_editor.hpp"

namespace survey {
class SurveyBuilderWindow : public QMainWindow {
    Q_OBJECT
public:

    explicit SurveyBuilderWindow(bool is_test, QWidget *parent = nullptr);

private slots:
    void add_section();
    void save_survey();

private:
    bool is_test_;

    QWidget *content_ = nullptr;
    
    QList<SectionEditor *> sections_;
    QVBoxLayout *sections_layout_ = nullptr;
    QStringListModel* sections_list_ = nullptr;

    QPushButton *add_section_button_ = nullptr;
    QPushButton *save_survey_button_ = nullptr;

    nlohmann::json build_survey_json(int id) const;
    static int generate_survey_id();
};
}  // namespace survey

#endif
