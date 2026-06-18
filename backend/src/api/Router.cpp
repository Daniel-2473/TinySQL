#include "Router.h"
#include <iostream>

Router::Router(crow::App<crow::CORSHandler>& app) : app(app) {}

void Router::registerRoutes() {
    CROW_ROUTE(app, "/api/query")
        .methods(crow::HTTPMethod::POST, crow::HTTPMethod::Options)
    ([this](const crow::request& req) {
        std::cout << "Método recibido: " << crow::method_name(req.method) << std::endl;

        if (req.method == crow::HTTPMethod::Options) {
            crow::response res(204);
            res.add_header("Access-Control-Allow-Origin",  "*");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            return res;
        }
        return queryHandler.handleQuery(req);
    });
}