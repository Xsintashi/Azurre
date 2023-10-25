#include "Notifications.h"
#include "../Helpers.h"

// https://github.com/seksea/gamesneeze/blob/master/src/core/features/notifications.cpp

struct Notification {
    float openTime;
    Color4 color;
    char text[128];
};

std::vector<Notification> notifications = {};

void Notifications::draw(ImDrawList* drawList) {
    float yOffset = 0.f;
    for (Notification& notif : notifications) {
        float slideValue = memory->globalVars->realtime - (notif.openTime + 8.f);
        ImColor shadowColor = ImColor(0, 0, 0, 255);
        notif.color.color[3] = 1.f - (slideValue * 3.f);
        shadowColor.Value.w = 1.f - (slideValue * 3.f);
        //static
        drawList->AddText(ImVec2((slideValue > 0.f) ? 4.f - slideValue * 800.f : 4.f, yOffset + 1.f), shadowColor, std::string("azurre | ").append(notif.text).c_str());
        //sliding out
        drawList->AddText(ImVec2((slideValue > 0.f) ? 3.f - slideValue * 800.f : 3.f, yOffset), Helpers::calculateColor(notif.color), std::string("azurre | ").append(notif.text).c_str());

        yOffset += 13;

        if (notif.color.color[3] < 0.f) {
            notifications.erase(notifications.begin());
        }
    }
    while (notifications.size() > 8) {
        notifications.erase(notifications.begin());
    }
}

void Notifications::addNotification(Color4 color, const char* fmt, ...) {
    Notification notif;
    notif.openTime = memory->globalVars->realtime;
    notif.color = color;

    va_list args;
    va_start(args, fmt);
    vsnprintf(notif.text, sizeof(Notification::text), fmt, args);
    va_end(args);

    notifications.push_back(notif);
    Helpers::CheatConsoleNotification(color, std::string(fmt).append("\n").c_str());
}