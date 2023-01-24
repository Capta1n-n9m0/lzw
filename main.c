#include <stdio.h>
#include <stdlib.h>


typedef unsigned char BYTE;

int main() {
  FILE *f = fopen("input1.txt", "rb");
  if(!f) exit(1);
  
  fseek(f, 0, SEEK_END);
  long fSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  BYTE *buffer = calloc(fSize, sizeof(BYTE));
  fread(buffer, 1, fSize, f);
  fclose(f);
  
  printf("%s\n", buffer);
  
  
  
  
  
  
  free(buffer);
  return 0;
}
