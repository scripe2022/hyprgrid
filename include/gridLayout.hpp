#pragma once

#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/helpers/math/Math.hpp>
#include <hyprland/src/layout/IHyprLayout.hpp>

struct SGridNodeData {
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    PHLWINDOWREF pWindow;
    int workspaceID = -1;
    CBox box;
    bool operator==(const SGridNodeData& rhs) const { return pWindow.lock() == rhs.pWindow.lock(); }
    SGridNodeData() { timestamp = std::chrono::system_clock::now(); }
};

struct SGridWorkspaceData {
    int workspaceID = -1;
    float windowRatio = 16 / 9.0f;
    int gaps = 25;
    bool operator==(const SGridWorkspaceData& rhs) const { return workspaceID == rhs.workspaceID; }
};

struct SGridWindowData {
    Vector2D position;
    Vector2D size;
};

class CHyprGridLayout: public IHyprLayout {
    public:
    virtual void onWindowCreatedTiling(PHLWINDOW, eDirection direction = DIRECTION_DEFAULT);
    virtual void onWindowRemovedTiling(PHLWINDOW);
    virtual std::string getLayoutName();

    virtual bool isWindowTiled(PHLWINDOW);
    virtual void recalculateMonitor(const MONITORID&);
    virtual void recalculateWindow(PHLWINDOW);
    virtual void resizeActiveWindow(const Vector2D&, eRectCorner corner, PHLWINDOW pWindow = nullptr);
    virtual void fullscreenRequestForWindow(PHLWINDOW, const eFullscreenMode CURRENT_EFFECTIVE_MODE, const eFullscreenMode EFFECTIVE_MODE);
    virtual std::any layoutMessage(SLayoutMessageHeader, std::string);
    virtual SWindowRenderLayoutHints requestRenderHints(PHLWINDOW);
    virtual void switchWindows(PHLWINDOW, PHLWINDOW);
    virtual void moveWindowTo(PHLWINDOW, const std::string& dir, bool silent);
    virtual void alterSplitRatio(PHLWINDOW, float, bool);
    virtual void replaceWindowDataWith(PHLWINDOW from, PHLWINDOW to);
    virtual Vector2D predictSizeForNewWindowTiled();
    virtual bool isWindowReachable(PHLWINDOW);

    virtual void onEnable();
    virtual void onDisable();

    private:
    // std::set<SGridNodeData, cmpTimestamp> m_nodesData;
    // std::set<SGridNodeData, cmpTimestamp>::iterator getNodeFromWindow(PHLWINDOW pWindow);
    std::vector<SGridNodeData> m_nodesData;
    std::vector<SGridNodeData>::iterator getNodeFromWindow(PHLWINDOW pWindow);
    int m_gridRows, m_gridCols;
    int getNodesOnWorkspace(const int& ws);
    void calculateWorkspace(PHLWORKSPACE);
    void applyNodeDataToWindow(PHLWINDOW pWindow, CBox box);
    void updateWindowSize(Vector2D monitorSize, int numWindows, SGridWorkspaceData workspaceData, WORKSPACEID wsid);
    PHLWINDOW getNextWindow(PHLWINDOW);
    PHLWINDOW getPrevWindow(PHLWINDOW);

    friend struct SGridNodeData;
    friend struct SGridWindowData;
};
