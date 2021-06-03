#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

int
main(int argc, char *argv[])
{
  int read_file, i, n, j;
  j = 0;

  


  if(argc <= 1){
    printf(1, "nano requires at least 1 arguments\n");
    exit();
  }

//Open the file to be copied
if((read_file = open(argv[1], 0)) < 0){
    printf(1, "cp: cannot open %s\n", argv[1]);
    exit();
}



while((n = read(read_file, buf, sizeof(buf))) > 0){
    
    for(i=0; i<n; i++){
      if(buf[i] == '\n'){
        printf(1, "%s ", j);
        j++;
      }
      //printf(1, "%d", buf[i]);
    }
    printf(1, buf);
    
}

printf(1, "\n", argv[1]);

//Close files and exit
close(read_file);
exit();
}