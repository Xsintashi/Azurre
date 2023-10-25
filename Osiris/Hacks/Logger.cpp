#include "Logger.h"
#include "Notifications.h"

struct EventLog {
    int type = 0;
    bool mode = false; // Notification : Chat
    Color4 colorNotification;

    //damageLog 0
    //bombBeginPlantedLog 1
    //bombAbortPlantedLog 2
    //bombDropped 3
    //bombPickedUp 4
    //bombBeginDefuseLog 5
    //bombAbortDefuseLog 6
    //itemPickup 7
} logs;

Color4 Log::LogColor() noexcept {
    return logs.colorNotification;
}

void Log::DrawGUI() noexcept {
    ImGui::SetNextItemWidth(100.f);
    ImGuiCustom::multiCombo("Logger", logs.type, "Damage\0Bomb Planting\0Bomb Plant Aborting\0Dropped Bomb\0Picked bomb\0Bomb Defusing\0Bomb Defuse Aborting\0Picking Item\0");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.f);
    ImGuiCustom::boolCombo("Type", logs.mode, "Chat\0Notification\0");
    ImGuiCustom::colorPicker("Notification Color", logs.colorNotification);
}

void Log::damageLog(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 0))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));
    const auto attacker = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("attacker")));

    if (!userID) return;

    if (!attacker) return;

    if (localPlayer->getUserId() != attacker->getUserId() && localPlayer->getUserId() != userID->getUserId()) return;

    const auto damage = event.getInt("dmg_health");
    const std::vector hitgroups = { "Generic", "Head", "Chest", "Stomach", "Left Arm", "Right Arm", "Left Leg", "Right Leg", "", "", "Gear" };

    std::string information = "";

    if (logs.mode) {
        information += attacker->getPlayerName().c_str();
        information += " hit ";
        information += userID->getPlayerName().c_str();
        information += " for ";
        information += std::to_string(damage).c_str();
        information += " in the ";
        information += hitgroups[event.getInt("hitgroup")];

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isAttacker = attacker == localPlayer.get();
    const auto isUserID = userID == localPlayer.get();
    const auto colour = isAttacker ? "\x06" : (isUserID ? "\x06" : "\x07");


    information += colour;
    information += attacker->getPlayerName().c_str();
    information += "\x01 hit ";
    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 for ";
    information += colour;
    information += std::to_string(damage).c_str();
    information += "\x01 in the ";
    information += colour;
    information += hitgroups[event.getInt("hitgroup")];

    Helpers::CheatChatNotification(false, information.c_str());
}

void Log::droppedBomb(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 3))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    std::string information = "";

    if (logs.mode) {
        information += userID->getPlayerName().c_str();
        information += " dropped the bomb";

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";

    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 dropped the bomb";
    information += colour;

    Helpers::CheatChatNotification(false, information.c_str());

}

void Log::pickedUpBomb(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 4))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    std::string information = "";

    if (logs.mode) {
        information += userID->getPlayerName().c_str();
        information += " picked up the bomb";

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";

    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 picked up the bomb";
    information += colour;

    Helpers::CheatChatNotification(false, information.c_str());

}

void Log::bombBeginPlaningLog(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 1))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    const auto sites = event.getInt("site") == 454 ? "A Site" : "B Site";

    std::string information = "";

    if (logs.mode) {
        information += userID->getPlayerName().c_str();
        information += " is planting the bomb at ";
        information += sites;

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";

    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 is planting the bomb at ";
    information += colour;
    information += sites;
    
    Helpers::CheatChatNotification(false, information.c_str());
    
}
void Log::bombAbortedPlantingLog(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 2))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    const auto site = event.getInt("site") == 454 ? "A Site" : "B Site";

    std::string information = "";

    if (logs.mode) {
        information += userID->getPlayerName().c_str();
        information += " stopped planting the bomb at ";
        information += site;

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";

    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 stopped planting the bomb at ";
    information += colour;
    information += site;

    Helpers::CheatChatNotification(false, information.c_str());

}

void Log::bombBeginDefuseLog(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 5))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    if (!localPlayer) return;

    const auto hasKit = event.getInt("haskit") == 1 ? " with a kit" : "";

    std::string information = "";

    if (logs.mode) {
        
        information += userID->getPlayerName().c_str();
        information += " is defusing the bomb";
        information += hasKit;

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }
    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";


    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 is defusing the bomb";
    information += colour;
    information += hasKit;

    Helpers::CheatChatNotification(false, information.c_str());
}
void Log::bombAbortDefuseLog(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 6))
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    const auto hasKit = event.getInt("haskit") == 1 ? " with a kit" : "";

    std::string information = "";

    if (logs.mode) {
        
        information += userID->getPlayerName().c_str();
        information += " stopped defusing the bomb";
        information += hasKit;

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }
    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";


    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 stopped defusing the bomb";
    information += colour;
    information += hasKit;

    Helpers::CheatChatNotification(false, information.c_str());
}

void Log::pickedUpItem(GameEvent& event) noexcept {

    if (!logs.type)
        return;

    if (!localPlayer) return;

    if (!Helpers::getByteFromBytewise(logs.type, 7))
        return;

    if ((*memory->gameRules)->freezePeriod())
        return;

    const auto userID = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event.getInt("userid")));

    if (!userID ) return;

    const char* item = event.getString("item");
    const bool silent = event.getBool("silent");

    std::string information = "";

    if (logs.mode) {
        information += userID->getPlayerName().c_str();
        information += " picked up ";
        if (silent) information += "silently ";
        information += "the ";
        information += item;

        Notifications::addNotification(logs.colorNotification, information.c_str());
        return;
    }

    const auto isUserID = userID == localPlayer.get();
    const auto colour = isUserID ? "\x06" : "\x07";

    information += colour;
    information += userID->getPlayerName().c_str();
    information += "\x01 picked up ";
    if (silent) information += "silently ";
    information += "the ";
    information += "\xC";
    information += item;

    Helpers::CheatChatNotification(false, information.c_str());
}

static void from_json(const json& j, EventLog& l) {
    read(j, "Type", l.type);
}

static void to_json(json& j, const EventLog& o, const EventLog& dummy = {}) {
    WRITE("Type", type);
}

json Log::toJson() noexcept
{
    json j;
    to_json(j, logs);
    return j;
}

void Log::fromJson(const json& j) noexcept
{
    from_json(j, logs);
}

void Log::resetConfig() noexcept
{
    logs = {};
}