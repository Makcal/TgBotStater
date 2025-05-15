#ifndef INCLUDE_tgbotstater_state_storage_memory
#define INCLUDE_tgbotstater_state_storage_memory

#include "tg_stater/meta.hpp"
#include "tg_stater/state.hpp"
#include "tg_stater/state_storage/common.hpp"

#include <type_traits>
#include <unordered_map>
#include <utility>

namespace tg_stater {

template <concepts::State StateT_>
class MemoryStateStorage {
    std::unordered_map<StateKey, StateT_> states;

  public:
    using StateT = StateT_;

    [[nodiscard]] StateT* operator[](const StateKey& key) {
        auto it = states.find(key);
        return it == states.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const StateT* operator[](const StateKey& key) const {
        auto it = states.find(key);
        return it == states.end() ? nullptr : &it->second;
    }

    void erase(const StateKey& key) {
        if (auto it = states.find(key); it != states.end())
            states.erase(it);
    }

    template <typename T>
        requires meta::is_part_of_variant<std::remove_cvref_t<T>, StateT>
    StateT& put(const StateKey& key, T&& state) {
        return states.insert_or_assign(key, std::forward<T>(state)).first->second;
    }
};
} // namespace tg_stater

#endif // INCLUDE_tgbotstater_state_storage_memory
