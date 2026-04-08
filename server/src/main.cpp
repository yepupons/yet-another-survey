#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <database.hpp>

using namespace drogon;

int main(int argc, char *argv[]) {
    survey::Database db;
    app().addListener("127.0.0.1", 8080);

    app().registerHandler(
        "/survey",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                int survey_id = std::stoi(request->getParameter("id"));
                const auto survey_data = db.read_survey(survey_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(survey_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/survey",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            try {
                auto json = request->getJsonObject();
                db.write_survey(json->toStyledString());
            } catch (const std::exception &e) {
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setBody("Saved");
            cb(resp);
        },
        {Post}
    );

    app().registerHandler(
        "/answer",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            try {
                auto json = request->getJsonObject();
                db.write_answer(json->toStyledString());
            } catch (const std::exception &e) {
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setBody("Saved");
            cb(resp);
        },
        {Post}
    );

    /*
    app().registerHandler(
        "/answer",
        [&db](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            try {
                int answer_id = std::stoi(request->getParameter("id"));
                const auto answer_data = db.read_answer(answer_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(answer_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k404NotFound);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );
    */

    app().registerHandler(
        "/passed-surveys",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                int session_id = std::stoi(request->getParameter("session-id"));
                const auto passed_surveys_data =
                    db.read_passed_surveys(session_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(passed_surveys_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/survey-results",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            try {
                // ?session-id=...&survey-id=...
                int session_id = std::stoi(request->getParameter("session-id"));
                int survey_id = std::stoi(request->getParameter("survey-id"));
                const auto survey_results_data =
                    db.read_survey_results(session_id, survey_id);
                auto resp = HttpResponse::newHttpResponse();
                resp->setContentTypeCode(CT_APPLICATION_JSON);
                resp->setBody(survey_results_data);
                cb(resp);
            } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().run();
    return 0;
}
