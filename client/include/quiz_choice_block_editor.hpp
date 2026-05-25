#ifndef QUIZ_CHOICE_BLOCK_EDITOR_HPP_
#define QUIZ_CHOICE_BLOCK_EDITOR_HPP_

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class QuizChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit QuizChoiceBlockEditor(
        QStringListModel *outcomes_model,
        QWidget *parent = nullptr
    );

    nlohmann::json to_json(bool preview_mode) const override;

private slots:
    void add_option();

private:
    QStringListModel *outcomes_model_ = nullptr;

    QList<QLineEdit *> options_;
    QList<QComboBox *> outcome_selectors_;

    QVBoxLayout *options_layout_ = nullptr;
    QPushButton *add_option_button_ = nullptr;
    QCheckBox *required_ = nullptr;
};
}  // namespace survey

#endif
