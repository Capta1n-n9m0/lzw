#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLE_SIZE 4096
#define MAX_CODE_LENGTH 16

typedef unsigned char BYTE;

/* Function to get the code for a given string */
int get_code(const char *str, const char **table, int table_size) {
  int i;
  for (i = 0; i < table_size; i++) {
    if (strcmp(str, table[i]) == 0) {
      return i;
    }
  }
  return -1;
}

void compress(FILE *input, FILE *output) {
  fseek(input, 0, SEEK_END);
  long size = ftell(input);
  fseek(input, 0, SEEK_SET);
  BYTE *inputBuffer = calloc(size, sizeof(BYTE));
  fread(inputBuffer, 1, size, input);
  
  char *table[MAX_TABLE_SIZE];
  int table_size = 256;
  int next_code = 256;
  
  // Initialize the table with single-character strings
  int i;
  for (i = 0; i < 256; i++) {
    table[i] = calloc(2, sizeof(char));
    table[i][0] = (char)i;
  }
  
  int current_code = inputBuffer[0];
  int next_char = inputBuffer[1];
  char *current_string = calloc(MAX_CODE_LENGTH, sizeof(char));
  current_string[0] = (char)current_code;
  current_string[1] = '\0';
  
  for (i = 1; i < size; i++) {
    next_char = inputBuffer[i];
    char *next_string = calloc(strlen(current_string) + 2, sizeof(char));
    strcpy(next_string, current_string);
    next_string[strlen(current_string)] = (char)next_char;
    next_string[strlen(current_string) + 1] = '\0';
    
    int code = get_code(next_string, (const char **)table, table_size);
    if (code != -1) {
      current_string = next_string;
    } else {
      fwrite(&current_code, sizeof(int), 1, output);
      if (table_size < MAX_TABLE_SIZE) {
        table[table_size] = next_string;
        table_size++;
      }
      current_string = calloc(MAX_CODE_LENGTH, sizeof(char));
      current_string[0] = (char)next_char;
      current_string[1] = '\0';
      current_code = get_code(current_string, (const char **)table, table_size);
    }
  }
  fwrite(&current_code, sizeof(int), 1, output);
  free(inputBuffer);
}

void decompress(FILE *input, FILE *output) {
  fseek(input, 0, SEEK_END);
  long size = ftell(input);
  fseek(input, 0, SEEK_SET);
  BYTE *inputBuffer = calloc(size, sizeof(BYTE));
  fread(inputBuffer, 1, size, input);
  
  char *table[MAX_TABLE_SIZE];
  int table_size = 256;
  int next_code = 256;
  
  // Initialize the table with single-character strings
  int i;
  for (i = 0; i < 256; i++) {
    table[i] = calloc(2, sizeof(char));
    table[i][0] = (char)i;
  }
  
  int current_code = inputBuffer[0];
  int next_char;
  char *current_string = calloc(MAX_CODE_LENGTH, sizeof(char));
  strcpy(current_string, table[current_code]);
  
  for (i = 1; i < size; i++) {
    next_char = inputBuffer[i];
    if (next_code < MAX_TABLE_SIZE) {
      char *next_string = calloc(strlen(current_string) + 2, sizeof(char));
      strcpy(next_string, current_string);
      next_string[strlen(current_string)] = (char)next_char;
      next_string[strlen(current_string) + 1] = '\0';
      
      table[next_code] = next_string;
      next_code++;
    }
    fwrite(current_string, sizeof(char), strlen(current_string), output);
    
    current_string = calloc(MAX_CODE_LENGTH, sizeof(char));
    strcpy(current_string, table[next_char]);
    current_code = next_char;
  }
  fwrite(current_string, sizeof(char), strlen(current_string), output);
  free(inputBuffer);
}

void error(char *message) {
  fprintf(stderr, "%s\n", message);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 4) error("Usage: ./lzw [-c|-d] input_file output_file");
  if (argv[1][0] != '-' || (argv[1][1] != 'c' && argv[1][1] != 'd'))
    error("Usage: ./lzw [-c|-d] input_file output_file");
  
  if (argv[1][1] == 'c') {
    FILE *input = fopen(argv[2], "rb");
    if (!input) error("Error: Could not open input file");
    
    FILE *output = fopen(argv[3], "wb");
    if (!output) error("Error: Could not open output file");
    
    compress(input, output);
    
    fclose(input);
    fclose(output);
  } else if (argv[1][1] == 'd') {
    FILE *input = fopen(argv[2], "rb");
    if (!input) error("Error: Could not open input file");
    
    FILE *output = fopen(argv[3], "wb");
    if (!output) error("Error: Could not open output file");
    
    decompress(input, output);
    
    fclose(input);
    fclose(output);
  }
  
  return 0;
}
