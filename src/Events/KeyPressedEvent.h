#ifndef KEY_PRESSED_EVENT_H
#define KEY_PRESSED_EVENT_H

#include "../Events/Event.h"
#include <SDL2/SDL.h>

class KeyPressedEvent : public Event {
public:
    SDL_Keycode symbol;
    KeyPressedEvent(SDL_Keycode symbol) : symbol(symbol) {}
};

#endif // !KEY_PRESSED_EVENT_H
