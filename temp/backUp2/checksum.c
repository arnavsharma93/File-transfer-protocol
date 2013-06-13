#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hextab = "0123456789ABCDEF";

char* encode_int(int i) {
  char* c = (char*)malloc(sizeof(char) * 9);
  int j;
  for (j = 0; j < 4; j++) {
    c[(j << 1)] = hextab[((i % 256) >> 4)];
    c[(j << 1) + 1] = hextab[((i % 256) % 16)];

    i = (i >> 8);
  }
  c[8] = 0;

  return c;
}

int checksum(char* str) {
  int i;
  int chk = 0x12345678;

  for (i = 0; str[i] != '\0'; i++) {
    chk += ((int)(str[i]) * (i + 1));
  }

  return chk;
}

int main()
{
  long f_size;
  char* code;
  size_t code_s, result;
  char filename[100];
  scanf("%s",filename);
  FILE* fp = fopen(filename, "r");
  fseek(fp, 0, SEEK_END);
  f_size = ftell(fp); /* This returns 29696, but file is 85 bytes */
  fseek(fp, 0, SEEK_SET);
  code_s = sizeof(char) * f_size;
  code = malloc(code_s);
  result = fread(code, 1, f_size, fp);
  
  printf("%s\n",encode_int(checksum(code)));

  return 0;
}
