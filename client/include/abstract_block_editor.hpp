#ifndef ABSTRACT_BLOCK_EDITOR_HPP_
#define ABSTRACT_BLOCK_EDITOR_HPP_
#include <qimage.h>
#include <qobject.h>
#include <qpixmap.h>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <functional>
#include <nlohmann/json.hpp>
#include "enums.hpp"
#include "pretty_view.hpp"

namespace survey {
class BlockEditor : public QWidget {
    Q_OBJECT

public:
    BlockEditor(SurveyType type, QWidget *parent)
        : QWidget(parent), type_(type) {
    }

    virtual ~BlockEditor() = default;
    virtual void to_json(
        bool preview_mode,
        std::function<void(const nlohmann::json &)> success,
        std::function<void(const std::string &)> failure
    ) const = 0;

signals:
    void remove_requested(BlockEditor *editor);
    void move_up_requested(BlockEditor *editor);
    void move_down_requested(BlockEditor *editor);

protected:
    SurveyType type_;
    QLineEdit *question_ = nullptr;
    QPushButton *upload_image_button_ = nullptr;
    QLabel *image_preview_ = nullptr;
    QString image_name_;
    QByteArray image_data_;

protected slots:

    void upload_image() {
        QFileDialog::getOpenFileContent(
            "Images (*.jpg *.jpeg)",
            [&](const QString &file_name, const QByteArray &file_content) {
                if (file_name.isEmpty()) {
                    return;
                }
                if (file_content.size() > 1024 * 1024) {
                    show_message_box(
                        this, QMessageBox::Warning, "Error", "File size too big"
                    );
                    return;
                }
                image_name_ = file_name;
                image_data_ = file_content;

                QImage image = QImage::fromData(file_content)
                                   .scaled(
                                       500, 500, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation
                                   );
                image_preview_->setPixmap(QPixmap::fromImage(image));
            }
        );
    }
};
}  // namespace survey

#endif
