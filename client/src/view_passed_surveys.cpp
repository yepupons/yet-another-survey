#include "view_passed_surveys.hpp"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace survey {
ViewPassedSurveys::ViewPassedSurveys(
    const QStringList &survey_ids,
    QWidget *parent
)
    : QDialog(parent) {
    setWindowTitle("Passed surveys");

    auto *layout = new QVBoxLayout(this);
    if (survey_ids.isEmpty()) {
        layout->addWidget(new QLabel("No passed surveys yet.", this));
        setLayout(layout);
        return;
    }

    for (const auto &id : survey_ids) {
        auto *button = new QPushButton(id, this);
        layout->addWidget(button);
    }

    setLayout(layout);
}
}  // namespace survey
