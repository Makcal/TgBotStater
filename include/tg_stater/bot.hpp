#ifndef INCLUDE_tgbotstater_bot
#define INCLUDE_tgbotstater_bot

#include "tg_stater/dependencies.hpp"
#include "tg_stater/detail/logging.hpp"
#include "tg_stater/handler/callback.hpp"
#include "tg_stater/handler/event.hpp"
#include "tg_stater/handler/type.hpp"
#include "tg_stater/meta.hpp"
#include "tg_stater/state.hpp"
#include "tg_stater/state_storage/common.hpp"
#include "tg_stater/state_storage/memory.hpp"

#if !defined(TGBOTSTATER_LOGGING_OFF) && !defined(TGBOTSTATER_NOT_DEMANGLE_TYPES)
#include <boost/core/demangle.hpp>
#endif
#include <tgbot/Api.h>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>
#include <tgbot/types/Message.h>

#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace tg_stater {

namespace detail {

template <concepts::State StateT, concepts::StateStorage<StateT> StateStorageT, typename Dependencies, typename Handler>
struct HandlerValidator {
  private:
    using FT = std::decay_t<decltype(Handler::f)>;
    using FirstArgT = std::remove_cvref_t<typename meta::function_traits<FT>::template ArgT<0>>;
    static constexpr bool takesState = concepts::StateOption<FirstArgT, StateT>;

  public:
    using CallbackT = Callback<Handler::f,
                               StateT,
                               std::conditional_t<takesState, FirstArgT, NulloptStateOption>,
                               Handler::event,
                               Handler::type,
                               StateStorageT,
                               Dependencies>;
};

// CheckIsCallback
template <typename T, typename StateT, typename StateStorageT, typename DependenciesT>
struct CheckIsCallback : std::false_type {};

// CheckIsCallback
template <auto F,
          concepts::State StateT,
          concepts::OptionalStateOption<StateT> StateOptionT,
          concepts::Event auto Event,
          concepts::HandlerType auto HandlerType,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT>
struct CheckIsCallback<Callback<F, StateT, StateOptionT, Event, HandlerType, StateStorageT, DependenciesT>,
                       StateT,
                       StateStorageT,
                       DependenciesT> : std::true_type {};

struct CallbackFinder {
  private:
    template <auto F, typename... Callbacks>
    struct CallbackFilter;

    template <auto F, typename Callback, typename... Callbacks>
    struct CallbackFilter<F, Callback, Callbacks...> {
      private:
        static constexpr bool match = F.template operator()<Callback>();
        using other = CallbackFilter<F, Callbacks...>::type;

      public:
        using type =
            std::conditional_t<match,
                               decltype(std::tuple_cat(std::declval<std::tuple<Callback>>(), std::declval<other>())),
                               other>;
    };

    template <auto F>
    struct CallbackFilter<F> {
        using type = std::tuple<>;
    };

  public:
    CallbackFinder() = delete;

    static constexpr auto noStateFilter = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::NoState>;
    };
    static constexpr auto anyStateFilter = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::AnyState>;
    };
    template <typename StateOptionT>
    static constexpr auto filterByStateOption = []<typename Callback>() {
        return std::same_as<typename Callback::TypeT, HandlerTypes::State> &&
               std::same_as<StateOptionT, typename Callback::StateOption>;
    };
    template <typename EventT>
    static constexpr auto filterByEventType =
        []<typename Callback>() { return std::same_as<EventT, typename Callback::EventT>; };

    template <auto Filter, typename... Callbacks>
    using find = CallbackFilter<Filter, Callbacks...>::type;
};

// Main implementation class.
template <concepts::State StateT,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT,
          typename... Callbacks>
    requires(detail::CheckIsCallback<Callbacks, StateT, StateStorageT, DependenciesT>::value && ...)
class StaterBase {
    StateStorageT stateStorage;
    DependenciesT dependencies;

    // Helper function to invoke handlers
    template <typename StateOption, typename Callback, typename... Args>
    static constexpr void invokeCallback(Args&&... args) {
#if !defined(TGBOTSTATER_NOT_DEMANGLE_TYPES) and !defined(TGBOTSTATER_LOGGING_OFF)
        const std::string handlerNameWrapped =
            boost::core::demangle(typeid(meta::ValueProxy<Callback::underlying>).name());

        using namespace std::literals;
        static constexpr std::size_t demangledSkip = "tg_stater::meta::ValueProxy<"sv.size();
        std::string_view handlerName = handlerNameWrapped;
        handlerName = handlerName.substr(demangledSkip, handlerName.size() - demangledSkip - 1); // last is '>'
        if (handlerName.starts_with('&'))
            handlerName = handlerName.substr(1);
#ifndef TGBOTSTATER_FULL_DEMANGLE
        std::size_t qualifiedNameLength = handlerName.find('{'); // for lambdas
        handlerName = handlerName.substr(handlerName.substr(0, qualifiedNameLength).rfind("::") + 2);
#endif
#else
        const char* handlerName = typeid(meta::ValueProxy<Callback::underlying>).name();
#endif
        if constexpr (std::is_void_v<StateOption>)
            logging::log("No state. Running handler {}\n", handlerName);
        else {
#if !defined(TGBOTSTATER_NOT_DEMANGLE_TYPES) and !defined(TGBOTSTATER_LOGGING_OFF)
#ifndef TGBOTSTATER_FULL_DEMANGLE
            const std::string stateNameFull = boost::core::demangle(typeid(StateOption).name());
            std::string_view stateName = stateNameFull;
            stateName = stateName.substr(stateName.rfind("::") + 2);
#else
            const std::string stateName = boost::core::demangle(typeid(StateOption).name());
#endif
#else
            const char* stateName = typeid(StateOption).name();
#endif
            logging::log("Current state is {}. Running handler {}\n", stateName, handlerName);
        }
        Callback::func(std::forward<Args>(args)...);
    }

