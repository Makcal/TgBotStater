#ifndef INCLUDE_tgbotstater_detail_logging
#define INCLUDE_tgbotstater_detail_logging

#include "tg_stater/meta.hpp"

#if !(defined(TGBOTSTATER_LOGGING_OFF) || defined(TGBOTSTATER_NOT_DEMANGLE_TYPES))
#include <boost/core/demangle.hpp>
#endif

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#if __cpp_lib_format >= 201907L
#define USE_STD_FORMAT
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

template <typename Callback>
auto getHandlerName() {
#ifndef TGBOTSTATER_NOT_DEMANGLE_TYPES
    std::string nameWrapped = boost::core::demangle(typeid(meta::ValueProxy<Callback::underlying>).name());

    using namespace std::literals;
    static constexpr std::size_t demangledSkip = "tg_stater::meta::ValueProxy<"sv.size();

    const char* start = nameWrapped.c_str();
    const char* end = nameWrapped.c_str() + nameWrapped.size();

    start += demangledSkip;
    end--; // extra '>' at the end

    if (*start == '&')
        start++;

#ifndef TGBOTSTATER_FULL_TYPE_NAMES
    std::string_view fullName = std::string_view{start, end};
    if (std::size_t braceStart = fullName.find('{'); braceStart == -1) {
        // function pointer
        if (fullName[0] == '(') {
            // global namespace
            std::size_t secondPar = fullName.find('(', 1);
            if (secondPar != -1) {
                end = start + secondPar;
                start++;
            }
        } else {
            // some namespace
            std::size_t namespacesLength = fullName.rfind("::") + 2;
            if (namespacesLength != -1)
                start += namespacesLength;
        }
    } else {
        // lambda
        end = start + braceStart - 2;
        std::size_t lastNamespaceSep = std::string_view{start, end}.rfind("::");
        if (lastNamespaceSep != -1)
            start += lastNamespaceSep + 2;
        nameWrapped.erase(0, static_cast<std::size_t>(start - nameWrapped.c_str()));
        nameWrapped.erase(static_cast<std::size_t>(end - start));
        return "lambda from " + std::move(nameWrapped);
    }
#endif

    nameWrapped.erase(0, static_cast<std::size_t>(start - nameWrapped.c_str()));
    nameWrapped.erase(static_cast<std::size_t>(end - start));
    return nameWrapped;
#else
    return typeid(meta::ValueProxy<Callback::underlying>).name();
#endif
}

template <typename StateOption>
auto getStateName() {
#ifndef TGBOTSTATER_NOT_DEMANGLE_TYPES
#ifndef TGBOTSTATER_FULL_TYPE_NAMES
    std::string fullName = boost::core::demangle(typeid(StateOption).name());
    fullName.erase(0, fullName.rfind("::") + 2);
    return fullName;
#else
    return boost::core::demangle(typeid(StateOption).name());
#endif
#else
    return typeid(StateOption).name();
#endif
}

} // namespace tg_stater::detail::logging

#endif // INCLUDE_tgbotstater_detail_logging
