#include "Game.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../Logger/Logger.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>

Game::Game() {
    m_isRunning = false;
    m_registry = std::make_unique<Registry>();
    m_assetStore = std::make_unique<AssetStore>();
    Logger::Log("Game constructor called!");
}

Game::~Game() { Logger::Log("Game destructor called!"); }

void Game::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Err("Error initializing SDL.");
        return;
    }
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    windowWidth = displayMode.w;
    windowHeight = displayMode.h;
    m_window =
        SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         windowWidth, windowHeight, SDL_WINDOW_BORDERLESS);
    if (!m_window) {
        Logger::Err("Error creating SDL window.");
        return;
    }
    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) {
        Logger::Err("Error creating SDL renderer.");
        return;
    }
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
    m_isRunning = true;
}

void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
        case SDL_QUIT:
            m_isRunning = false;
            break;
        case SDL_KEYDOWN:
            if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                m_isRunning = false;
            }
            break;
        }
    }
}

void Game::LoadLevel(int level) {
    // Add the sytems that need to be processed in our game
    m_registry->AddSystem<MovementSystem>();
    m_registry->AddSystem<RenderSystem>();

    // Adding assets to the asset store
    m_assetStore->AddTexture(m_renderer, "tank-image",
                             "./assets/images/tank-panther-right.png");
    m_assetStore->AddTexture(m_renderer, "truck-image",
                             "./assets/images/truck-ford-right.png");
    m_assetStore->AddTexture(m_renderer, "tilemap-image",
                             "./assets/tilemaps/jungle.png");

    // Load the tilemap
    int    tileSize = 32;
    double tileScale = 2.0;
    int    mapNumCols = 25;
    int    mapNumRows = 20;

    std::fstream mapFile;
    mapFile.open("./assets/tilemaps/jungle.map");

    for (int y = 0; y < mapNumRows; y++) {
        for (int x = 0; x < mapNumCols; x++) {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = m_registry->CreateEntity();
            tile.AddComponent<TransformComponent>(
                glm::vec2(x * (tileScale * tileSize),
                          y * (tileScale * tileSize)),
                glm::vec2(tileScale, tileScale), 0.0);
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize,
                                               tileSize, srcRectX, srcRectY);
        }
    }
    mapFile.close();

    // Create an entity
    Entity tank = m_registry->CreateEntity();
    tank.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0),
                                          glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(40.0, 0.0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32);

    Entity truck = m_registry->CreateEntity();
    truck.AddComponent<TransformComponent>(glm::vec2(50.0, 100.0),
                                           glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(20.0, 0.0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32);
}

void Game::Setup() { LoadLevel(1); }

void Game::Update() {
    // If we are too fast, waste some time until we reach the
    // MILLISECS_PER_FRAME
    int timeToWait =
        MILLISECS_PER_FRAME - (SDL_GetTicks() - m_millisecsPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        SDL_Delay(timeToWait);
    }

    // The difference in ticks since the last frame, converted to seconds
    double deltaTime = (SDL_GetTicks() - m_millisecsPreviousFrame) / 1000.0;

    // Store the "previous" frame time
    m_millisecsPreviousFrame = SDL_GetTicks();

    // Update the registry to process the entities that are waiting to be
    // created/deleted
    m_registry->Update();

    // Invoke all the systems that need to update
    m_registry->GetSystem<MovementSystem>().Update(deltaTime);
}

void Game::Render() {
    SDL_SetRenderDrawColor(m_renderer, 21, 21, 21, 255);
    SDL_RenderClear(m_renderer);

    // Invoke all the systems that need to render
    m_registry->GetSystem<RenderSystem>().Update(m_renderer, m_assetStore);

    SDL_RenderPresent(m_renderer);
}

void Game::Run() {
    Setup();
    while (m_isRunning) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::Destroy() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
