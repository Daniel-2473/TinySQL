#include "src/api/Router.h"

int main() {
    crow::SimpleApp app;
    Router router(app);
    router.registerRoutes();
    app.port(8080).multithreaded().run();
}