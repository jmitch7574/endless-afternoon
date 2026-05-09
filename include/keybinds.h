#include "raylib.h"

#ifdef ARCADEMIA

#define MOVE_LEFT      KEY_LEFT
#define MOVE_RIGHT     KEY_RIGHT
#define MOVE_UP        KEY_UP
#define MOVE_DOWN      KEY_DOWN

#define ACCEPT         KEY_Z
#define ACCEPT_STRING  "A"

#else

#define MOVE_LEFT      KEY_A
#define MOVE_RIGHT     KEY_D
#define MOVE_UP        KEY_W
#define MOVE_DOWN      KEY_S

#define ACCEPT         KEY_ENTER
#define ACCEPT_STRING  "Enter"

#endif