#ifndef SURVEY_BUILDER_WINDOW_HPP_
#define SURVEY_BUILDER_WINDOW_HPP_
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>

namespace survey {
class SurveyBuilderWindow : public QWidget {
    Q_OBJECT
public:
    enum class Mode { Survey, Test };

    explicit SurveyBuilderWindow(Mode mode, QWidget *parent = nullptr);

private slots:
    void add_block_menu();
    void add_single_choice();
    void add_multiple_choice();
    void add_text_block();
    void save_survey();

private:
    Mode mode_;

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
