#pragma once
#include "crow.h"
#include "handlers/QueryHandler.h"

class Router {
public:
    Router(crow::SimpleApp& app);
    void registerRoutes();

private:
    crow::SimpleApp& app;
    QueryHandler     queryHandler;
};