#ifndef ABSTRACT_BLOCK_EDITOR_HPP_
#define ABSTRACT_BLOCK_EDITOR_HPP_
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "pretty_view.hpp"

namespace survey {
class BlockEditor : public QWidget {
    Q_OBJECT

public:
    BlockEditor(bool is_test, QWidget *parent)
        : QWidget(parent), is_test_(is_test) {
    }

    virtual ~BlockEditor() = default;
    virtual nlohmann::json to_json(bool preview_mode) const = 0;

signals:
    void remove_requested(BlockEditor *editor);

protected:
    bool is_test_;
    QLineEdit *question_ = nullptr;
    QPushButton *upload_image_button_ = nullptr;
    QLabel *image_preview_ = nullptr;
    QString image_path_;

protected slots:

    void upload_image() {
        QString file_path = QFileDialog::getOpenFileName(
            this, "Choose Image", "", "Images (*.jpg *.jpeg)"
        );
        if (file_path.isEmpty()) {
            return;
        }
        QFileInfo file_info(file_path);
        if (file_info.size() > 5 * 1024 * 1024) {
            show_message_box(
                this, QMessageBox::Warning, "Error", "File size too big"
            );
            return;
        }
        image_path_ = file_path;
        QPixmap pixmap = QPixmap(file_path).scaled(
            700, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation
        );
        image_preview_->setPixmap(pixmap);
    }
};
}  // namespace survey

#endif
