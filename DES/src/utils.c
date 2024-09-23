#include "..\inc\libs.h"


void print_binary64(uint64_t value)
{
  for (int i = 63; i >= 0; i--){
    printf("%d", (value >> i) & 1);
    if (i % 8 == 0){
      printf(" ");
    }
  }
  printf("\n");
}
void print_binary32(uint32_t value)
{
  for (int i = 31; i >= 0; i--){
    printf("%d", (value >> i) & 1);
    if (i % 8 == 0){
      printf(" ");
    }
  }
  printf("\n");
}
void print_binary8(uint32_t value)
{
  for (int i = 7; i >= 0; i--){
    printf("%d", (value >> i) & 1);
    if (i % 8 == 0){
      printf(" ");
    }
  }
  printf("\n");
}