#include <stdio.h>
#include "timelib.h"

int main(void)
{
  int i;
  for (i = 0; i <100000000; i++);
  printf("%d\n", cpspc_time());
}
