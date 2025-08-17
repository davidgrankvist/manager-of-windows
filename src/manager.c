#include "manager.h"

typedef struct {
    void* handle;
} Window;

typedef enum {
    LAYOUT_FLOATING,
    LAYOUT_MAXIMIZED,
    LAYOUT_TILED,
} LayoutType;

#define MAX_WINDOWS 1000
#define MAX_WORKSPACES 10
#define DEFAULT_WORKSPACE 1

typedef struct {
    LayoutType layout;
    Window windows[MAX_WINDOWS];
    int windowCount;
} Workspace;

typedef struct {
    Workspace workspaces[MAX_WORKSPACES];
    int currentWorkspace;
} WindowManagerState;

WindowManagerState state = {
    .workspaces = {0},
    .currentWorkspace = DEFAULT_WORKSPACE,
};

WindowPlatform platform = {0};

void InitWindowPlatform(WindowPlatform pl) {
    platform = pl;
}

void AddWindow(void* handle) {
    Window win = {
        .handle = handle,
    };

    Workspace* workspace = &state.workspaces[state.currentWorkspace];
    workspace->windows[workspace->windowCount++] = win;
}

static void HideWorkspaceWindows(int workspaceIndex) {
    Workspace* workspace = &state.workspaces[workspaceIndex];
    for (int i = 0; i < workspace->windowCount; i++) {
        platform.HideWindow(workspace->windows[i].handle);
    }
}

static void ShowWorkspaceWindows(int workspaceIndex) {
    Workspace* workspace = &state.workspaces[workspaceIndex];
    for (int i = 0; i < workspace->windowCount; i++) {
        platform.ShowWindow(workspace->windows[i].handle);
    }
}

void SelectWorkspace(int workspace) {
    if (workspace == state.currentWorkspace) {
        return;
    }

    HideWorkspaceWindows(state.currentWorkspace);
    ShowWorkspaceWindows(workspace);
    state.currentWorkspace = workspace;
}
