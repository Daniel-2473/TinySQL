#include "crow.h"

int main() {
    crow::SimpleApp app;

    // Ruta básica
    CROW_ROUTE(app, "/")([]{
        return "Crow está funcionando!";
    });

    // Ruta con parámetro
    CROW_ROUTE(app, "/hola/<string>")([](std::string nombre){
        return "Hola, " + nombre + "!";
    });

    // Ruta que devuelve JSON
    CROW_ROUTE(app, "/json")([]{
        crow::json::wvalue data;
        data["status"] = "ok";
        data["framework"] = "Crow";
        data["version"] = "1.0";
        return data;
    });

    app.port(8080).multithreaded().run();
}