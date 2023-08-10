#ifndef ECS_H
#define ECS_H

#include "../Logger/Logger.h"
#include <bitset>
#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <vector>

const unsigned int MAX_COMPONENTS = 32;

////////////////////////////////////////////////////////////////////////////////
// Signature
////////////////////////////////////////////////////////////////////////////////
// We use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.
////////////////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
protected:
    static int nextId;
};

// Used to assign a unique id to a component type
template <typename T>
class Component : public IComponent {
public:
    // Returns the unique id of Component<T>
    static int GetId() {
        static auto id = nextId++;
        return id;
    }
};

class Entity {
private:
    int id;

public:
    Entity(int id) : id(id){};
    Entity(const Entity& entity) = default;
    int GetId() const;

    Entity& operator=(const Entity& other) = default;
    bool    operator==(const Entity& other) const { return id == other.id; }
    bool    operator!=(const Entity& other) const { return id != other.id; }
    bool    operator>(const Entity& other) const { return id > other.id; }
    bool    operator<(const Entity& other) const { return id < other.id; }

    template <typename TComponent, typename... TArgs>
    void AddComponent(TArgs&&... args);
    template <typename TComponent>
    void RemoveComponent();
    template <typename TComponent>
    bool HasComponent() const;
    template <typename TComponent>
    TComponent& GetComponent() const;

    // Hold a pointer to the entity's owner registry
    class Registry* registry;
};

////////////////////////////////////////////////////////////////////////////////
// System
////////////////////////////////////////////////////////////////////////////////
// The system processes entities that contain a specific signature
////////////////////////////////////////////////////////////////////////////////
class System {
private:
    Signature           m_componentSignature;
    std::vector<Entity> m_entities;

public:
    System() = default;
    ~System() = default;

    void                AddEntityToSystem(Entity entity);
    void                RemoveEntityFromSystem(Entity entity);
    std::vector<Entity> GetSystemEntities() const;
    const Signature&    GetComponentSignature() const;

    // Defines the component type that entities must have to be considered by
    // the system
    template <typename TComponent>
    void RequireComponent();
};

////////////////////////////////////////////////////////////////////////////////
// Pool
////////////////////////////////////////////////////////////////////////////////
// A pool is just a vector (contiguous data) of objects of type T
////////////////////////////////////////////////////////////////////////////////
class IPool {
public:
    virtual ~IPool() {}
};

template <typename T>
class Pool : public IPool {
private:
    std::vector<T> m_data;

public:
    Pool(int size = 100) { m_data.resize(size); }

    virtual ~Pool() = default;

    bool isEmpty() const { return m_data.empty(); }

    int GetSize() const { return m_data.size(); }

    void Resize(int n) { m_data.resize(n); }

    void Clear() { m_data.clear(); }

    void Add(T object) { m_data.push_back(object); }

    void Set(int index, T object) { m_data[index] = object; }

    T& Get(int index) { return static_cast<T&>(m_data[index]); }

    T& operator[](unsigned int index) { return m_data[index]; }
};

////////////////////////////////////////////////////////////////////////////////
// Registry
////////////////////////////////////////////////////////////////////////////////
// The registry manages the creation and destruction of entities, add systems,
// and components.
////////////////////////////////////////////////////////////////////////////////
class Registry {
private:
    int m_numEntities = 0;

    // Vector of component pools, each pool contains all the data for a certain
    // compoenent type [Vector index = component type id] [Pool index = entity
    // id]
    std::vector<std::shared_ptr<IPool>> m_componentPools;

    // Vector of component signatures per entity, saying which component is
    // turned "on" for a given entity [Vector index = entity id]
    std::vector<Signature> m_entityComponentSignatures;

    // Map of active systems
    // [Map key = system type id]
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems;

    // Set of entities that are flagged to be added or removed in the next
    // registry Update()
    std::set<Entity> m_entitiesToBeAdded;
    std::set<Entity> m_entitiesToBeKilled;

public:
    Registry() { Logger::Log("Registry constructor called"); }

    ~Registry() { Logger::Log("Registry destructor called"); }

    // The registry Update() finally processes the entities that are waiting to
    // be added/killed to the systems
    void Update();

    // Entity management
    Entity CreateEntity();

    // Component management
    template <typename TComponent, typename... TArgs>
    void AddComponent(Entity entity, TArgs&&... args);
    template <typename TComponent>
    void RemoveComponent(Entity entity);
    template <typename TComponent>
    bool HasComponent(Entity entity) const;
    template <typename TComponent>
    TComponent& GetComponent(Entity entity) const;

    // System management
    template <typename TSystem, typename... TArgs>
    void AddSystem(TArgs&&... args);
    template <typename TSystem>
    void RemoveSystem();
    template <typename TSystem>
    bool HasSystem() const;
    template <typename TSystem>
    TSystem& GetSystem() const;

    // Checks the component signature of an entity and add the entity to the
    // systems that are interested in it
    void AddEntityToSystems(Entity entity);
};

template <typename TComponent>
void System::RequireComponent() {
    const auto componentId = Component<TComponent>::GetId();
    m_componentSignature.set(componentId);
}

template <typename TSystem, typename... TArgs>
void Registry::AddSystem(TArgs&&... args) {
    std::shared_ptr<TSystem> newSystem =
        std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    m_systems.insert(
        std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem() {
    auto system = m_systems.find(std::type_index(typeid(TSystem)));
    m_systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const {
    return m_systems.find(std::type_index(typeid(TSystem))) != m_systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
    auto system = m_systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(system->second));
}

template <typename TComponent, typename... TArgs>
void Registry::AddComponent(Entity entity, TArgs&&... args) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= static_cast<int>(m_componentPools.size())) {
        m_componentPools.resize(componentId + 1, nullptr);
    }

    if (!m_componentPools[componentId]) {
        std::shared_ptr<Pool<TComponent>> newComponentPool(
            new Pool<TComponent>());
        m_componentPools[componentId] = newComponentPool;
    }

    std::shared_ptr<Pool<TComponent>> componentPool =
        std::static_pointer_cast<Pool<TComponent>>(
            m_componentPools[componentId]);

    if (entityId >= componentPool->GetSize()) {
        componentPool->Resize(m_numEntities);
    }

    TComponent newComponent(std::forward<TArgs>(args)...);

    componentPool->Set(entityId, newComponent);

    m_entityComponentSignatures[entityId].set(componentId);

    Logger::Log("Component id = " + std::to_string(componentId) +
                " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    m_entityComponentSignatures[entityId].set(componentId, false);

    Logger::Log("Component id = " + std::to_string(componentId) +
                " was removed from entity id " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    return m_entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    auto       componentPool = std::static_pointer_cast<Pool<TComponent>>(
        m_componentPools[componentId]);
    return componentPool->Get(entityId);
}

template <typename TComponent, typename... TArgs>
void Entity::AddComponent(TArgs&&... args) {
    registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
    registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
    return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
    return registry->GetComponent<TComponent>(*this);
}

#endif // !ECS_H
