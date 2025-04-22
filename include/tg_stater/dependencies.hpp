#ifndef INCLUDE_tg_bot_stater_handler_dependencies
#define INCLUDE_tg_bot_stater_handler_dependencies

namespace tg_stater {

template <typename... Deps>
class Dependencies : public Deps... {};

} // namespace tg_stater

#endif // INCLUDE_tg_bot_stater_handler_dependencies
