#ifndef SECTION_EDITOR_HPP_
#define SECTION_EDITOR_HPP_
#include <qcheckbox.h>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class SectionEditor : public QWidget {
    Q_OBJECT
public:
    explicit SectionEditor(
        SurveyType type,
        QStringListModel *sections_list,
        QWidget *parent = nullptr
    );

    void build_questions_json(
        bool preview_mode,
        std::shared_ptr<nlohmann::json> section,
        int current_question,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const;
    void to_json(
        bool preview_mode,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const;
    void setup_block_actions(BlockEditor *block);

private slots:
    void add_block();
    void add_outcome();

private:
    QLineEdit *title_ = nullptr;
    QList<BlockEditor *> questions_;
    QVBoxLayout *questions_layout_ = nullptr;
    QComboBox *next_section_ = nullptr;
    QStringListModel *sections_list_ = nullptr;
    QStringListModel *outcomes_model_ = nullptr;
    QList<QLineEdit *> outcomes_;
    QVBoxLayout *outcomes_layout_ = nullptr;
    QCheckBox *use_AI_ = nullptr;
    QPushButton *add_block_button_ = nullptr;

    SurveyType type_;
};
}  // namespace survey

#endif