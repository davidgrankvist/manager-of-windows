#ifndef manager_h
#define manager_h

void AddWindow(void* handle);
void SelectWorkspace(int workspace);

typedef struct {
    void (*ShowWindow)(void* handle);
    void (*HideWindow)(void* handle);
    void (*MoveResizeWindow)(void* handle, int x, int y, int width, int height);
} WindowPlatform;

void InitWindowPlatform(WindowPlatform platform);

#endif
