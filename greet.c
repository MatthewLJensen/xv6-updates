#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
  if(argc < 2){
    printf(1, "Hello world!\n");
    exit();
  }

  printf(2, "Hello %s\n", argv[1]);
  exit();

}