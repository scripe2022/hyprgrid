#include <cmath>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/helpers/MiscFunctions.hpp>
#include <hyprland/src/render/decorations/CHyprGroupBarDecoration.hpp>
#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <cmath>

#include "gridLayout.hpp"

void buildMessage(std::ostringstream& oss) {
}

template <typename T, typename... Args>
void buildMessage(std::ostringstream& oss, const T& first, const Args&... args) {
    oss << first << "\n";
    buildMessage(oss, args...);
}

template <typename... Args>
void debug(const Args&... args) {
    std::ostringstream oss;
    buildMessage(oss, args...);

    std::string command = "echo \"\n" + oss.str() + "\" | socat - UNIX-CONNECT:/tmp/debug_listener";

    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Error: Failed to send message to socket." << std::endl;
    }
}

std::vector<SGridNodeData>::iterator CHyprGridLayout::getNodeFromWindow(PHLWINDOW pWindow) {
    for (auto nd = m_nodesData.begin(); nd != m_nodesData.end(); ++nd) {
        if (nd->pWindow.lock() == pWindow) return nd;
    }
    return m_nodesData.end();
}

int CHyprGridLayout::getNodesOnWorkspace(const int& ws) {
    int num = 0;
    for (auto nd = m_nodesData.begin(); nd != m_nodesData.end(); ++nd) {
        if (nd->workspaceID == ws) ++num;
    }
    return num;
}

void CHyprGridLayout::onWindowCreatedTiling(PHLWINDOW pWindow, eDirection direction) {
    if (pWindow->m_bIsFloating) return;

    const auto WSID = pWindow->workspaceID();

    const auto PMONITOR = g_pCompositor->getMonitorFromID(pWindow->m_iMonitorID);

    auto PNODE = SGridNodeData();
    PNODE.pWindow = pWindow;
    PNODE.workspaceID = WSID;
    m_nodesData.emplace_back(PNODE);

    // recalculateWindow(pWindow);
    recalculateMonitor(pWindow->m_iMonitorID);
}

void CHyprGridLayout::onWindowRemovedTiling(PHLWINDOW pWindow) {
    const auto PNODE = getNodeFromWindow(pWindow);
    if (PNODE == m_nodesData.end()) return;

    pWindow->unsetWindowData(PRIORITY_LAYOUT);
    pWindow->updateWindowData();

    if (pWindow->isFullscreen()) g_pCompositor->setWindowFullscreenInternal(pWindow, FSMODE_NONE);

    m_nodesData.erase(PNODE);

    recalculateMonitor(pWindow->m_iMonitorID);
}

bool CHyprGridLayout::isWindowTiled(PHLWINDOW pWindow) { return getNodeFromWindow(pWindow) != m_nodesData.end(); }

void CHyprGridLayout::recalculateMonitor(const MONITORID& monid) {
    const auto PMONITOR = g_pCompositor->getMonitorFromID(monid);
    if (!PMONITOR || !PMONITOR->activeWorkspace) return;

    const auto PWORKSPACE = PMONITOR->activeWorkspace;
    if (!PWORKSPACE) return;
    g_pHyprRenderer->damageMonitor(PMONITOR);

    if (PMONITOR->activeSpecialWorkspace) { calculateWorkspace(PMONITOR->activeSpecialWorkspace); }
    calculateWorkspace(PWORKSPACE);
}

void CHyprGridLayout::calculateWorkspace(PHLWORKSPACE PWORKSPACE) {
    if (!PWORKSPACE) return;

    const auto PMONITOR = g_pCompositor->getMonitorFromID(PWORKSPACE->m_iMonitorID);
    if (!PMONITOR) return;

    if (PWORKSPACE->m_bHasFullscreenWindow) {
        const auto PFULLWINDOW = g_pCompositor->getFullscreenWindowOnWorkspace(PWORKSPACE->m_iID);

        if (PWORKSPACE->m_efFullscreenMode == FSMODE_FULLSCREEN) {
            PFULLWINDOW->m_vRealPosition = PMONITOR->vecPosition;
            PFULLWINDOW->m_vRealSize = PMONITOR->vecSize;
        }
        return;
    }

    const auto NODECOUNT = getNodesOnWorkspace(PWORKSPACE->m_iID);

    updateWindowSize(PMONITOR->vecSize, NODECOUNT, SGridWorkspaceData(), PWORKSPACE->m_iID);
}

