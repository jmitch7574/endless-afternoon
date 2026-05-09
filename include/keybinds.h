#pragma once

#include "raylib.h"

#ifdef ARCADEMIA

// Mappings for arcade machine controller
#define MOVE_LEFT KEY_LEFT
#define MOVE_RIGHT KEY_RIGHT
#define MOVE_UP KEY_UP
#define MOVE_DOWN KEY_DOWN

#define PRIMARY KEY_Z
#define SECONDARY KEY_X
#define PRIMARY_STRING "A"
#define SECONDARY_STRING "B"

#else

// Default keybinds for keyboard
#define MOVE_LEFT KEY_A
#define MOVE_RIGHT KEY_D
#define MOVE_UP KEY_W
#define MOVE_DOWN KEY_S

#define PRIMARY KEY_ENTER
#define SECONDARY KEY_LEFT_SHIFT
#define PRIMARY_STRING "Enter"
#define SECONDARY_STRING "Shift"

#endif

#define ACCEPT_STRING PRIMARY_STRING
