#ifndef INCLUDE_tgbotstater_detail_logging
#define INCLUDE_tgbotstater_detail_logging

#include <iostream>
#include <utility>
#include <version>

#if __cpp_lib_format >= 201907L
#define USE_STD_FORMAT
#include <format>
#else
#include <fmt/base.h>
#include <fmt/format.h>
#endif // __cpp_lib_format >= 201907L

namespace tg_stater::detail::logging {

#ifdef USE_STD_FORMAT
template <typename... Args>
void log(std::format_string<Args...> format, Args&&... args) {
#ifndef TGBOTSTATER_LOGGING_OFF
    std::clog << std::format(format, std::forward<Args>(args)...);
#endif // TGBOTSTATER_LOGGING_OFF
}
#else
template <typename... Args>
void log(fmt::format_string<Args...> format, Args&&... args) {
#ifndef TGBOTSTATER_LOGGING_OFF
    std::clog << fmt::format(format, std::forward<Args>(args)...);
#endif // TGBOTSTATER_LOGGING_OFF
}
#endif // USE_STD_FORMAT

} // namespace tg_stater::detail::logging

#endif // INCLUDE_tgbotstater_detail_logging