void CHyprGridLayout::applyNodeDataToWindow(PHLWINDOW pWindow, CBox box) {
    if (!pWindow) return;
    pWindow->m_vPosition = box.pos();
    pWindow->m_vSize = box.size();
    int borderSize = pWindow->getRealBorderSize();
    pWindow->m_vRealPosition = box.pos() + Vector2D(borderSize, borderSize);
    pWindow->m_vRealSize = box.size() - Vector2D(2 * borderSize, 2 * borderSize);

    g_pXWaylandManager->setWindowSize(pWindow, pWindow->m_vRealSize.goal());
    pWindow->updateWindowDecos();
}

void CHyprGridLayout::updateWindowSize(Vector2D monitorSize, int numWindows, SGridWorkspaceData workspaceData, WORKSPACEID wsid) {
    // monitorSize.x: width
    // monitorSize.y: height
    int win_width = -1, win_height = -1;
    m_gridRows = -1, m_gridCols = -1;
    // bs row
    for (int left = 1, right = numWindows; left <= right; ) {
        int wpr = (left + right) / 2;
        int rows = std::ceil((float)numWindows / wpr);
        int w = std::floor((((float)monitorSize.x + workspaceData.gaps) / wpr) - workspaceData.gaps);
        int h = std::floor((float)w / workspaceData.windowRatio);
        int total_height = (h + workspaceData.gaps) * rows - workspaceData.gaps;
        if (total_height <= monitorSize.y) {
            if (w > win_width) win_width = w, win_height = h, m_gridCols = wpr, m_gridRows = rows;
            right = wpr - 1;
        }
        else left = wpr + 1;
    }
    // bs col
    for (int left = 1, right = numWindows; left <= right; ) {
        int wpc = (left + right) >> 1;
        int cols = std::ceil((float)numWindows / wpc);
        int h = std::floor((((float)monitorSize.y + workspaceData.gaps) / wpc) - workspaceData.gaps);
        int w = std::floor((float)w * workspaceData.windowRatio);
        int total_width = (w + workspaceData.gaps) * cols - workspaceData.gaps;
        if (total_width <= monitorSize.x) {
            if (w > win_width) win_width = w, win_height = h, m_gridCols = cols, m_gridRows = wpc;
            right = wpc - 1;
        }
        else left = wpc + 1;
    }
    // print
    std::vector<SGridNodeData>::iterator winid = m_nodesData.begin();
    int top_offset_base = (monitorSize.y - m_gridRows * (win_height + workspaceData.gaps) + workspaceData.gaps) / 2;
    for (int i = 0; i < m_gridRows; ++i) {
        int n = (i != m_gridRows - 1) ? m_gridCols : numWindows - i * m_gridCols;
        int row_len = n * (win_width + workspaceData.gaps) - workspaceData.gaps;
        int left_offset_base = (monitorSize.x - row_len) / 2;
        int top_offset = top_offset_base + i * (win_height + workspaceData.gaps);
        for (int j = 0; j < n; ++j) {
            int left_offset = left_offset_base + j * (win_width + workspaceData.gaps);
            while (winid != m_nodesData.end() && winid->workspaceID != wsid) ++winid;
            CBox wb = {Vector2D(left_offset, top_offset), Vector2D(win_width, win_height)};
            if (wb != winid->box) {
                winid->box = wb;
                applyNodeDataToWindow(winid->pWindow.lock(), wb);
            }
            ++winid;
        }
    }
}

void CHyprGridLayout::recalculateWindow(PHLWINDOW) {}
void CHyprGridLayout::resizeActiveWindow(const Vector2D &, eRectCorner corner, PHLWINDOW pWindow) {}
void CHyprGridLayout::fullscreenRequestForWindow(PHLWINDOW, const eFullscreenMode CURRENT_EFFECTIVE_MODE, const eFullscreenMode EFFECTIVE_MODE) {}


