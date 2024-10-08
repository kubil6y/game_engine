#include "Game.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../Events/KeyPressedEvent.h"
#include "../Logger/Logger.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/RenderSystem.h"
#include "SDL_video.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>

int Game::s_windowWidth;
int Game::s_windowHeight;
int Game::s_mapWidth;
int Game::s_mapHeight;

Game::Game() {
    m_isRunning = false;
    m_isDebug = false;
    m_registry = std::make_unique<Registry>();
    m_assetStore = std::make_unique<AssetStore>();
    m_eventBus = std::make_unique<EventBus>();
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
    s_windowWidth = 800;
    s_windowHeight = 600;

    m_window =
        SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         s_windowWidth, s_windowHeight, SDL_WINDOW_BORDERLESS);
    if (!m_window) {
        Logger::Err("Error creating SDL window.");
        return;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    if (!m_renderer) {
        Logger::Err("Error creating SDL renderer.");
        return;
    }
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_RenderSetLogicalSize(m_renderer, s_windowWidth, s_windowHeight);

    // Initialize the camera view with the entire screen area
    m_camera = {0, 0, s_windowWidth, s_windowHeight};

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
            m_eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);

            if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                m_isRunning = false;
            }
            if (sdlEvent.key.keysym.sym == SDLK_o) {
                m_isDebug = !m_isDebug;
            }
            break;
        }
    }
}

void Game::LoadLevel(int level) {
    // Add the sytems that need to be processed in our game
    m_registry->AddSystem<MovementSystem>();
    m_registry->AddSystem<RenderSystem>();
    m_registry->AddSystem<AnimationSystem>();
    m_registry->AddSystem<CollisionSystem>();
    m_registry->AddSystem<RenderColliderSystem>();
    m_registry->AddSystem<DamageSystem>();
    m_registry->AddSystem<KeyboardControlSystem>();
    m_registry->AddSystem<CameraMovementSystem>();
    m_registry->AddSystem<ProjectileEmitSystem>();
    m_registry->AddSystem<ProjectileLifecycleSystem>();

    // Adding assets to the asset store
    m_assetStore->AddTexture(m_renderer, "tank-image",
                             "./assets/images/tank-panther-right.png");
    m_assetStore->AddTexture(m_renderer, "truck-image",
                             "./assets/images/truck-ford-right.png");
    m_assetStore->AddTexture(m_renderer, "chopper-image",
                             "./assets/images/chopper-spritesheet.png");
    m_assetStore->AddTexture(m_renderer, "radar-image",
                             "./assets/images/radar.png");
    m_assetStore->AddTexture(m_renderer, "bullet-image",
                             "./assets/images/bullet.png");
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
            tile.Group("tiles");
            tile.AddComponent<TransformComponent>(
                glm::vec2(x * (tileScale * tileSize),
                          y * (tileScale * tileSize)),
                glm::vec2(tileScale, tileScale), 0.0);
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize,
                                               tileSize, 0, false, srcRectX,
                                               srcRectY);
        }
    }
    mapFile.close();

    s_mapWidth = mapNumCols * tileSize * tileScale;
    s_mapHeight = mapNumRows * tileSize * tileScale;

    // Create an entity
    Entity chopper = m_registry->CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 100.0),
                                             glm::vec2(1.0, 1.0), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 1);
    chopper.AddComponent<AnimationComponent>(2, 10, true);
    chopper.AddComponent<BoxColliderComponent>(32, 32);
    chopper.AddComponent<KeyboardControlledComponent>(
        glm::vec2(0, -120), glm::vec2(120, 0), glm::vec2(0, 120),
        glm::vec2(-120, 0));
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(150.0, 150.0), 0,
                                                     10000, 10, true);

    Entity radar = m_registry->CreateEntity();
    radar.AddComponent<TransformComponent>(
        glm::vec2(s_windowWidth - 80.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2, true);
    radar.AddComponent<AnimationComponent>(8, 5, true);

    Entity tank = m_registry->CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(500.0, 10.0),
                                          glm::vec2(1.0, 1.0), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
    tank.AddComponent<BoxColliderComponent>(32, 32);
    tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000,
                                                  3000, 10, false);
    tank.AddComponent<HealthComponent>(100);

    Entity truck = m_registry->CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0),
                                           glm::vec2(1.0, 1.0), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 2);
    truck.AddComponent<BoxColliderComponent>(32, 32);
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 100.0), 2000,
                                                   5000, 10, false);
    truck.AddComponent<HealthComponent>(100);
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

    // Store the dprevious" frame time
    m_millisecsPreviousFrame = SDL_GetTicks();

    // Reset all events handlers for the current frame
    m_eventBus->Reset();

    // Perform the subscription of the events for all systems
    m_registry->GetSystem<DamageSystem>().SubscribeToEvents(m_eventBus);
    m_registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(
        m_eventBus);
    m_registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(m_eventBus);

    // Update the registry to process the entities that are waiting to
    // be created/deleted
    m_registry->Update();

    // Invoke all the systems that needs to update
    m_registry->GetSystem<MovementSystem>().Update(deltaTime);
    m_registry->GetSystem<AnimationSystem>().Update();
    m_registry->GetSystem<CollisionSystem>().Update(m_eventBus);
    m_registry->GetSystem<CameraMovementSystem>().Update(m_camera);
    m_registry->GetSystem<ProjectileEmitSystem>().Update(m_registry);
    m_registry->GetSystem<ProjectileLifecycleSystem>().Update();
}

void Game::Render() {
    SDL_SetRenderDrawColor(m_renderer, 21, 21, 21, 255);
    SDL_RenderClear(m_renderer);

    // Invoke all the systems that need to render
    m_registry->GetSystem<RenderSystem>().Update(m_renderer, m_assetStore,
                                                 m_camera);

    if (m_isDebug) {
        m_registry->GetSystem<RenderColliderSystem>().Update(m_renderer,
                                                             m_camera);
    }

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
