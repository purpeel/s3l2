#include "consoleApp.hpp"
#include "BPlusTree.hpp"

int main() {
    using App = VFSConsoleApp<BPlusTree>; 
    App app;

    App::showStart();
    while (true) {
        app.showCurrent();
        try {
            app.execute( App::awaitInput() );
        } catch (ExitSignal& sig) {
            break;
        } catch (Exception& ex) {
            App::showError( ex );
            continue;
        }
    }

    return 0;    
}