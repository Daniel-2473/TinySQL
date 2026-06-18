#pragma once
#include <string>
#include <vector>

struct QueryResult {
    bool        success;
    std::string message;
    double      executionTimeMs;

    std::vector<std::string>              columns;
    std::vector<std::vector<std::string>> rows;
};