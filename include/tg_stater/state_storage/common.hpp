#ifndef INCLUDE_tgbotstater_state_storage_common
#define INCLUDE_tgbotstater_state_storage_common

#include "tg_stater/logging.hpp"
#include "tg_stater/meta.hpp"
#include "tg_stater/state.hpp"
#include "tg_stater/tg_types.hpp"

#ifdef TGBOTSTATER_USE_STD_FORMAT
#include <format>
#else
#include <fmt/format.h>
#endif // TGBOTSTATER_USE_STD_FORMAT

#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>
#include <utility>

namespace tg_stater {

struct StateKey {
    // If there is no chat, fallback to userId.
    // If the bot needs to distinguish users in a chat, the developer can implement it by themself.
    ChatUserIdT chatId;
    std::optional<ThreadIdT> threadId = std::nullopt;

    bool operator==(StateKey o) const {
        return chatId == o.chatId && threadId == o.threadId;
    }
};

namespace concepts {

namespace detail {

template <typename StateStorageT, typename T>
struct CanBePutIntoStorage {
    static constexpr bool value = requires(StateStorageT& s, const StateKey& key) {
        { s.put(key, std::declval<const T&>()) } -> std::same_as<typename StateStorageT::StateT&>;
        { s.put(key, std::declval<T &&>()) } -> std::same_as<typename StateStorageT::StateT&>;
    };
};

} // namespace detail

template <typename T, typename StateT>
concept StateStorage = State<StateT> && requires(T& s, const StateKey& key) {
    typename T::StateT;
    requires std::same_as<typename T::StateT, StateT>;

    { s[key] } -> std::same_as<StateT*>; // pointer here represents a non-const nullable reference (optional<StateT&>)
    { s.erase(key) } -> std::same_as<void>;
    requires meta::check_for_each_in_variant<StateT, meta::curry<detail::CanBePutIntoStorage, T>::template type>;
};

} // namespace concepts

template <typename StorageT_>
    requires concepts::StateStorage<StorageT_, typename StorageT_::StateT>
class StateProxy {
  public:
    using StateT = StorageT_::StateT;
    using StorageT = StorageT_;

  private:
    std::reference_wrapper<StorageT> storage;
    StateKey key;

  public:
    explicit StateProxy(StorageT& storage, const StateKey& key) : storage{storage}, key{key} {}

    [[nodiscard]] const StateKey& getKey() const {
        return key;
    }

    [[nodiscard]] StateT* get() const {
        return storage.get()[key];
    }

    [[nodiscard]] StateT* operator()() const {
        return get();
    }

    void erase() const {
        storage.get().erase(key);
    }

    template <typename T>
        requires meta::is_part_of_variant<std::remove_cvref_t<T>, StateT>
    StateT& put(T&& state) const {
        return storage.get().put(key, std::forward<T>(state));
    }
};

} // namespace tg_stater

template <>
struct std::hash<tg_stater::StateKey> {
    std::size_t operator()(const tg_stater::StateKey& key) const {
        std::size_t h1 = hash<decltype(key.chatId)>{}(key.chatId);
        std::size_t h2 = hash<decltype(key.threadId)>{}(key.threadId);
        return h1 ^ (h2 << 1U);
    }
};

#ifdef TGBOTSTATER_USE_STD_FORMAT
template <>
struct std::formatter<tg_stater::StateKey> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
            throw std::format_error("Only empty format args are allowed for StateKey.");
        return it;
    }

    template <class FmtContext>
    FmtContext::iterator format(const tg_stater::StateKey& key, FmtContext& ctx) const {
        std::ostringstream out;
        out << "{chatId=" << key.chatId;
        if (key.threadId) {
            out << ", threadId=" << *key.threadId;
        }
        out << '}';
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};
#else
template <>
struct fmt::formatter<tg_stater::StateKey> : formatter<string_view> {
    format_context::iterator format(tg_stater::StateKey key, format_context& ctx) const {
        std::ostringstream out;
        out << "{chatId=" << key.chatId;
        if (key.threadId) {
            out << ", threadId=" << *key.threadId;
        }
        out << '}';
        return formatter<string_view>::format(std::move(out).str(), ctx);
    }
};
#endif

#endif // INCLUDE_tgbotstater_state_storage_common
