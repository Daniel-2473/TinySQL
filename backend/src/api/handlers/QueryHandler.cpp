#include "QueryHandler.h"

crow::response QueryHandler::handleQuery(const crow::request& req) {
    auto json_entrante = crow::json::load(req.body);
    std::cout << "JSON recibido: " << req.body << std::endl; // Debug: imprimir el JSON recibido
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
    result.columns         = {"Carnet", "Nombre"};
    result.rows            = {
        {"20260102", "Estudiante Alfa"},
        {"20260304", "Estudiante Beta"}
    };

    crow::response res(toJson(result));
    addCorsHeaders(res);
    return res;
}

crow::json::wvalue QueryHandler::toJson(const QueryResult& result) {
    crow::json::wvalue json;
    json["status"]          = result.success ? "success" : "error";
    json["message"]         = result.message;
    json["executionTimeMs"] = result.executionTimeMs;

    for (int i = 0; i < result.columns.size(); i++)
        json["columns"][i] = result.columns[i];

    for (int i = 0; i < result.rows.size(); i++)
        for (int j = 0; j < result.rows[i].size(); j++)
            json["data"][i][j] = result.rows[i][j];

    return json;
}

void QueryHandler::addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin",  "*");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    res.add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
}