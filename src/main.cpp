#include <unistd.h>

#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/desktop/Workspace.hpp>
#include <thread>

#include "globals.hpp"
#include "gridLayout.hpp"
// Methods
inline std::unique_ptr<CHyprGridLayout> g_pGridLayout;

// static void deleteWorkspaceData(int ws) {
//     if (g_pGridLayout) g_pGridLayout->removeWorkspaceData(ws);
// }
// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

void moveWorkspaceCallback(void *self, SCallbackInfo &cinfo, std::any data) {
    std::vector<std::any> moveData = std::any_cast<std::vector<std::any>>(data);
    PHLWORKSPACE ws = std::any_cast<PHLWORKSPACE>(moveData.front());
    // deleteWorkspaceData(ws->m_iID);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    g_pGridLayout = std::make_unique<CHyprGridLayout>();
    static auto MWCB = HyprlandAPI::registerCallbackDynamic(PHANDLE, "moveWorkspace", moveWorkspaceCallback);

    static auto DWCB = HyprlandAPI::registerCallbackDynamic(PHANDLE, "destroyWorkspace", [&](void *self, SCallbackInfo &, std::any data) {
        CWorkspace *ws = std::any_cast<CWorkspace *>(data);
        // deleteWorkspaceData(ws->m_iID);
    });

    HyprlandAPI::addLayout(PHANDLE, "grid", g_pGridLayout.get());

    HyprlandAPI::reloadConfig();

    return {"Grid layout", "Plugin for grid layout", "jyhptr", "0.0"};
}

APICALL EXPORT void PLUGIN_EXIT() { HyprlandAPI::invokeHyprctlCommand("seterror", "disable"); }
