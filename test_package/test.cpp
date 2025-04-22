#include <tg_stater/bot.hpp>
#include <tgbot/Bot.h>

#include <variant>
#include <vector>

struct StateA {
    std::vector<int> v;
};

struct StateB {
    int x;
};

using State = std::variant<StateA, StateB>;

int main() {
    using namespace tg_stater;

    TgBot::Bot bot{""};

    Setup<State>::Stater<> stater{};
    // stater.start(std::move(bot));
}
