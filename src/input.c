#include "input.h"

static inline bool IsKeyDownBuf(KeyCode key, uint buf) {
    return HAS_BITS(buf, key);
}

bool IsKeyDown(KeyCode key, InputBuffer* buf) {
    return IsKeyDownBuf(key, buf->curr);
}

bool IsKeyUp(KeyCode key, InputBuffer* buf) {
    return !IsKeyDown(key, buf);
}

bool IsKeyPressed(KeyCode key, InputBuffer* buf) {
    return IsKeyDownBuf(key, buf->curr) && !IsKeyDownBuf(key, buf->prev);
}

bool IsKeyReleased(KeyCode key, InputBuffer* buf) {
    return !IsKeyDownBuf(key, buf->curr) && IsKeyDownBuf(key, buf->prev);
}

void SetKeyDown(KeyCode key, InputBuffer* buf) {
    buf->curr |= key;
}

void SetKeyUp(KeyCode key, InputBuffer* buf) {
    buf->curr &= ~key;
}

void ResetInputBuffer(InputBuffer* buf) {
    buf->curr = 0;
    buf->prev = 0;
}

void UpdateInputBuffer(InputBuffer* buf) {
    buf->prev = buf->curr;
    buf->curr = 0;
}
