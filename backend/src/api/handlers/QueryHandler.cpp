#include "QueryHandler.h"

crow::response QueryHandler::handleQuery(const crow::request& req) {
    auto json_entrante = crow::json::load(req.body);

    if (!json_entrante) {
        QueryResult error;
        error.success        = false;
        error.message        = "JSON inválido";
        error.executionTimeMs = 0;

        crow::response res(400, toJson(error));
        addCorsHeaders(res);
        return res;
    }

    std::string query     = json_entrante["query"].s();
    std::string currentDb = json_entrante["currentDatabase"].s();

    // Mock por ahora
    QueryResult result;
    result.success         = true;
    result.message         = "OK";
    result.executionTimeMs = 4.32;

    // Llenar columnas manualmente
    result.columns[0] = "Carnet";
    result.columns[1] = "Nombre";
    result.columnCount = 2;

    // Llenar filas manualmente
    result.rows[0][0] = "20260102";
    result.rows[0][1] = "Estudiante Alfa";
    result.rows[1][0] = "20260304";
    result.rows[1][1] = "Estudiante Beta";
    result.rowCount   = 2;

    crow::response res(toJson(result));
    addCorsHeaders(res);
    return res;
}

crow::json::wvalue QueryHandler::toJson(const QueryResult& result) {
    crow::json::wvalue json;
    json["status"]          = result.success ? "success" : "error";
    json["message"]         = result.message;
    json["executionTimeMs"] = result.executionTimeMs;

    // Usar columnCount en vez de .size()
    for (int i = 0; i < result.columnCount; i++)
        json["columns"][i] = result.columns[i];

    // Usar rowCount y columnCount en vez de .size()
    for (int i = 0; i < result.rowCount; i++)
        for (int j = 0; j < result.columnCount; j++)
            json["data"][i][j] = result.rows[i][j];

    return json;
}

void QueryHandler::addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin",  "*");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
}