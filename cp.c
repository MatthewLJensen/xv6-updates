#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

int
main(int argc, char *argv[])
{
  int read_file, write_file, i;

  if(argc <= 2){
    printf(1, "cp requires at least 2 arguments\n");
    exit();
  }

//Open the file to be copied
if((read_file = open(argv[1], 0)) < 0){
    printf(1, "cp: cannot open %s\n", argv[1]);
    exit();
}

//Open the file to create and copy to
if((write_file = open(argv[2], O_CREATE|O_RDWR)) < 0){
    printf(1, "cp: cannot open %s\n", argv[2]);
    exit();
}

//Read through the read_file and write those buffers to the write_file until the file has been read all the way through
while((i = read(read_file, buf, sizeof(buf))) > 0){
    write(write_file, buf, i);
}

//Close files and exit
close(read_file);
close(write_file);
exit();
}