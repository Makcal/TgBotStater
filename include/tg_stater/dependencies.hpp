#ifndef INCLUDE_tgbotstater_handler_dependencies
#define INCLUDE_tgbotstater_handler_dependencies

namespace tg_stater {

template <typename... Deps>
class Dependencies : public Deps... {};

} // namespace tg_stater

#endif // INCLUDE_tgbotstater_handler_dependencies
