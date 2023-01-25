#include <stdio.h>
#include <stdlib.h>


typedef unsigned char BYTE;

void compress(FILE *input, FILE *output){
  int prefix = getc(input);
  if(prefix == EOF) return;
  
  int character;
  
  
  
  
  
}

void decompress(FILE *input, FILE *output){
  int prefix = getc(input);
  if(prefix == EOF) return;
  
  int character;
  
  
  
  
  
}

void error(char *message){
  fprintf(stderr, "%s\n", message);
  exit(1);
}

int main(int argc, char **argv) {
  if(argc != 3) error("Usage: ./lzw [-c|-d] input_file output_file");
  if(argv[1][0] != '-' || (argv[1][1] != 'c' && argv[1][1] != 'd')) error("Usage: ./lzw [-c|-d] input_file output_file");
  
  if(argv[1][1] == 'c'){
    FILE *input = fopen(argv[2], "rb");
    if(!input) error("Error: Could not open input file");
    FILE *output = fopen(argv[3], "wb");
    if(!output) error("Error: Could not open output file");
    
    compress(input, output);
    
    fclose(input);
    fclose(output);
  }
  else if(argv[1][1] == 'd'){
    FILE *input = fopen(argv[2], "rb");
    if(!input) error("Error: Could not open input file");
    FILE *output = fopen(argv[3], "wb");
    if(!output) error("Error: Could not open output file");
    
    decompress(input, output);
    
    fclose(input);
    fclose(output);
  }
  return 0;
}
