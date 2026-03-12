#include <drogon/drogon.h>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace drogon;

static std::filesystem::path resolve_public_root() {
    auto root = std::filesystem::current_path() / "public";
    if (!std::filesystem::exists(root)) {
        root = std::filesystem::current_path() / ".." / "public";
    }
    return root;
}

int main(int argc, char *argv[]) {
    app().addListener("127.0.0.1", 8080);

    app().registerHandler(
        "/file",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            std::string parametr_survey_id = request->getParameter("id");
            auto root = resolve_public_root();
            std::filesystem::path p =
                root / parametr_survey_id / "survey" / "data.json";
            auto resp = HttpResponse::newFileResponse(p.string());
            cb(resp);
        },
        {Get}
    );

    // TODO: add HTTP status codes and error handlings
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

            auto root = resolve_public_root();
            std::filesystem::path base = root / survey_id / "responses";
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

    app().registerHandler(
        "/registertest",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            auto json = request->getJsonObject();

            std::string survey_id = request->getParameter("id");

            auto now = std::chrono::system_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()
            )
                          .count();
            std::filesystem::path base = std::filesystem::current_path() /
                                         "public" / survey_id / "survey";
            auto root = resolve_public_root();
            base = root / survey_id / "survey";
            std::error_code ec;
            std::filesystem::create_directories(base, ec);
            if (ec) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody("Failed to create survey directory");
                cb(resp);
                return;
            }
            std::filesystem::path out_path = base / "data.json";

            std::ofstream out(out_path);
            Json::StreamWriterBuilder writer;
            writer["indentation"] = "  ";
            out << Json::writeString(writer, *json) << std::endl;

            std::filesystem::path responses_dir =
                root / survey_id / "responses";
            std::filesystem::create_directories(responses_dir, ec);
            if (ec) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody("Failed to create responses directory");
                cb(resp);
                return;
            }

            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Saved");
            cb(resp);
        },
        {Post}
    );

    app().run();
    return 0;
}
