#include "main_window.hpp"
#include <QApplication>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace survey {
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}
} // namespace survey

int main(int argc, char *argv[]) {
    //in-file parsing
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/file");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, survey::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    const nlohmann::json survey_data = nlohmann::json::parse(readBuffer);

    //out-file creating
    nlohmann::json answer_data = {
        {"answer_data", {{"survey_id", 0}, {"answer_id", 0}}}, {"answers", {}}};
    answer_data["answer_data"]["survey_id"] = survey_data.at("survey_data").at("id");
    answer_data["answer_data"]["answer_id"] = 67;

    QApplication a(argc, argv);
    survey::MainWindow w(survey_data, answer_data);
    w.show();
    return a.exec();
}
