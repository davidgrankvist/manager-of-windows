#ifndef commands_h
#define commands_h

#include "common.h"
#include "input.h"

typedef enum {
    COMMAND_QUIT,
    COMMAND_SELECT_WORKSPACE_1,
    COMMAND_SELECT_WORKSPACE_2,
    COMMAND_ENUM_COUNT,
} CommandType;

typedef struct {
    CommandType type;
    union {
        byte stub;
    } payload;
} Command;

typedef KeyCode KeyMap[COMMAND_ENUM_COUNT];

#endif
