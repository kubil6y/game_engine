#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

int IComponent::nextId = 0;

int Entity::GetId() const { return id; }
void Entity::Kill() { registry->KillEntity(*this); }
void System::AddEntityToSystem(Entity entity) { m_entities.push_back(entity); }

void System::RemoveEntityFromSystem(Entity entity) {
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
                       [&entity](Entity other) { return entity == other; }),
        m_entities.end());
}

std::vector<Entity> System::GetSystemEntities() const { return m_entities; }

const Signature& System::GetComponentSignature() const {
    return m_componentSignature;
}

Entity Registry::CreateEntity() {
    int entityId;

    if (m_freeIds.empty()) {
        entityId = m_numEntities++;
        if (entityId >= static_cast<int>(m_entityComponentSignatures.size())) {
            m_entityComponentSignatures.resize(entityId + 1);
        }
    } else {
        // Reuse an id from the list of previously removed entities
        entityId = m_freeIds.front();
        m_freeIds.pop_front();
    }

    Entity entity(entityId);
    entity.registry = this;
    m_entitiesToBeAdded.insert(entity);

    Logger::Log("Entity created with id " + std::to_string(entityId));

    return entity;
}

void Registry::KillEntity(Entity entity) {
    m_entitiesToBeKilled.insert(entity);
}

void Registry::AddEntityToSystems(Entity entity) {
    const auto entityId = entity.GetId();

    const auto& entityComponentSignature =
        m_entityComponentSignatures[entityId];

    for (auto& system : m_systems) {
        const auto& systemComponentSignature =
            system.second->GetComponentSignature();

        bool isInterested =
            (entityComponentSignature & systemComponentSignature) ==
            systemComponentSignature;

        if (isInterested) {
            system.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity) {
    for (auto systemPair : m_systems) {
        systemPair.second->RemoveEntityFromSystem(entity);
    }
}

void Registry::Update() {
    // Processing the entities that are waiting to be created to the active
    // systems
    for (auto entity : m_entitiesToBeAdded) {
        AddEntityToSystems(entity);
    }
    m_entitiesToBeAdded.clear();

    // Processing the entities that are waiting to be killed from the
    // active systems
    for (auto entity : m_entitiesToBeKilled) {
        const int entityId = entity.GetId();
        RemoveEntityFromSystems(entity);

        m_entityComponentSignatures[entityId].reset();
        m_freeIds.push_back(entityId);
    }
    m_entitiesToBeKilled.clear();
}
