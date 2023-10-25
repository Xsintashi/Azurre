#include "EventListener.h"
#include "fnv.h"
#include "GameData.h"
#include "Interfaces.h"
#include "Memory.h"

#include <cassert>
#include <utility>

#include "InventoryChanger/InventoryChanger.h"

#include "SDK/Engine.h"
#include "SDK/GameEvent.h"
#include "SDK/UtlVector.h"

#include "Hacks/Lobby.h"
#include "Hacks/Logger.h"
#include "Hacks/Misc.h"
#include "Hacks/ProfileChanger.h"
#include "Hacks/Visuals.h"
#include "Hacks/Walkbot.h"

namespace
{
    class EventListenerImpl : public GameEventListener {
    public:
        void fireGameEvent(GameEvent* event) override
        {
            switch (fnv::hashRuntime(event->getName())) {
            case fnv::hash("round_start"):
                GameData::clearProjectileList();
                Visuals::damageNumberEvent(event, true);
                Misc::preserveKillfeed(true);
                Misc::BuyBot(event);
                Misc::autoDefuseEvents(*event);
                Misc::FlashGame(1);
                Profile::ChangeInGame();
                //Misc::faceitSimulator(event, 0);
                [[fallthrough]];
            case fnv::hash("round_freeze_end"):
                Misc::purchaseList(event);
                break;
            case fnv::hash("round_end"):
                //Misc::faceitSimulator(event, 2);
                break;
            case fnv::hash("player_death"): {
                auto& inventoryChanger = inventory_changer::InventoryChanger::instance();
                inventoryChanger.updateStatTrak(*event);
                inventoryChanger.overrideHudIcon(*event);
                Misc::killMessage(*event);
                Misc::DeadMessage(*event);
                Misc::killSound(*event);
                Misc::killfeedChanger(*event);
                Visuals::killLightning(*event);
                break;
            }
            case fnv::hash("player_hurt"):
                Misc::playHitSound(*event);
                Visuals::damageNumberEvent(event);
                Visuals::hitEffect(event);
                Visuals::hitMarker(event);
                Log::damageLog(*event);
                Visuals::drawHitboxMatrix(event);
                //Misc::faceitSimulator(event, 1);
                break;
            case fnv::hash("vote_cast"):
                Misc::voteRevealer(*event);
                break;
            case fnv::hash("round_mvp"):
                inventory_changer::InventoryChanger::instance().onRoundMVP(*event);
                Misc::customMVPSound(*event);
                break;
            case fnv::hash("cs_win_panel_match"):
                Lobby::autoDisconnect();
                Visuals::damageNumberEvent(event, true);
                break;
            case fnv::hash("smokegrenade_detonate"):
                Misc::drawSmokeTimerEvent(event);
                break;
            case fnv::hash("molotov_detonate"):
                Misc::drawMolotovTimerEvent(event);
                break;
            case fnv::hash("bomb_dropped"):
                Log::droppedBomb(*event);
                break;
            case fnv::hash("bomb_pickup"):
                Log::pickedUpBomb(*event);
                break;
            case fnv::hash("bomb_beginplant"):
                Log::bombBeginPlaningLog(*event);
                break;
            case fnv::hash("bomb_abortplant"):
                Log::bombAbortedPlantingLog(*event);
                break;
            case fnv::hash("bomb_begindefuse"):
                Log::bombBeginDefuseLog(*event);
                break;
            case fnv::hash("bomb_abortdefuse"):
                Log::bombAbortDefuseLog(*event);
                break;
            case fnv::hash("player_ping"):
                Walkbot::event(*event);
                break;
            case fnv::hash("player_footstep"):
                Misc::footstepESP(*event);
                break;
            case fnv::hash("client_disconnect"): 
                Visuals::damageNumberEvent(event, true);
                break;
            }
        }

        static EventListenerImpl& instance() noexcept
        {
            static EventListenerImpl impl;
            return impl;
        }
    };
}

void EventListener::init() noexcept
{
    assert(interfaces);

    // If you add here listeners which aren't used by client.dll (e.g., item_purchase, bullet_impact), the cheat will be detected by AntiDLL (community anticheat).
    // Instead, register listeners dynamically and only when certain functions are enabled - see Misc::updateEventListeners(), //Visuals::updateEventListeners()

    auto gameEventManager = interfaces->gameEventManager;
    gameEventManager->addListener(&EventListenerImpl::instance(), "round_start");
    gameEventManager->addListener(&EventListenerImpl::instance(), "round_end");
    gameEventManager->addListener(&EventListenerImpl::instance(), "round_freeze_end");
    gameEventManager->addListener(&EventListenerImpl::instance(), "player_hurt");
    gameEventManager->addListener(&EventListenerImpl::instance(), "player_death");
    gameEventManager->addListener(&EventListenerImpl::instance(), "vote_cast");
    gameEventManager->addListener(&EventListenerImpl::instance(), "round_mvp");
    gameEventManager->addListener(&EventListenerImpl::instance(), "cs_win_panel_match");
    gameEventManager->addListener(&EventListenerImpl::instance(), "grenade_thrown");
    gameEventManager->addListener(&EventListenerImpl::instance(), "smokegrenade_detonate");
    gameEventManager->addListener(&EventListenerImpl::instance(), "molotov_detonate");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_dropped");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_pickup");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_beginplant");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_abortplant");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_begindefuse");
    gameEventManager->addListener(&EventListenerImpl::instance(), "bomb_abortdefuse");
    gameEventManager->addListener(&EventListenerImpl::instance(), "smokegrenade_detonate");
    gameEventManager->addListener(&EventListenerImpl::instance(), "molotov_detonate");
    gameEventManager->addListener(&EventListenerImpl::instance(), "player_footstep");
    gameEventManager->addListener(&EventListenerImpl::instance(), "player_ping");
    gameEventManager->addListener(&EventListenerImpl::instance(), "client_disconnect");

    // Move our player_death listener to the first position to override killfeed icons (InventoryChanger::overrideHudIcon()) before HUD gets them
    if (const auto desc = memory->getEventDescriptor(gameEventManager, "player_death", nullptr))
        std::swap(desc->listeners[0], desc->listeners[desc->listeners.size - 1]);
    else
        assert(false);

    // Move our round_mvp listener to the first position to override event data (InventoryChanger::onRoundMVP()) before HUD gets them
    if (const auto desc = memory->getEventDescriptor(gameEventManager, "round_mvp", nullptr))
        std::swap(desc->listeners[0], desc->listeners[desc->listeners.size - 1]);
    else
        assert(false);
}

void EventListener::remove() noexcept
{
    assert(interfaces);

    interfaces->gameEventManager->removeListener(&EventListenerImpl::instance());
}
