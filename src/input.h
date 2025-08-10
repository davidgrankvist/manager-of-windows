#ifndef input_h
#define input_h

#include <stdbool.h>

typedef unsigned int uint;

typedef enum {
    KEY_1 = 1 << 1,
    KEY_2 = 1 << 2,
    KEY_3 = 1 << 3,
    KEY_4 = 1 << 4,
    KEY_5 = 1 << 5,
    KEY_6 = 1 << 6,
    KEY_7 = 1 << 7,
    KEY_8 = 1 << 8,
    KEY_9 = 1 << 9,
    KEY_0 = 1 << 10,
    KEY_Q = 1 << 11,
    KEY_W = 1 << 12,
    KEY_SUPER_LEFT = 1 << 13,
} KeyCode;

#define KEY_NONE 0

#define HAS_BITS(n, bs) ((n & bs) == bs)

typedef struct {
    uint curr;
    uint prev;
} InputBuffer;

bool IsKeyDown(KeyCode key, InputBuffer* buf);
bool IsKeyUp(KeyCode key, InputBuffer* buf);
bool IsKeyPressed(KeyCode key, InputBuffer* buf);
bool IsKeyReleased(KeyCode key, InputBuffer* buf);
void SetKeyDown(KeyCode key, InputBuffer* buf);
void SetKeyUp(KeyCode key, InputBuffer* buf);
void ResetInputBuffer(InputBuffer* buf);
void UpdateInputBuffer(InputBuffer* buf); // reset curr, update prev

#endif
