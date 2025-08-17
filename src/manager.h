#ifndef manager_h
#define manager_h

void AddWindow(void* handle);
void SelectWorkspace(int workspace);

typedef struct {
    void (*ShowWindow)(void* handle);
    void (*HideWindow)(void* handle);
} WindowPlatform;

void InitWindowPlatform(WindowPlatform platform);

#endif
