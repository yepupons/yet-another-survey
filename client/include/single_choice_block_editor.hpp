#ifndef SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#define SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QList>
#include <QComboBox>
#include <QStringListModel>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class SingleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit SingleChoiceBlockEditor(bool is_test, QStringListModel *sections_list, QWidget *parent = nullptr);

    nlohmann::json to_json() const override;

private slots:
    void add_option();

private:
    QLineEdit *question_ = nullptr;

    QList<QLineEdit *> options_;
    QList<QRadioButton *> link_enablings_;
    QList<QComboBox *> links_;
    QStringListModel *sections_list_ = nullptr;

    QVBoxLayout *options_layout_ = nullptr;
    QPushButton *add_option_button_ = nullptr;
    QCheckBox *required_ = nullptr;

    QButtonGroup *correct_answers_ = nullptr;
};
}  // namespace survey

#endif