#ifndef GAME_H
#define GAME_H

#include "../AssetStore/AssetStore.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include <SDL2/SDL.h>

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
private:
    bool          m_isRunning;
    bool          m_isDebug;
    int           m_millisecsPreviousFrame = 0;
    SDL_Window*   m_window;
    SDL_Renderer* m_renderer;
    SDL_Rect      m_camera;

    std::unique_ptr<EventBus>   m_eventBus;
    std::unique_ptr<Registry>   m_registry;
    std::unique_ptr<AssetStore> m_assetStore;

public:
    Game();
    ~Game();
    void Initialize();
    void Run();
    void Setup();
    void LoadLevel(int level);
    void ProcessInput();
    void Update();
    void Render();
    void Destroy();

    static int windowWidth;
    static int windowHeight;
    static int mapWidth;
    static int mapHeight;
};

#endif // !GAME_H