PHLWINDOW CHyprGridLayout::getNextWindow(PHLWINDOW pWindow) {
    auto PNODE = getNodeFromWindow(pWindow);
    int i = (PNODE - m_nodesData.begin() + 1) % m_nodesData.size();
    while (m_nodesData[i].workspaceID != pWindow->workspaceID()) i = (i + 1) % m_nodesData.size();
    return m_nodesData[i].pWindow.lock();
}

PHLWINDOW CHyprGridLayout::getPrevWindow(PHLWINDOW pWindow) {
    auto PNODE = getNodeFromWindow(pWindow);
    int i = (PNODE - m_nodesData.begin() - 1 + m_nodesData.size()) % m_nodesData.size();
    while (m_nodesData[i].workspaceID != pWindow->workspaceID()) i = (i - 1 + m_nodesData.size()) % m_nodesData.size();
    return m_nodesData[i].pWindow.lock();
}

std::any CHyprGridLayout::layoutMessage(SLayoutMessageHeader header, std::string message) {
    auto switchToWindow = [&](PHLWINDOW PWINDOWTOCHANGETO) {

        // if (header.pWindow->isFullscreen()) {
        //     const auto PWORKSPACE = header.pWindow->m_pWorkspace;
        //     const auto FSMODE = header.pWindow->m_sFullscreenState.internal;
        //     const auto WORKSPACEDATA = getMasterWorkspaceData(PWORKSPACE->m_iID);
        //     g_pCompositor->setWindowFullscreenInternal(header.pWindow, FSMODE_NONE);
        //     g_pCompositor->focusWindow(PWINDOWTOCHANGETO);
        //     if (WORKSPACEDATA->inherit_fullscreen) g_pCompositor->setWindowFullscreenInternal(PWINDOWTOCHANGETO, FSMODE);
        // }
        // else {
            g_pCompositor->focusWindow(PWINDOWTOCHANGETO);
            g_pCompositor->warpCursorTo(PWINDOWTOCHANGETO->middle());
        // }
        g_pInputManager->m_pForcedFocus = PWINDOWTOCHANGETO;
        g_pInputManager->simulateMouseMovement();
        g_pInputManager->m_pForcedFocus.reset();
    };

    CVarList vars(message, 0, ' ');
    if (vars.size() < 1 || vars[0].empty()) {
        Debug::log(ERR, "layoutmsg called without parameters");
        return 0;
    }
    auto command = vars[0];

    if (command == "cyclenext") {
        const auto PWINDOW = header.pWindow;
        if (!PWINDOW) return 0;
        const auto PNEXTWINDOW = getNextWindow(PWINDOW);
        switchToWindow(PNEXTWINDOW);
    }
    else if (command == "cycleprev") {
        const auto PWINDOW = header.pWindow;
        if (!PWINDOW) return 0;
        const auto PPREVWINDOW = getPrevWindow(PWINDOW);
        switchToWindow(PPREVWINDOW);
    }
    else if (command == "swapnext") {
        if (!validMapped(header.pWindow)) return 0;
    }
    return 0;
}

SWindowRenderLayoutHints CHyprGridLayout::requestRenderHints(PHLWINDOW pWindow) {
    SWindowRenderLayoutHints hints;
    return hints;
}
void CHyprGridLayout::switchWindows(PHLWINDOW, PHLWINDOW) {}

void CHyprGridLayout::moveWindowTo(PHLWINDOW, const std::string& dir, bool silent) {
    debug(dir);
}

void CHyprGridLayout::alterSplitRatio(PHLWINDOW, float, bool) {}
void CHyprGridLayout::replaceWindowDataWith(PHLWINDOW from, PHLWINDOW to) {}
Vector2D CHyprGridLayout::predictSizeForNewWindowTiled() {
    return {};
}
void CHyprGridLayout::onEnable() {}
void CHyprGridLayout::onDisable() {}

std::string CHyprGridLayout::getLayoutName() { return "grid"; }

bool CHyprGridLayout::isWindowReachable(PHLWINDOW) {
    return true;
}
