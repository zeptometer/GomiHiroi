#include "object.h"

KrtObj allocKrtObj(KrtType type);
KrtEnv allocKrtEnv();
void   markAndSweep();

KrtEnv currentFrame;
