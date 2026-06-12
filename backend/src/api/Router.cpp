// api/Router.cpp
#include "Router.h"

Router::Router(crow::SimpleApp& app) : app(app) {}

void Router::registerRoutes() {

    // Preflight CORS
    CROW_ROUTE(app, "/api/query").methods(crow::HTTPMethod::OPTIONS)
    ([](const crow::request&) {
        crow::response res(204);
        res.add_header("Access-Control-Allow-Origin",  "*");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        return res;
    });

    // Query principal
    CROW_ROUTE(app, "/api/query").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req) {
        return queryHandler.handleQuery(req);
    });
}