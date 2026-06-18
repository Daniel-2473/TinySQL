#include "crow.h"
#include "SystemCatalog.h"
#include "BTree.h"
#include "Quicksort.h"
#include "src/api/Router.h"

int main() {
    initializeSystemCatalog();
    loadIndexes();
    crow::SimpleApp app;
    Router router(app);
    router.registerRoutes();
    app.port(8080).multithreaded().run();
}