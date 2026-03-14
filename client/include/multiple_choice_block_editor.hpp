#ifndef MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#define MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QList>
#include <QButtonGroup>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class MultipleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit MultipleChoiceBlockEditor(bool is_test, QWidget *parent = nullptr);

    nlohmann::json to_json() const override;

private slots:
    void add_option();

private:
    QLineEdit *question_ = nullptr;
    QList<QLineEdit *> options_;
    QVBoxLayout *options_layout_ = nullptr;
    QPushButton *add_option_button_ = nullptr;
    QCheckBox *required_ = nullptr;
    
    QButtonGroup *correct_answers_ = nullptr;
};
}  // namespace survey

#endif