#include <drogon/drogon.h>
#include <filesystem>

using namespace drogon;

int main(int argc, char *argv[]) {
    app().addListener("127.0.0.1", 8080);

    app().registerHandler(
        "/file",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&cb) {
            std::filesystem::path p =
                std::filesystem::current_path() / "public" / "sample.json";
            if (!std::filesystem::exists(p)) {
                p = std::filesystem::current_path() / ".." / "public" /
                    "sample.json";
            }
            auto resp = HttpResponse::newFileResponse(p.string());

            // works only if you start from root, code aboce searches for file
            // auto resp =
            // HttpResponse::newFileResponse("./public/sample.json");
            cb(resp);
        },
        {Get}
    );

    app().run();
    return 0;
}
