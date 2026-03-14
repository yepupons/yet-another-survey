#ifndef TEXT_BLOCK_EDITOR_HPP_
#define TEXT_BLOCK_EDITOR_HPP_
#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class TextBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit TextBlockEditor(bool is_test, QWidget *parent = nullptr);

    nlohmann::json to_json() const override;

private slots:
    void add_correct_answer();

private:
    QLineEdit *question_ = nullptr;
    QCheckBox *required_ = nullptr;

    QList<QLineEdit *> correct_answers_;
    QVBoxLayout *correct_answers_layout_ = nullptr;
    QPushButton *add_correct_answer_button_ = nullptr;
};
}  // namespace survey

#endif