#include <drogon/drogon.h>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace drogon;

int main(int argc, char *argv[]) {
    app().addListener("127.0.0.1", 8080);

    app().registerHandler(
        "/file",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            std::string parametr_survey_id = request->getParameter("id");
            std::filesystem::path p = std::filesystem::current_path() /
                                      "public" / parametr_survey_id / "survey" /
                                      "data.json";
            if (!std::filesystem::exists(p)) {
                p = std::filesystem::current_path() / ".." / "public" /
                    parametr_survey_id / "survey" / "data.json";
            }
            auto resp = HttpResponse::newFileResponse(p.string());
            cb(resp);
        },
        {Get}
    );

    app().registerHandler(
        "/response",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            auto json = request->getJsonObject();

            std::string survey_id = request->getParameter("id");

            auto now = std::chrono::system_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()
            )
                          .count();
            // maybe we should add hash to it, but later D:
            std::string filename = std::to_string(ts) + ".json";

            std::filesystem::path base = std::filesystem::current_path() /
                                         "public" / survey_id / "responses";
            if (!std::filesystem::exists(base)) {
                base = std::filesystem::current_path() / ".." / "public" /
                       survey_id / "responses";
            }
            std::filesystem::create_directories(base);
            std::filesystem::path out_path = base / filename;

            std::ofstream out(out_path);
            Json::StreamWriterBuilder writer;
            // for pretty json)))
            writer["indentation"] = "  ";
            out << Json::writeString(writer, *json) << std::endl;

            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Saved");
            cb(resp);
        },
        {Post}
    );

    app().run();
    return 0;
}
