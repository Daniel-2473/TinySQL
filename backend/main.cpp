
#include "crow.h"
#include "crow/middlewares/cors.h"
#include "api/Router.h"

int main() {
    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET, crow::HTTPMethod::Options)
        .headers("Content-Type");

    Router router(app);
    router.registerRoutes();
    app.port(8080).multithreaded().run();
    return 0;
}