#ifndef INCLUDE_tgbotstater_detail_logging
#define INCLUDE_tgbotstater_detail_logging

#include <iostream>
#include <utility>
#include <version>

#if __cpp_lib_format >= 201907L
#define FMT_NAMESPACE std
#include <format>
#else
#define FMT_NAMESPACE fmt
#include <fmt/base.h>
#include <fmt/format.h>
#endif // __cpp_lib_format >= 201907L

namespace tg_stater::detail::logging {

template <typename... Args>
void log(FMT_NAMESPACE::format_string<Args...> format, Args&&... args) {
#ifndef TGBOTSTATER_LOGGING_OFF
    std::clog << FMT_NAMESPACE::format(format, std::forward<Args>(args)...);
#endif // TGBOTSTATER_LOGGING_OFF
}

} // namespace tg_stater::detail::logging

#endif // INCLUDE_tgbotstater_detail_logging
