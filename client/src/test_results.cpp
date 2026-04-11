#include "test_results.hpp"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>

namespace survey {
ViewTestResults::ViewTestResults(nlohmann::json &results, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Test results");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *scroll_area = new QScrollArea(this);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);

    auto *content = new QWidget(scroll_area);
    content->setObjectName("centralWidget");
    auto *content_layout = new QVBoxLayout(content);
    content_layout->setAlignment(Qt::AlignTop);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(16);
    nlohmann::json &results_json = results;

    for (size_t section_indx = 0;
         section_indx < results_json.at("sections").size(); section_indx++) {
        auto *section_title = new QLabel(
            QString("Section ") + QString::number(section_indx + 1), content
        );
        section_title->setObjectName("titleLabel");
        content_layout->addWidget(section_title);
        for (size_t question_indx = 0;
             question_indx <
             results_json.at("sections").at(section_indx).size();
             question_indx++) {
            QString status = results_json.at("sections")
                                     .at(section_indx)
                                     .at(question_indx)
                                     .get<int>()
                                 ? QString("Correct!")
                                 : QString("Wrong!");
            auto *question_result = new QLabel(
                QString("Вопрос ") + QString::number(question_indx + 1) +
                    QString(": ") + status,
                content
            );
            content_layout->addWidget(question_result);
        }
    }
    content_layout->addStretch();
    scroll_area->setWidget(content);
    layout->addWidget(scroll_area);
    setLayout(layout);
}
}  // namespace survey
