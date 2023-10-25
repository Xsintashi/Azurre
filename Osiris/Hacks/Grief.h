#pragma once

#include "../JsonForward.h"

struct ImDrawList;
struct UserCmd;

namespace Fun {
    bool unmuteAllPlayers() noexcept;
    bool getDisconnect() noexcept;
    const char* getDisconnectMessage() noexcept;
    void prepareNamesSpam() noexcept;
    bool changeName(bool reconnect, const char* newName, float delay) noexcept;
    void changeMyName(bool set) noexcept;
    void stealNames() noexcept;
    void repeatSomeoneMessage(const void*& data, int& size) noexcept;
    void blockBot(UserCmd* cmd) noexcept;

    void ChatSpammer() noexcept;




    // GUI
    void menuBarItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;

}