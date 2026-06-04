#ifndef SURVEY_BUILDER_WINDOW_HPP_
#define SURVEY_BUILDER_WINDOW_HPP_
#include <qtmetamacros.h>
#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QTextEdit>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include "nlohmann/json_fwd.hpp"
#include "section_editor.hpp"

namespace survey {
class SurveyBuilderWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit SurveyBuilderWindow(SurveyType type, QWidget *parent = nullptr);

private slots:
    void add_section();
    void save_survey();

private:
    SurveyType type_;

    QWidget *content_ = nullptr;

    QLineEdit *title_ = nullptr;
    QTextEdit *description_ = nullptr;
    QCheckBox *is_public_ = nullptr;
    QList<SectionEditor *> sections_;
    QVBoxLayout *sections_layout_ = nullptr;
    QStringListModel *sections_list_ = nullptr;

    QPushButton *add_section_button_ = nullptr;
    QPushButton *save_survey_button_ = nullptr;

    void build_sections_json(
        bool preview_mode,
        std::shared_ptr<nlohmann::json> survey,
        int current_section,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const;
    void build_survey_json(
        bool preview_mode,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const;
    static const QString write_type(SurveyType type);
};
}  // namespace survey

#endif
