#ifndef INCLUDE_tgbotstater_handler_handler
#define INCLUDE_tgbotstater_handler_handler

#include "tg_stater/handler/event.hpp"
#include "tg_stater/handler/type.hpp"

namespace tg_stater {

template <concepts::Event auto Event, auto F, concepts::HandlerType auto Type = HandlerTypes::State{}>
struct Handler {
    Handler() = delete;

    static constexpr auto event = Event;
    static constexpr auto f = F;
    static constexpr auto type = Type;
};

} // namespace tg_stater

#endif // INCLUDE_tgbotstater_handler_handler
