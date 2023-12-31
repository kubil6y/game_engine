#ifndef ASSET_STORE_H
#define ASSET_STORE_H

#include <SDL2/SDL.h>
#include <map>
#include <string>

class AssetStore {
private:
    std::map<std::string, SDL_Texture*> m_textures;
    // TODO: create a map for fonts
    // TODO: create a map for audio

public:
    AssetStore();
    ~AssetStore();

    void         ClearAssets();
    void         AddTexture(SDL_Renderer* renderer, const std::string& assetId,
                            const std::string& filePath);
    SDL_Texture* GetTexture(const std::string& assetId);
};

#endif // !ASSET_STORE_H
