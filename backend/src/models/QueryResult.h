#pragma once
#include <string>
#include <vector>

struct QueryResult {
    bool        success = false;
    std::string message;
    double      executionTimeMs = 0.0;
    std::vector<std::string>              columns;
    std::vector<std::vector<std::string>> rows;
};