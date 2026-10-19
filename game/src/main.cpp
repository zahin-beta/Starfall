#include "Game.h"
#include <cstring>
int main(int argc, char** argv) {
    Game game;
    if (argc > 1 && std::strcmp(argv[1], "--bench") == 0) {
        game.RunBenchmark();
        return 0;
    }
    game.Run();
    return 0;
}
