#ifndef SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#define SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QRadioButton>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class SingleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit SingleChoiceBlockEditor(
        Created_Type type,
        QStringListModel *sections_list,
        QWidget *parent = nullptr
    );

    void to_json(
        bool preview_mode,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const override;

private slots:
    void add_option();

private:
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