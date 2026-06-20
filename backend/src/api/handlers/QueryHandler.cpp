#include "api/handlers/QueryHandler.h"  
#include "crow.h"                      
#include "query/Lexer.h"
#include "query/Parser.h"
#include "query/SemanticValidator.h"
#include "SystemCatalog.h"

// El currentDatabase se guarda como atributo de la clase
// porque persiste entre consultas
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

    std::string query     = json_entrante["query"].s();
    std::string clientDb  = json_entrante["currentDatabase"].s();

    // 1. Lexer
    Lexer lexer(query);
    std::vector<Token> tokens = lexer.tokenize();

    // 2. Parser
    Parser parser(tokens);
    ParsedQuery parsed = parser.parse();

    // 3. Validación semántica
    SemanticValidator validator;
    validator.validate(parsed, currentDatabase);

    // 4. Si hay error, devolver a React
    if (parsed.hasError) {
        QueryResult error;
        error.success = false;
        error.message = parsed.errorMessage;
        crow::response res(toJson(error));
        addCorsHeaders(res);
        return res;
    }

    // 5. Si es SET DATABASE, actualizar el contexto
    if (parsed.type == QueryType::SET_DATABASE) {
        currentDatabase = parsed.databaseName;
        QueryResult result;
        result.success = true;
        result.message = "Base de datos activa: " + currentDatabase;
        crow::response res(toJson(result));
        addCorsHeaders(res);
        return res;
    }

    // 6. Aquí irá la ejecución real con StoredDataManager
    QueryResult result;
    result.success = true;
    result.message = "Consulta válida. Ejecución pendiente.";
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