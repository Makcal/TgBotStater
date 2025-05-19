# TgBotStater
A C++ library for constructing Telegram bots in compile-time!

# Installation
The recommended way of connecting the library is via Conan package manager. To be updated.

# Usage
## Basic example of syntax
See [test.cpp](test_package/test.cpp)

## Handlers
All handlers are constexpr [_structural_](https://en.cppreference.com/w/cpp/language/template_parameters) lambdas or similar objects.
If you need some runtime dependecies, pass them through `Dependecies` template type parameter of `Setup` and `Stater`'s constructor argument.

Valid handler signatures:
 * from `void(const EventArgs&...)`
 * up to `void(StateOption&, const EventArgs&..., const TgBot::Api&, const StateStorageProxy&, const Dependencies&)`

Everything except for `EventArgs` is optional.
Handlers that are not bound to a specific state can't take state parameter.
For the lists of `EventArgs` see `CallbackArgs` member at [event.hpp](include/tg_stater/handler/event.hpp)

> [!NOTE]
> To get `chatId` or an event's similar target id, you'd better use `StateProxy.getKey()`.
> But it's also possible to reference `message.chat->...` objects in message handlers as the chat's id is used for chat/user recognition.
> Other handler types have similar things, but you'd better check the source code of [event.hpp](include/tg_stater/handler/event.hpp) for that information.

## Reference to a state

> [!IMPORTANT]
> When you put a new state object into a storage, the old one most probably dies and its reference is invalidated.
> Be very careful with this! See an example:
```cpp
[](StateA& state, const Message& m, const StateProxy& stateManager) {
    std::println("{}", state.foo); // OK
    stateManager.put(StateB{});
    std::println("{}", state.foo); // UB, the reference is dangling
}
```
