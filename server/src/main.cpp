#include <drogon/HttpTypes.h>
#include <drogon/drogon.h>
#include <database.hpp>

using namespace drogon;

// TODO: maybe not throw raw error to client, but just write it in debug mode +
// log?

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
#ifdef YAZ_DEBUG
                std::cerr << "Error reading survey "
                          << request->getParameter("id") << ": " << e.what()
                          << std::endl;
#endif
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
#ifdef YAZ_DEBUG
                std::cerr << "Received survey: " << json->toStyledString()
                          << std::endl;
#endif
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error saving survey "
                          << request->getParameter("id") << ": " << e.what()
                          << std::endl;
#endif
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
#ifdef YAZ_DEBUG
                std::cerr << "Received answer: " << json->toStyledString()
                          << std::endl;
#endif
            } catch (const std::exception &e) {
#ifdef YAZ_DEBUG
                std::cerr << "Error saving answer: " << e.what() << std::endl;
#endif
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
#ifdef YAZ_DEBUG
                std::cerr << "Error reading passed surveys for "
                          << request->getParameter("session-id") << ": "
                          << e.what() << std::endl;
#endif
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
#ifdef YAZ_DEBUG
                std::cerr << "Error reading survey results for "
                          << request->getParameter("session-id")
                          << " and survey "
                          << request->getParameter("survey-id") << ": "
                          << e.what() << std::endl;
#endif
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
            }
        },
        {Get}
    );

    app().registerHandler(
        "/check",
        [&db](
            const HttpRequestPtr &request,
            std::function<void(const HttpResponsePtr &)> &&cb
        ) {
            auto resp = HttpResponse::newHttpResponse();
            std::string out;
            try {
                auto user_answers = request->getJsonObject();
                out = db.get_result(user_answers->toStyledString());
            } catch (const std::exception &e) {
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(e.what());
                cb(resp);
                return;
            }
            resp->setContentTypeCode(CT_APPLICATION_JSON);
            resp->setBody(out);
            cb(resp);
        },
        {Post}
    );

    app().run();
    return 0;
}
