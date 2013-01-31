#include <time.h>

int cpspc_time(void)
{
  return clock() * 1000 / CLOCKS_PER_SEC;
}
