#include "crow.h"
#include "crow/middlewares/cors.h"
#include "api/Router.h"
#include "SystemCatalog.h"
#include <filesystem>
#include <iostream>

int main() {
    // Fijar el directorio de trabajo al directorio donde está main.cpp (raíz del backend)
    std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
    std::filesystem::path workDir = exePath.parent_path().parent_path(); // sube de build/ a backend/
    std::filesystem::current_path(workDir);
    std::cout << "Directorio de trabajo: " << std::filesystem::current_path() << std::endl;

    // Inicializar el SystemCatalog (crea las carpetas y archivos .bin si no existen)
    initializeSystemCatalog();
    loadIndexes();

    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("*")
        .methods(crow::HTTPMethod::POST, crow::HTTPMethod::GET, crow::HTTPMethod::Options)
        .headers("Content-Type");

    Router router(app);
    router.registerRoutes();
    app.port(8080).multithreaded().run();
    return 0;
}