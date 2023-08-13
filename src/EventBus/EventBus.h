#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include "../Events/Event.h"
#include "../Logger/Logger.h"
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <typeindex>

class IEventCallback {
private:
    virtual void Call(Event& e) = 0; // pure virtual method

public:
    virtual ~IEventCallback() = default;
    void Execute(Event& e) { Call(e); };
};

template <typename TOwner, typename TEvent>
class EventCallback : public IEventCallback {
private:
    typedef void (TOwner::*CallbackFunction)(TEvent&);

    TOwner*          m_ownerInstance;
    CallbackFunction m_callbackFunction;

    virtual void Call(Event& e) override {
        std::invoke(m_callbackFunction, m_ownerInstance,
                    static_cast<TEvent&>(e));
    };

public:
    EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction) {
        m_ownerInstance = ownerInstance;
        m_callbackFunction = callbackFunction;
    }

    virtual ~EventCallback() override = default;
};

using HandlerList = std::list<std::unique_ptr<IEventCallback>>;

class EventBus {
private:
    std::map<std::type_index, std::unique_ptr<HandlerList>> m_subscribers;

public:
    EventBus() { Logger::Log("EventBus constructor called!"); }
    ~EventBus() { Logger::Log("EventBus destructor called!"); }

    // Clears the subscriber list
    void Reset() {
        m_subscribers.clear();
    }

    // Example: eventBus->SubscribeToEvent<CollisionEvent>(this,
    // &Game::onCollision);
    template <typename TEvent, typename TOwner>
    void SubscribeToEvent(TOwner* ownerInstance,
                          void (TOwner::*callbackFunction)(TEvent&)) {
        if (!m_subscribers[typeid(TEvent)].get()) {
            m_subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
        }
        auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(
            ownerInstance, callbackFunction);
        m_subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
    }

    // Example: eventBus->EmitEvent<CollisionEvent>(player, enemy);
    template <typename TEvent, typename... TArgs>
    void EmitEvent(TArgs&&... args) {
        auto handlers = m_subscribers[typeid(TEvent)].get();
        if (handlers) {
            for (auto it = handlers->begin(); it != handlers->end(); it++) {
                auto   handler = it->get();
                TEvent event(std::forward<TArgs>(args)...);
                handler->Execute(event);
            }
        }
    }
};

#endif // !EVENT_BUS_H
