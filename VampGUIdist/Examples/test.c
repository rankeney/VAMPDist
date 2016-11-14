#include <stdio.h>
#include <stdlib.h>

int test(int val)
{
  int ret;

  if ((val >= 0) && (val < 10))
    ret = 1;
  else
  if ((val >= 10) && (val < 100))
    ret = 2;
  else
  if ((val >= 100) && (val < 1000))
    ret = 3;
  else
  if ((val >= 1000) && (val < 10000))
    ret = 4;
  else
    ret = 0;

  return ret;
}

int main(int argc, char *argv[])
{
  int ret;

  if (argc != 2)
  {
    printf("usage: %s <val>\n", argv[0]);

    return 0;
  }

  ret = test(atoi(argv[1]));
  printf("Result is %d\n", ret);

  return 1;
}
