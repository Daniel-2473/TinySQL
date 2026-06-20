#include "api/handlers/QueryHandler.h"
#include "crow.h"
#include "query/Lexer.h"
#include "query/Parser.h"
#include "query/SemanticValidator.h"
#include "SystemCatalog.h"
#include "StoredDataManager.h"

// currentDatabase persiste entre consultas (contexto activo)
std::string QueryHandler::currentDatabase = "";

crow::response QueryHandler::handleQuery(const crow::request& req) {
    auto json_entrante = crow::json::load(req.body);
    if (!json_entrante) {
        QueryResult error;
        error.success = false;
        error.message = "JSON inválido";
        crow::response res(400, toJson(error));
        addCorsHeaders(res);
        return res;
    }

    std::string query    = json_entrante["query"].s();
    std::string clientDb = json_entrante["currentDatabase"].s();

    // 1. Lexer
    Lexer lexer(query);
    std::vector<Token> tokens = lexer.tokenize();

    // 2. Parser
    Parser parser(tokens);
    ParsedQuery parsed = parser.parse();

    // 3. Validación semántica
    SemanticValidator validator;
    validator.validate(parsed, currentDatabase);

    // 4. Si hay error de sintaxis o semántica, devolver a React
    if (parsed.hasError) {
        QueryResult error;
        error.success = false;
        error.message = parsed.errorMessage;
        crow::response res(toJson(error));
        addCorsHeaders(res);
        return res;
    }

    // 5. Ejecutar con StoredDataManager
    StoredDataManager sdm;
    QueryResult result;

    switch (parsed.type) {

        case QueryType::CREATE_DATABASE:
            result = sdm.execCreateDatabase(parsed);
            break;

        case QueryType::SET_DATABASE:
            // Solo actualizamos el contexto; la validación semántica ya verificó que existe
            currentDatabase = parsed.databaseName;
            result.success  = true;
            result.message  = "Base de datos activa: " + currentDatabase;
            break;

        case QueryType::CREATE_TABLE:
            result = sdm.execCreateTable(parsed, currentDatabase);
            break;

        case QueryType::DROP_TABLE:
            result = sdm.execDropTable(parsed, currentDatabase);
            break;

        case QueryType::CREATE_INDEX:
            result = sdm.execCreateIndex(parsed, currentDatabase);
            break;

        case QueryType::INSERT:
            result = sdm.execInsert(parsed, currentDatabase);
            break;

        case QueryType::SELECT:
            result = sdm.execSelect(parsed, currentDatabase);
            break;

        case QueryType::UPDATE:
            result = sdm.execUpdate(parsed, currentDatabase);
            break;

        case QueryType::DELETE:
            result = sdm.execDelete(parsed, currentDatabase);
            break;

        default:
            result.success = false;
            result.message = "Tipo de consulta no soportado.";
            break;
    }

    crow::response res(toJson(result));
    addCorsHeaders(res);
    return res;
}

crow::json::wvalue QueryHandler::toJson(const QueryResult& result) {
    crow::json::wvalue json;
    json["success"] = result.success;
    json["message"] = result.message;

    if (!result.columns.empty()) {
        crow::json::wvalue cols(crow::json::type::List);
        for (size_t i = 0; i < result.columns.size(); i++)
            cols[i] = result.columns[i];
        json["columns"] = std::move(cols);
    }

    if (!result.rows.empty()) {
        crow::json::wvalue rows(crow::json::type::List);
        for (size_t i = 0; i < result.rows.size(); i++) {
            crow::json::wvalue row(crow::json::type::List);
            for (size_t j = 0; j < result.rows[i].size(); j++)
                row[j] = result.rows[i][j];
            rows[i] = std::move(row);
        }
        json["rows"] = std::move(rows);
    }

    return json;
}

void QueryHandler::addCorsHeaders(crow::response& res) {
    res.add_header("Access-Control-Allow-Origin",  "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}
