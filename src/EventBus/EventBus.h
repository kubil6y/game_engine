#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include "../Logger/Logger.h"
#include <map>
#include <memory>
#include <typeindex>
#include <vector>

class EventBus {
private:
    //std::map < std::type_index, std::unique_ptr<HandlerList>> m_subscribers;

public:
    EventBus() { Logger::Log("EventBus constructor called!"); }
    ~EventBus() { Logger::Log("EventBus destructor called!"); }

    template <typename TEvent>
    void SubscribeToEvent() {
        // TODO:
    }

    void EmitEvent() {
        // TODO:
    }
};

#endif // !EVENT_BUS_H
