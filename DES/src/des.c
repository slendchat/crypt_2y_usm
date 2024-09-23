#include "..\inc\tables.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 8

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

// returns amount of 64byte blocks used to store text
int32_t write_text_data(uint64_t *pdata)
{
  uint32_t res;
  char ch = 0;
  int position = 0;
  while ((ch = getchar()) != '\n' && position < BUFF_SIZE * 8 - 1){
    uint64_t temp = 0x00;
    pdata[position / 8] |= (temp | ch) << (8 * (7 - position % 8));
    position++;
  }
  if(position%8==0){
    res = (position / 8);
  }
  else{
    res = (position / 8) + 1;
  }
  return res;
}

uint64_t drop_parity(uint64_t key64b)
{
  uint64_t key56b = 0;
  for (int i = 1; i <= 8; i++)
  {
    key56b |= ((key64b >> (i - 1) * 8) << 57) >> (8 * (8 - i) + i);
  }
  return key56b;
}

void key_gen(uint64_t *keys48b, uint64_t key)
{
  uint64_t key_permutated = 0;
  for (int i = 0; i < sizeof(PC1) / sizeof(PC1[1]); i++){ // sizeof(PC1)/sizeof(PC1[1]) = 56
    key_permutated |= ((key >> (64 - PC1[i])) & 0x01) << (63 - i);
  }
  key_permutated = key_permutated >> 8;

  printf("[+] Keys\n");
  printf("[+] original key: ");
  print_binary64(key);
  printf("[+] key permutated:");
  print_binary64(key_permutated);

  uint32_t keyleft = (uint32_t)((key_permutated >> 28) & 0xFFFFFFF);
  uint32_t keyright = (uint32_t)(key_permutated & 0xFFFFFFF);

  printf("\n[+] Keys left and right permutated:\n");
  print_binary32(keyleft);
  print_binary32(keyright);
  printf("\n");
  //key expansion

  //key shifting
  for (int i = 0; i < 16; i++){

  }
  

  for (int i = 0; i < 16; i++){
    uint64_t key48b_concat = 0;

    keyright = (keyright << (iteration_shift[i])) & 0xFFFFFFF | (keyright >> (32 - 4 - iteration_shift[i]));
    keyleft = (keyleft << (iteration_shift[i])) & 0xFFFFFFF | (keyleft >> (32 - 4 - iteration_shift[i]));

    printf("\n[+] Subkeys left and right shifted on %d, i:%d:\n", iteration_shift[i], i + 1);
    print_binary32(keyleft);
    print_binary32(keyright);

    key48b_concat = ((((uint64_t)keyleft) & 0xFFFFFFF) << 28) | (keyright);
    printf("[+] 56b Key concatanated %d: ", i + 1);
    print_binary64(key48b_concat);

    // compress keys
    for (int j = 0; j < 48; j++){
      keys48b[i] |= ((key48b_concat >> (56 - PC2[j]) & 0x01) << (55 - j - 8));
    }
    printf("[+] 48b Subkey permutated %d: ", i + 1);
    print_binary64(keys48b[i]);
  }
}

