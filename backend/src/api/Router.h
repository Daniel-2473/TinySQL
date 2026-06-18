#pragma once
#include "crow.h"
#include "crow/middlewares/cors.h"
#include "handlers/QueryHandler.h"

class Router {
public:
    Router(crow::App<crow::CORSHandler>& app);
    void registerRoutes();

private:
    crow::App<crow::CORSHandler>& app;
    QueryHandler queryHandler;
};