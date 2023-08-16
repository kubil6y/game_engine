#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

int IComponent::s_nextId = 0;

int  Entity::GetId() const { return id; }
void Entity::Kill() { registry->KillEntity(*this); }

void Entity::Tag(const std::string& tag) const {
    registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const {
    return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) const {
    registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const {
    return registry->EntityBelongsToGroup(*this, group);
}

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
    Logger::Log("Entity killed with id " + std::to_string(entity.GetId()));
    m_entitiesToBeKilled.insert(entity);
}

void Registry::TagEntity(Entity entity, const std::string& tag) {
    m_entityPerTag.emplace(tag, entity);
    m_tagPerEntity.emplace(entity.GetId(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const {
    if (m_tagPerEntity.find(entity.GetId()) == m_tagPerEntity.end()) {
        return false;
    }
    return m_entityPerTag.find(tag)->second == entity;
}

Entity Registry::GetEntityByTag(const std::string& tag) const {
    return m_entityPerTag.at(tag);
}

void Registry::RemoveEntityTag(Entity entity) {
    auto taggedEntity = m_tagPerEntity.find(entity.GetId());
    if (taggedEntity != m_tagPerEntity.end()) {
        auto tag = taggedEntity->second;
        m_entityPerTag.erase(tag);
        m_tagPerEntity.erase(taggedEntity);
    }
}

void Registry::GroupEntity(Entity entity, const std::string& group) {
    m_entitiesPerGroup.emplace(group, std::set<Entity>());
    m_entitiesPerGroup[group].emplace(entity);
    m_groupPerEntity.emplace(entity.GetId(), group);
}

bool Registry::EntityBelongsToGroup(Entity             entity,
                                    const std::string& group) const {
    if (m_entitiesPerGroup.find(group) == m_entitiesPerGroup.end()) {
        return false;
    }
    auto groupEntities = m_entitiesPerGroup.at(group);
    return groupEntities.find(entity.GetId()) != groupEntities.end();
}

std::vector<Entity>
Registry::GetEntitiesByGroup(const std::string& group) const {
    auto& setOfEntities = m_entitiesPerGroup.at(group);
    return std::vector<Entity>(setOfEntities.begin(), setOfEntities.end());
}

void Registry::RemoveEntityGroup(Entity entity) {
    // if in group, remove entity from group management
    auto groupedEntity = m_groupPerEntity.find(entity.GetId());
    if (groupedEntity != m_groupPerEntity.end()) {
        auto group = m_entitiesPerGroup.find(groupedEntity->second);
        if (group != m_entitiesPerGroup.end()) {
            auto entityInGroup = group->second.find(entity);
            if (entityInGroup != group->second.end()) {
                group->second.erase(entityInGroup);
            }
        }
        m_groupPerEntity.erase(groupedEntity);
    }
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

        // Make the entity id available to be reused
        m_freeIds.push_back(entityId);

        // Remove any trace of that entity from tag/group maps
        RemoveEntityTag(entity);
        RemoveEntityGroup(entity);
    }
    m_entitiesToBeKilled.clear();
}
