// query/SemanticValidator.h
#pragma once
#include "ParsedQuery.h"
#include <string>

class SemanticValidator {
public:
    // currentDatabase es el contexto activo (SET DATABASE)
    void validate(ParsedQuery& query, const std::string& currentDatabase);

private:
    void validateCreateDatabase(ParsedQuery& q);
    void validateSetDatabase(ParsedQuery& q);
    void validateCreateTable(ParsedQuery& q, const std::string& db);
    void validateDropTable(ParsedQuery& q, const std::string& db);
    void validateCreateIndex(ParsedQuery& q, const std::string& db);
    void validateSelect(ParsedQuery& q, const std::string& db);
    void validateInsert(ParsedQuery& q, const std::string& db);
    void validateUpdate(ParsedQuery& q, const std::string& db);
    void validateDelete(ParsedQuery& q, const std::string& db);

    // Marca el query con error
    void setError(ParsedQuery& q, const std::string& msg);

    // Verifica que haya una base de datos activa
    bool requireDatabase(ParsedQuery& q, const std::string& db);
};