    // Helper function to invoke handlers
    template <typename StateOption, typename... Callbacks_, typename... Args>
    static constexpr void invokeCallbacks(meta::Proxy<Callbacks_...> /*unused*/, Args&&... args) noexcept {
        try {
            (invokeCallback<StateOption, Callbacks_>(std::forward<Args>(args)...), ...);
        } catch (const std::exception& e) {
            logging::log("Caught exception in handler: {}\n", e.what());
        } catch (...) {
            logging::log("Non-std::exception exception was caught\n");
        }
    }

    template <typename... EventCallbacks, typename... EventArgs>
    void handleEvent(TgBot::Bot& bot, const StateKey& stateKey, EventArgs... eventArgs) {
        const TgBot::Api& api = bot.getApi();
        StateT* const mCurrentState = stateStorage[stateKey];
        const StateProxy stateProxy{stateStorage, stateKey};

        if (mCurrentState) {
            StateT& currentStateRef = *mCurrentState;
            std::visit(
                [&](auto& state) {
                    using StateOption = std::remove_reference_t<decltype(state)>;
                    constexpr auto stateFilter = CallbackFinder::filterByStateOption<StateOption>;
                    using StateCallbacks = CallbackFinder::find<stateFilter, EventCallbacks...>;
                    invokeCallbacks<StateOption>(
                        meta::TupleToProxy<StateCallbacks>{}, state, eventArgs..., api, stateProxy, dependencies);
                },
                currentStateRef);
        } else {
            // no state handlers
            using NoStateCallbacks = CallbackFinder::find<CallbackFinder::noStateFilter, EventCallbacks...>;
            invokeCallbacks<void>(meta::TupleToProxy<NoStateCallbacks>{}, eventArgs..., api, stateProxy, dependencies);
        }
        // any state handlers
        using AnyStateCallbacks = CallbackFinder::find<CallbackFinder::anyStateFilter, EventCallbacks...>;
        invokeCallbacks<void>(meta::TupleToProxy<AnyStateCallbacks>{}, eventArgs..., api, stateProxy, dependencies);
    }

    template <typename... EventCallbacks, typename... Args>
    void handleEventProxy(meta::Proxy<EventCallbacks...> /*unused*/, Args&&... args) {
        handleEvent<EventCallbacks...>(std::forward<Args>(args)...);
    }

    static void logEvent(std::string_view event_type, const StateKey& key) {
        detail::logging::log("{}: Trying to handle {} from {}\n", std::chrono::system_clock::now(), event_type, key);
    }

    template <typename Callbacks_>
    auto makeMessageHandler(std::string_view event, TgBot::Bot& bot) {
        return [&, event](const TgBot::Message::Ptr& mp) {
            const StateKey key = EventCategories::Message::getStateKey(mp);
            logEvent(event, key);
            handleEventProxy(meta::TupleToProxy<Callbacks_>{}, bot, key, *mp);
        };
    }

    template <typename EventT>
    using FindEventCallbacks = CallbackFinder::find<CallbackFinder::filterByEventType<EventT>, Callbacks...>;

    template <typename T>
    struct GroupCommandCallbacks;

    template <typename... CommandCallbacks>
    struct GroupCommandCallbacks<std::tuple<CommandCallbacks...>> {
        using type =
            meta::group_by<[]<typename C>() { return std::string_view{C::event.command}; }, CommandCallbacks...>;
    };

