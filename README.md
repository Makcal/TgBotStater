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
