#ifndef commands_h
#define commands_h

#include "common.h"
#include "input.h"

typedef enum {
    COMMAND_NONE,
    // Global
    COMMAND_QUIT,
    // Select workspace mode
    COMMAND_SELECT_WORKSPACE_0,
    COMMAND_SELECT_WORKSPACE_1,
    COMMAND_SELECT_WORKSPACE_2,
    COMMAND_SELECT_WORKSPACE_3,
    COMMAND_SELECT_WORKSPACE_4,
    COMMAND_SELECT_WORKSPACE_5,
    COMMAND_SELECT_WORKSPACE_6,
    COMMAND_SELECT_WORKSPACE_7,
    COMMAND_SELECT_WORKSPACE_8,
    COMMAND_SELECT_WORKSPACE_9,
    // Select window mode
    COMMAND_MODE_SELECT_WINDOW,
    COMMAND_SELECT_WINDOW_LEFT,
    COMMAND_SELECT_WINDOW_RIGHT,
    COMMAND_SELECT_WINDOW_UP,
    COMMAND_SELECT_WINDOW_DOWN,
    //
    COMMAND_ENUM_COUNT,
} CommandType;

typedef struct {
    CommandType type;
    union {
        byte stub;
    } payload;
} Command;

typedef KeyCode KeyMap[COMMAND_ENUM_COUNT];

typedef enum {
    MODE_SELECT_WORKSPACE,
    MODE_SELECT_WINDOW,
 } CommandModeType;

#endif
