#pragma once
#include "crow.h"
#include "models/QueryResult.h"

class QueryHandler {
public:
    crow::response handleQuery(const crow::request& req);

private:
    crow::json::wvalue toJson(const QueryResult& result);
    void addCorsHeaders(crow::response& res);
};