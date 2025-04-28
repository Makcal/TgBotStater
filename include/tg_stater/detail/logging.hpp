#ifndef INCLUDE_tg_bot_stater_detail_logging
#define INCLUDE_tg_bot_stater_detail_logging

#include <fmt/base.h>

#include <iostream>
#include <utility>
#include <version>

#if __cpp_lib_format >= 201907L
#define USE_STD_FORMAT
#include <format>
#else
#include <fmt/format.h>
#endif // __cpp_lib_format >= 201907L

namespace tg_stater::detail::logging {

#ifdef USE_STD_FORMAT
template <typename... Args>
void log(std::format_string<Args...> format, Args&&... args) {
    std::clog << std::format(format, std::forward<Args>(args)...);
}
#else
template <typename... Args>
void log(fmt::format_string<Args...> format, Args&&... args) {
    std::clog << fmt::format(format, std::forward<Args>(args)...);
}
#endif // USE_STD_FORMAT

} // namespace tg_stater::detail::logging

#endif // INCLUDE_tg_bot_stater_detail_logging
