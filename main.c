#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct{
  int prefix; // prefix for byte > 255
  int character; // the last byte of the string
} DictElement;

void dictionaryArrayAdd(int prefix, int character, int value);
int dictionaryArrayPrefix(int value);
int dictionaryArrayCharacter(int value);

DictElement dictionaryArray[4095];

// add prefix + character to the dictionary
void dictionaryArrayAdd(int prefix, int character, int value) {
  dictionaryArray[value].prefix = prefix;
  dictionaryArray[value].character = character;
}

int dictionaryArrayPrefix(int value) {
  return dictionaryArray[value].prefix;
}

int dictionaryArrayCharacter(int value) {
  return dictionaryArray[value].character;
}


enum {
  emptyPrefix = -1 // empty prefix for ASCII characters
};

// the "string" in the dictionary consists of the last byte of the string and an index to a prefix for that string
struct DictNode {
  int value; // the position in the list
  int prefix; // prefix for byte > 255
  int character; // the last byte of the string
  struct DictNode *next;
};

void dictionaryInit();
void appendNode(struct DictNode *node);
void dictionaryDestroy();
int dictionaryLookup(int prefix, int character);
int dictionaryPrefix(int value);
int dictionaryCharacter(int value);
void dictionaryAdd(int prefix, int character, int value);

// the dictionary
struct DictNode *dictionary, *tail;

// initialize the dictionary of ASCII characters @12bits
void dictionaryInit() {
  int i;
  struct DictNode *node;
  for (i = 0; i < 256; i++) { // ASCII
    node = (struct DictNode *)malloc(sizeof(struct DictNode));
    node->prefix = emptyPrefix;
    node->character = i;
    appendNode(node);
  }
}

// add node to the list
void appendNode(struct DictNode *node) {
  if (dictionary != NULL) tail->next = node;
  else dictionary = node;
  tail = node;
  node->next = NULL;
}

// destroy the whole dictionary down to NULL
void dictionaryDestroy() {
  while (dictionary != NULL) {
    dictionary = dictionary->next; /* the head now links to the next element */
  }
}

// is prefix + character in the dictionary?
int dictionaryLookup(int prefix, int character) {
  struct DictNode *node;
  for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
    if (node->prefix == prefix && node->character == character) return node->value;
  }
  return emptyPrefix;
}

int dictionaryPrefix(int value) {
  struct DictNode *node;
  for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
    if (node->value == value) return node->prefix;
  }
  return -1;
}

int dictionaryCharacter(int value) {
  struct DictNode *node;
  for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
    if (node->value == value) {
      //printf("\nNODE %i %i %i\n", node->value, node->prefix, node->character);
      return node->character;
    }
  }
  return -1;
}

// add prefix + character to the dictionary
void dictionaryAdd(int prefix, int character, int value) {
  struct DictNode *node;
  node = (struct DictNode *)malloc(sizeof(struct DictNode));
  node->value = value;
  node->prefix = prefix;
  node->character = character;
  //printf("\n(%i) = (%i) + (%i)\n", node->value, node->prefix, node->character);
  appendNode(node);
}


void writeBinary(FILE * output, int code);
int readBinary(FILE * input);

int leftover = 0;
int leftoverBits;

void writeBinary(FILE * output, int code) {
  if (leftover > 0) {
    int previousCode = (leftoverBits << 4) + (code >> 8);
    
    fputc(previousCode, output);
    fputc(code, output);
    
    leftover = 0; // no leftover now
  } else {
    leftoverBits = code & 0xF; // save leftover, the last 00001111
    leftover = 1;
    
    fputc(code >> 4, output);
  }
}

int readBinary(FILE * input) {
  int code = fgetc(input);
  if (code == EOF) return 0;
  
  if (leftover > 0) {
    code = (leftoverBits << 8) + code;
    
    leftover = 0;
  } else {
    int nextCode = fgetc(input);
    
    leftoverBits = nextCode & 0xF; // save leftover, the last 00001111
    leftover = 1;
    
    code = (code << 4) + (nextCode >> 4);
  }
  return code;
}


enum {
  dictionarySize = 4095, // maximum number of entries defined for the dictionary (2^12 = 4096)
  codeLength = 12, // the codes which are taking place of the substrings
  maxValue = dictionarySize - 1
};

// function declarations
void compress(FILE *inputFile, FILE *outputFile);
void decompress(FILE *inputFile, FILE *outputFile);
int decode(int code, FILE * outputFile);

// compression
void compress(FILE *inputFile, FILE *outputFile) {
  int prefix = getc(inputFile);
  if (prefix == EOF) {
    return;
  }
  int character;
  
  int nextCode;
  int index;
  
  // LZW starts out with a dictionary of 256 characters (in the case of 8 codeLength) and uses those as the "standard"
  //  character set.
  nextCode = 256; // next code is the next available string code
  dictionaryInit();
  
  // while (there is still data to be read)
  while ((character = getc(inputFile)) != (unsigned)EOF) { // ch = read a character;
    
    // if (dictionary contains prefix+character)
    if ((index = dictionaryLookup(prefix, character)) != -1) prefix = index; // prefix = prefix+character
    else { // ...no, try to add it
      // encode s to output file
      writeBinary(outputFile, prefix);
      
      // add prefix+character to dictionary
      if (nextCode < dictionarySize) dictionaryAdd(prefix, character, nextCode++);
      
      // prefix = character
      prefix = character; //... output the last string after adding the new one
    }
  }
  // encode s to output file
  writeBinary(outputFile, prefix); // output the last code
  
  if (leftover > 0) fputc(leftoverBits << 4, outputFile);
  
  // free the dictionary here
  dictionaryDestroy();
}

// decompression
// to reconstruct a string from an index we need to traverse the dictionary strings backwards, following each
//   successive prefix index until this prefix index is the empty index
void decompress(FILE * inputFile, FILE * outputFile) {
  // int prevcode, currcode
  int previousCode; int currentCode;
  int nextCode = 256; // start with the same dictionary of 256 characters
  
  int firstChar;
  
  // prevcode = read in a code
  previousCode = readBinary(inputFile);
  if (previousCode == 0) {
    return;
  }
  fputc(previousCode, outputFile);
  
  // while (there is still data to read)
  while ((currentCode = readBinary(inputFile)) > 0) { // currcode = read in a code
    
    if (currentCode >= nextCode) {
      fputc(firstChar = decode(previousCode, outputFile), outputFile); // S+C+S+C+S exception [2.]
      //printf("%c", firstChar);
      //appendCharacter(firstChar = decode(previousCode, outputFile));
    } else firstChar = decode(currentCode, outputFile); // first character returned! [1.]
    
    // add a new code to the string table
    if (nextCode < dictionarySize) dictionaryArrayAdd(previousCode, firstChar, nextCode++);
    
    // prevcode = currcode
    previousCode = currentCode;
  }
  //printf("\n");
}

int decode(int code, FILE * outputFile) {
  int character; int temp;
  
  if (code > 255) { // decode
    character = dictionaryArrayCharacter(code);
    temp = decode(dictionaryArrayPrefix(code), outputFile); // recursion
  } else {
    character = code; // ASCII
    temp = code;
  }
  fputc(character, outputFile);
  //printf("%c", character);
  //appendCharacter(character);
  return temp;
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