void encrypt_data(uint64_t *keys, uint64_t *data, uint32_t size, uint64_t *data_encrypted)
{
  uint64_t left = 0;
  uint64_t right = 0;

  uint64_t *chunks = malloc(size * sizeof(uint64_t));
  memset(chunks, 0, size * sizeof(uint64_t));

  for (int i = 0; i < size; i++){
    chunks[i] = data[i];
  }

  for (int chunk_i = 0; chunk_i < size; chunk_i++){

    uint64_t temp;
    uint64_t chunk = 0;

    for (int i = 0; i < 64; i++){
      chunk |= ((chunks[chunk_i] >> (64 - IP[i])) & 0x01) << (63 - i);
    }

    left = chunk >> 32;
    right = chunk & 0xFFFFFFFF;

    printf("[+] Chunk of data: ");
    print_binary64(chunks[chunk_i]);
    printf("[+] Chunk of data permutated: ");
    print_binary64(chunk);
    printf("[+] Left part of the chunk: ");
    print_binary64(left);
    printf("[+] Right part of the chunk: ");
    print_binary64(right);

    for (int i = 0; i < 16; i++)
    {
      temp = right;
      for (int j = 0; j < 48; j++)
      {
        right |= ((right >> (32 - E[i])) & 0x01) << (31 - i);
      }
      printf("[+] data half blocks extended:\n");
      print_binary64(right);
      right = right ^ keys[i];
      print_binary64(right);

      printf("[+] data half blocks substituted:\n");
      uint8_t sblock6b[8] = {0};
      uint8_t sblock_res[8] = {0};
      for (int j = 0; j < 8; j++)
      {
        sblock6b[j] = (uint8_t)((right >> j * 6) & 0b111111);
        print_binary8(sblock6b[j]);
        uint8_t raw = ((sblock6b[j] & 0b00100000) >> 4) | (sblock6b[j] & 1);
        uint8_t col = (sblock6b[j] & 0b00011110) >> 1;
        sblock_res[j] = S[j][raw * 15 + col];
        printf("[+] SBLOCK res: ");
        print_binary8(sblock_res[j]);
      }
      // joining 4 bits to 32 bits
      uint64_t sblock = 0;
      for (int j = 0; j < 8; j++){
        sblock |= ((uint64_t)sblock_res[j]) << (4 * j);
      }

      for (int j = 0; j < 32; j++){
        right |= ((sblock >> (32 - P[i])) & 0x01) << (31 - i);
      }

      printf("[+] data half block after substitution:\n");
      printf("right: ");
      print_binary64(right);
      printf("left: ");
      print_binary64(left);
      right = right ^ left;
      left = temp;
    }
    uint64_t cipher_chunk = (left << 32) | (right);
    uint64_t encrypted_chunk = 0;
    printf("[+] Cipher chunk: ");
    print_binary64(cipher_chunk);

    for (int i = 0; i < 64; i++){
      encrypted_chunk |= ((cipher_chunk >> (64 - PI[i])) & 0x01) << (63 - i);
    }
    printf("[+] Encrypted chunk permutated: ");
    print_binary64(encrypted_chunk);

    data_encrypted[chunk_i] = encrypted_chunk;
  }
  free(chunks);
}

void reverse_array(uint64_t arr[], int size)
{
  int i = 0, j = size - 1;
  while (i < j)
  {
    // Swap arr[i] and arr[j]
    uint64_t temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;

    // Move towards the center
    i++;
    j--;
  }
}

int main(void)
{
  // key preparations
  uint64_t key = 0b1001110000110101101100011001001110100001100110101001011110100111;
  uint64_t keys48b[16] = {0};
  printf("[+] Key: ");
  print_binary64(key);

  // data preparations
  uint64_t *data_plain = malloc(BUFF_SIZE * sizeof(uint64_t));
  memset(data_plain, 0, BUFF_SIZE * sizeof(uint64_t));


  printf("[+] Input text - ");
  uint32_t data_size = write_text_data(data_plain);

  uint64_t *data_encrypted = malloc(BUFF_SIZE * sizeof(uint64_t));
  memset(data_encrypted, 0, data_size * sizeof(uint64_t));
  uint64_t *data_decrypted = malloc(BUFF_SIZE * sizeof(uint64_t));
  memset(data_decrypted, 0, data_size * sizeof(uint64_t));

  // keys generation
  key_gen(keys48b, key);

  encrypt_data(keys48b, data_plain, data_size, data_encrypted);



  printf("[+] Encrypted data:\n");
  for (int i = 0; i < data_size; i++){
    print_binary64(data_encrypted[i]);
  }

  //reverse key order to decrypt
  reverse_array(keys48b,16);
  encrypt_data(keys48b, data_encrypted, data_size, data_decrypted);

  printf("[+] Data size =%d Data:\n", data_size);
  for (int i = 0; i < 8; i++){
    print_binary64(data_plain[i]);
  }
  printf("[+] Encrypted data:\n");
  for (int i = 0; i < data_size; i++){
    print_binary64(data_encrypted[i]);
  }
    printf("[+] Decrypted data:\n");
  for (int i = 0; i < data_size; i++){
    print_binary64(data_decrypted[i]);
  }

  free(data_plain);
  free(data_encrypted);
  free(data_decrypted);
}