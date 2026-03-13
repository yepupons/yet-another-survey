#ifndef SURVEY_BUILDER_WINDOW_HPP_
#define SURVEY_BUILDER_WINDOW_HPP_
#include "builder_mode.hpp"
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>

namespace survey {
class SurveyBuilderWindow : public QWidget {
    Q_OBJECT
public:

    explicit SurveyBuilderWindow(BuilderMode mode, QWidget *parent = nullptr);

private slots:
    void add_block_menu();
    void add_single_choice();
    void add_multiple_choice();
    void add_text_block();
    void save_survey();

private:
    BuilderMode mode_;

    QScrollArea *scroll_ = nullptr;
    QWidget *content_ = nullptr;
    QVBoxLayout *contentLayout_ = nullptr;

    QPushButton *addBlockButton_ = nullptr;
    QPushButton *saveSurveyButton_ = nullptr;

    nlohmann::json build_survey_json(int) const;

    static int generate_survey_id();
};
}  // namespace survey

#endif
