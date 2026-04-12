#include "created_surveys_window.hpp"
#include <matplot/matplot.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"

namespace survey {
CreatedSurveysWindow::CreatedSurveysWindow(QWidget *parent) : QDialog(parent) {
    auto *layout = new QVBoxLayout();
    nlohmann::json surveys_ids;
    try {
        surveys_ids =
            ServerInteraction::get_created_surveys(session().get_id());
    } catch (const std::exception &e) {
        show_message_box(this, QMessageBox::Warning, "Error", e.what());
        deleteLater();
        return;
    }
    if (surveys_ids.empty()) {
        layout->addWidget(new QLabel("No created surveys yet.", this));
        setLayout(layout);
        return;
    }

    for (int id : surveys_ids) {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel(QString::number(id), this));

        auto *txt_import_button = new QPushButton("Import to txt", this);
        row->addWidget(txt_import_button);
        connect(txt_import_button, &QPushButton::clicked, this, [this, id]() {
            import_statistics_txt(id);
        });

        auto *jpg_import_button = new QPushButton("Import to jpg", this);
        row->addWidget(jpg_import_button);
        connect(jpg_import_button, &QPushButton::clicked, this, [this, id]() {
            import_statistics_jpg(id);
        });

        layout->addLayout(row);
    }

    setLayout(layout);
}

void CreatedSurveysWindow::import_statistics_txt(int survey_id) {
    std::ofstream file(std::to_string(survey_id) + ".txt");
    if (!file.is_open()) {
        show_message_box(
            this, QMessageBox::Warning, "Error", "Unable to create file"
        );
        return;
    }
    nlohmann::json survey = ServerInteraction::get_survey(survey_id);
    nlohmann::json stats = ServerInteraction::get_survey_statistics(survey_id);
    file << "Total number of answers: " << stats["total_answers"] << "\n\n";
    file << "Statistic by sections:\n\n";
    for (int i = 0; i < survey["sections"].size(); ++i) {
        file << "Section " << survey["sections"][i]["title"] << ":\n\n";
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            file << "Question " << question["text"] << ":\n";
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    file << question["options"][k - 1] << ": ";
                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        file << stats["sections"][i][j][std::to_string(k)];
                    } else {
                        file << 0;
                    }
                    file << " answer(s)\n";
                }
            } else if (question["type"] == "text") {
                for (const auto &answer_count :
                     stats["sections"][i][j].items()) {
                    file << '\"' << answer_count.key()
                         << "\": " << answer_count.value() << " answer(s)\n";
                }
            }
            file << '\n';
        }
    }
    file.close();
}

void CreatedSurveysWindow::import_statistics_jpg(int survey_id) {
    using namespace matplot;

    nlohmann::json survey = ServerInteraction::get_survey(survey_id);
    nlohmann::json stats = ServerInteraction::get_survey_statistics(survey_id);

    int total_rows = 0;
    for (const auto &section : survey["sections"]) {
        total_rows += section["questions"].size();
    }

    auto f = figure(true);
    f->size(1200, 300 * total_rows);

    int plot_index = 0;
    for (int i = 0; i < survey["sections"].size(); ++i) {
        const auto &section = survey["sections"][i];
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            subplot(total_rows, 1, plot_index++);
            title(
                "[Section: " + section["title"].get<std::string>() + "] " +
                "Question: " + question["text"].get<std::string>()
            );

            std::vector<double> values;
            std::vector<std::string> labels;
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    std::string label = question["options"][k - 1];
                    if (label.size() > 20) {
                        label = label.substr(0, 17) + "...";
                    }
                    labels.push_back(label);

                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        values.push_back(
                            stats["sections"][i][j][std::to_string(k)]
                        );
                    } else {
                        values.push_back(0);
                    }
                }
            } else if (question["type"] == "text") {
                if (stats["sections"][i][j].empty()) {
                    labels.push_back("No answers yet");
                    values.push_back(0);
                } else {
                    std::vector<std::pair<std::string, int>> answers;
                    for (const auto &item : stats["sections"][i][j].items()) {
                        answers.emplace_back(item.key(), item.value());
                    }
                    std::sort(
                        answers.begin(), answers.end(),
                        [](auto &a, auto &b) { return a.second > b.second; }
                    );

                    for (int k = 0; k < std::min(10UL, answers.size()); ++k) {
                        std::string label = answers[k].first;
                        if (label.size() > 20) {
                            label = label.substr(0, 17) + "...";
                        }
                        labels.push_back(label);
                        values.push_back(answers[k].second);
                    }
                }
            }
            bar(values);

            xticks(iota(1, labels.size()));
            xticklabels(labels);

            int max_value = *std::max_element(values.begin(), values.end());
            yticks(iota(0, max_value + 1));
            ylim({0, static_cast<double>(max_value + 1)});
        }
    }
    if (!save(std::to_string(survey_id) + ".jpg")) {
        show_message_box(
            this, QMessageBox::Warning, "Error", "Unable to create file"
        );
    };
    close();
}
}  // namespace survey