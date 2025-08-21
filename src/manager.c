#include "manager.h"

typedef struct {
    void* handle;
    int split;
} Window;

#define MAX_WINDOWS 1000
#define NUM_WORKSPACES 10
#define DEFAULT_WORKSPACE 1
#define MAX_SPLITS 1000

typedef struct {
    // position/size is in percent of screen
    int top;
    int left;
    int width;
    int height;
} Split;

typedef struct {
    int id;
    int width;
    int height;
} Screen;

typedef struct {
    Window windows[MAX_WINDOWS];
    int windowCount;
    Split splits[MAX_SPLITS];
    int splitCount;
    int currentSplit;
    Screen screen;
} Workspace;

typedef struct {
    Workspace workspaces[NUM_WORKSPACES];
    int currentWorkspace;
} WindowManagerState;

WindowManagerState state = {0};

void InitWindowManager(int screenWidth, int screenHeight) {
    Screen defaultScreen = (Screen) {
        .id = 0,
        .width = screenWidth,
        .height = screenHeight,
    };
    Split defaultSplit = (Split) {
        .top = 0,
        .left = 0,
        .width = 100,
        .height = 100,
    };
    for (int i = 0; i < NUM_WORKSPACES; i++) {
        Workspace workspace = {
            .splitCount = 1,
            .screen = defaultScreen,
        };
        workspace.splits[0] = defaultSplit,
        state.workspaces[i] = workspace;
    }

    state.currentWorkspace = 1;
}

WindowPlatform platform = {0};

void InitWindowPlatform(WindowPlatform pl) {
    platform = pl;
}

static inline int ScalePercent(int value, int percentage) {
    return value * (float)(percentage / 100.0);
}

static inline Workspace* GetWorkspace(int workspaceIndex) {
    return &state.workspaces[workspaceIndex];
}

static inline Workspace* GetCurrentWorkspace() {
    return GetWorkspace(state.currentWorkspace);
}

static inline Split* GetCurrentSplit() {
    Workspace* workspace = GetCurrentWorkspace();
    return &workspace->splits[workspace->currentSplit];
}

static void ApplyLayout(Window win) {
    Workspace* workspace = GetCurrentWorkspace();
    Screen screen = workspace->screen;
    Split split = workspace->splits[win.split];

    int width = ScalePercent(screen.width, split.width);
    int height = ScalePercent(screen.height, split.height);
    int top = ScalePercent(screen.height, split.top);
    int left = ScalePercent(screen.width, split.left);

    platform.MoveResizeWindow(win.handle, left, top, width, height);
}

static void UpdateWorkspaceLayout(Workspace* workspace) {
    for (int i = 0; i < workspace->windowCount; i++) {
        ApplyLayout(workspace->windows[i]);
    }
}

void SplitCurrentHorizontal() {
    Split* split = GetCurrentSplit();
    split->height /= 2;

    UpdateWorkspaceLayout(GetCurrentWorkspace());
}

void SplitCurrentVertical() {
    Split* split = GetCurrentSplit();
    split->width /= 2;

    UpdateWorkspaceLayout(GetCurrentWorkspace());
}

void AddWindow(void* handle) {
    Workspace* workspace = GetCurrentWorkspace();
    Window win = {
        .handle = handle,
        .split = workspace->splitCount - 1,
    };

    workspace->windows[workspace->windowCount++] = win;
    ApplyLayout(win);
}

static void HideWorkspaceWindows(int workspaceIndex) {
    Workspace* workspace = GetWorkspace(workspaceIndex);
    for (int i = 0; i < workspace->windowCount; i++) {
        platform.HideWindow(workspace->windows[i].handle);
    }
}

static void ShowWorkspaceWindows(int workspaceIndex) {
    Workspace* workspace = GetWorkspace(workspaceIndex);
    for (int i = 0; i < workspace->windowCount; i++) {
        platform.ShowWindow(workspace->windows[i].handle);
    }
}

void SelectWorkspace(int workspace) {
    if (workspace == state.currentWorkspace) {
        return;
    }

    Workspace* currentWorkspace = GetCurrentWorkspace();
    Workspace* targetWorkspace = GetWorkspace(workspace);
    int targetScreenId = targetWorkspace->screen.id;
    int currentScreenId = currentWorkspace->screen.id;
    if (targetScreenId == currentScreenId) {
        HideWorkspaceWindows(state.currentWorkspace);
        ShowWorkspaceWindows(workspace);
    }

    state.currentWorkspace = workspace;
}
