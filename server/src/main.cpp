#include <drogon/drogon.h>
#include <chrono>
#include <database.hpp>
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
            std::filesystem::path p = Database::resolve_public_root() /
                                      parametr_survey_id / "survey" /
                                      "data.json";
            if (!std::filesystem::exists(p)) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody("Survey not found");
                cb(resp);
                return;
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
            try {
                Database::write_response(
                    nlohmann::json::parse(json->toStyledString()),
                    std::stoi(survey_id)
                );
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }

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

            try {
                Database::register_test(
                    survey_id, nlohmann::json::parse(json->toStyledString())
                );
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }

            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Saved");
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/getpassed",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            std::string session_id = request->getParameter("session_id");
            nlohmann::json passed_surveys;
            try {
                passed_surveys = Database::send_passed_surveys(session_id);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            nlohmann::json passed_ids = nlohmann::json::array();
            for (const auto &tuple : passed_surveys) {
                if (tuple.is_array() && tuple.size() >= 1) {
                    passed_ids.push_back(tuple.at(0));
                }
            }
            auto resp = HttpResponse::newHttpResponse();

            resp->setBody(passed_ids.dump(2));
            cb(resp);
        },
        {Get}
    );

    app().run();
    return 0;
}