    void setup(TgBot::Bot& bot) {
        bot.getEvents().onNonCommandMessage(
            makeMessageHandler<FindEventCallbacks<Events::Message>>("non-command message", bot));
        using Grouped = GroupCommandCallbacks<FindEventCallbacks<Events::Command>>::type;
        [&]<typename... Groups>(meta::Proxy<Groups...>) {
            (bot.getEvents().onCommand({std::tuple_element_t<0, Groups>::event.command},
                                       makeMessageHandler<Groups>("command", bot)),
             ...);
        }(meta::TupleToProxy<Grouped>{});
        bot.getEvents().onUnknownCommand(
            makeMessageHandler<FindEventCallbacks<Events::UnknownCommand>>("unknown command", bot));
        bot.getEvents().onAnyMessage(makeMessageHandler<FindEventCallbacks<Events::AnyMessage>>("message", bot));
        bot.getEvents().onEditedMessage(
            makeMessageHandler<FindEventCallbacks<Events::EditedMessage>>("message edit", bot));
        bot.getEvents().onInlineQuery([&](const TgBot::InlineQuery::Ptr& iqp) {
            const StateKey key = EventCategories::InlineQuery::getStateKey(iqp);
            logEvent("inline query", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::InlineQuery>>{}, bot, key, *iqp);
        });
        bot.getEvents().onChosenInlineResult([&](const TgBot::ChosenInlineResult::Ptr& cirp) {
            const StateKey key = EventCategories::ChosenInlineResult::getStateKey(cirp);
            logEvent("chosen inline result", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChosenInlineResult>>{}, bot, key, *cirp);
        });
        bot.getEvents().onCallbackQuery([&](const TgBot::CallbackQuery::Ptr& cqp) {
            const StateKey key = EventCategories::CallbackQuery::getStateKey(cqp);
            logEvent("callback query", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::CallbackQuery>>{}, bot, key, *cqp);
        });
        bot.getEvents().onShippingQuery([&](const TgBot::ShippingQuery::Ptr& sqp) {
            const StateKey key = EventCategories::ShippingQuery::getStateKey(sqp);
            logEvent("shipping query", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ShippingQuery>>{}, bot, key, *sqp);
        });
        bot.getEvents().onPreCheckoutQuery([&](const TgBot::PreCheckoutQuery::Ptr& pcqp) {
            const StateKey key = EventCategories::PreCheckoutQuery::getStateKey(pcqp);
            logEvent("precheckout query", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::PreCheckoutQuery>>{}, bot, key, *pcqp);
        });
        bot.getEvents().onPollAnswer([&](const TgBot::PollAnswer::Ptr& pap) {
            const StateKey key = EventCategories::PollAnswer::getStateKey(pap);
            logEvent("poll answer", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::PollAnswer>>{}, bot, key, *pap);
        });
        bot.getEvents().onMyChatMember([&](const TgBot::ChatMemberUpdated::Ptr& cmup) {
            const StateKey key = EventCategories::ChatMemberUpdated::getStateKey(cmup);
            logEvent("chat member update for bot", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::MyChatMember>>{}, bot, key, *cmup);
        });
        bot.getEvents().onChatMember([&](const TgBot::ChatMemberUpdated::Ptr& cmup) {
            const StateKey key = EventCategories::ChatMemberUpdated::getStateKey(cmup);
            logEvent("chat member update", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChatMember>>{}, bot, key, *cmup);
        });
        bot.getEvents().onChatJoinRequest([&](const TgBot::ChatJoinRequest::Ptr& cjrp) {
            const StateKey key = EventCategories::ChatJoinRequest::getStateKey(cjrp);
            logEvent("chat join request", key);
            handleEventProxy(meta::TupleToProxy<FindEventCallbacks<Events::ChatJoinRequest>>{}, bot, key, *cjrp);
        });
    }

  public:
    explicit constexpr StaterBase(StateStorageT stateStorage = StateStorageT{},
                                  DependenciesT dependencies = DependenciesT{})
        : stateStorage{std::move(stateStorage)}, dependencies{std::move(dependencies)} {}

    using UpdatesList = std::vector<std::string>;

    // rvalue reference means taking the ownership
    void start(TgBot::Bot&& bot,          // NOLINT(*-rvalue-reference-param-not-moved)
               std::int32_t limit = 100,  // NOLINT(*-magic-numbers)
               std::int32_t timeout = 10, // NOLINT(*-magic-numbers)
               const std::shared_ptr<UpdatesList>& allowedUpdates = std::make_shared<UpdatesList>()) {
        setup(bot);

        detail::logging::log("Bot has started at https://t.me/{}\n", bot.getApi().getMe()->username);
#if !defined(TGBOTSTATER_LOGGING_OFF) && defined(TGBOTSTATER_NOT_DEMANGLE_TYPES)
        detail::logging::log("Debug type names can be demangled with `c++filt -t`\n");
#endif
        TgBot::TgLongPoll longPoll{bot, limit, timeout, allowedUpdates};
        while (true) {
            try {
                longPoll.start();
            } catch (const std::exception& e) {
                detail::logging::log("{}\n", e.what());
            }
        }
    }
};

} // namespace detail

// Converts each `Handler` to a `Callback`
template <concepts::State StateT,
          concepts::StateStorage<StateT> StateStorageT,
          typename DependenciesT,
          typename... Handlers>
using Stater =
    detail::StaterBase<StateT,
                       StateStorageT,
                       DependenciesT,
                       typename detail::HandlerValidator<StateT, StateStorageT, DependenciesT, Handlers>::CallbackT...>;

template <typename StateT, typename Dependencies = Dependencies<>, typename StateStorageT = MemoryStateStorage<StateT>>
struct Setup {
    template <typename... Handlers>
    using Stater = Stater<StateT, StateStorageT, Dependencies, Handlers...>;
};

template <typename StateT, typename... Handlers>
using DefaultStater = Setup<StateT>::template Stater<Handlers...>;

} // namespace tg_stater
#endif // INCLUDE_tgbotstater_bot
