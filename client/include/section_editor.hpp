#ifndef SECTION_EDITOR_HPP_
#define SECTION_EDITOR_HPP_
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
class SectionEditor : public QWidget {
    Q_OBJECT
public:
    explicit SectionEditor(
        bool is_test,
        QStringListModel *sections_list,
        QWidget *parent = nullptr
    );

    nlohmann::json to_json(bool preview_mode) const;

private slots:
    void add_block();

private:
    QLineEdit *title_ = nullptr;
    QList<BlockEditor *> questions_;
    QVBoxLayout *questions_layout_ = nullptr;
    QComboBox *next_section_ = nullptr;
    QStringListModel *sections_list_ = nullptr;
    QPushButton *add_block_button_ = nullptr;

    bool is_test_;
};
}  // namespace survey

#endif