#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        // TODO logging
        return 1;
    }

    const SDL_MessageBoxData messageBox =
    {
        .title = "Hello",
        .message = "World"
    };

    int buttonID;
    SDL_ShowMessageBox(&messageBox, &buttonID);

    SDL_Quit();
    return 0;
}
