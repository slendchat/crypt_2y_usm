#include "..\inc\tables.h"
#include "..\inc\libs.h"

#define BUFF_MAX_SIZE 128 //64bits 8 bytes

//returns buffer actual size in bytes
int32_t fill_text_buffer(uint64_t *pdata);
void print_binary8(uint8_t value);
void print_binary32(uint32_t value);
void print_binary64(uint64_t value);

uint64_t key_permutation(uint64_t key);
void shifting_key_pairs(uint32_t shifted_key_pairs[16][2], uint64_t key_permutated);
uint64_t join_half_keys(uint32_t keyleft, uint32_t keyright) ;
void form_keys48b(uint32_t shifted_key_pairs[16][2], uint64_t *keys48b);
void key_gen(uint64_t *keys48b, uint64_t key);

uint64_t initial_permutation(uint64_t data_block);
void div_block64_to_blocks32(uint32_t* left, uint32_t* right, uint64_t block64);
void sblock_fill(uint8_t sblock6b[8], uint64_t block64b);
uint8_t sblock_resolve(uint8_t sblock6b, int i);
uint32_t join4b_to_32b(uint8_t sblock4b[8]);
uint32_t permutation(uint32_t sblock_joined);
uint64_t concat_blocks32_to64(uint32_t left_block32, uint32_t right_block32);
uint64_t final_permutation(uint64_t block64);
uint64_t encrypt_DES(uint64_t data_block, uint64_t keys48b[16]);
uint64_t* reverse_array(const uint64_t arr[], int size);
void printchars(uint64_t value);


int main(void)
{
  // key preparations
  uint64_t key = 0b0001001100110100010101110111100110011011101111001101111111110001;
  // uint64_t key = 0b1111111111111111111111111111111111111111111111111111111101111111;
  // uint64_t key = 0b0000000000000000000000000000000000000000000000000000000100000001;
  uint64_t keys48b[16] = {0};
  uint64_t* reversed_keys48b;
  printf("[+] Key: ");
  print_binary64(key);

  // data preparations
  uint64_t *data_plain = malloc(BUFF_MAX_SIZE * sizeof(uint64_t));
  memset(data_plain, 0, BUFF_MAX_SIZE * sizeof(uint64_t));


  printf("[+] Input text - ");
  uint32_t data_size = fill_text_buffer(data_plain);

  uint64_t *data_encrypted = malloc(data_size * sizeof(uint64_t));
  memset(data_encrypted, 0, data_size * sizeof(uint64_t));

  uint64_t *data_decrypted = malloc(data_size * sizeof(uint64_t));
  memset(data_decrypted, 0, data_size * sizeof(uint64_t));

  // keys generation
  key_gen(keys48b, key);

  // TEST
  // uint64_t data_test = 0b0000000100100011010001010110011110001001101010111100110111101111;
  // printf("[+] Data test:\n");
  // print_binary64(data_test);
  // encrypt_DES(data_test,keys48b);


  // ENCRYPTION DATA BY BLOCKS
  for (int i = 0; i < data_size; i++){
    data_encrypted[i] = encrypt_DES(data_plain[i],keys48b);
  }
  
  //decrypt
  reversed_keys48b = reverse_array(keys48b, 16);
  for (int i = 0; i < data_size; i++){
    data_decrypted[i] = encrypt_DES(data_encrypted[i],reversed_keys48b);
  }

  printf("\n[+] Data size = %d Plain text:\n", data_size);
  for (int i = 0; i < data_size; i++){
    print_binary64(data_plain[i]);
  }
  printf("chars:\n");
  for (int i = 0; i < data_size; i++){
    printchars(data_plain[i]);
  }
  putchar('\n');

  printf("\n[+] Data size = %d Encrypted:\n", data_size);
  for (int i = 0; i < data_size; i++){
    print_binary64(data_encrypted[i]);
  }
  printf("chars:\n");
  for (int i = 0; i < data_size; i++){
    printchars(data_encrypted[i]);
  }
  putchar('\n');

  printf("\n[+] Data size = %d Decrypted:\n", data_size);
  for (int i = 0; i < data_size; i++){
    print_binary64(data_decrypted[i]);
  }
  printf("chars:\n");
  for (int i = 0; i < data_size; i++){
    printchars(data_decrypted[i]);
  }
  putchar('\n');

  free(data_plain);
  free(data_encrypted);
  free(data_decrypted);
  free(reversed_keys48b);
}

int32_t fill_text_buffer(uint64_t *pdata) 
{
  uint32_t res;
  char ch = 0;
  int position = 0;
  while ((ch = getchar()) != '\n' && position < BUFF_MAX_SIZE*8-1){
    uint64_t temp = 0;
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
void print_binary8(uint8_t value)
{
  for (int i = 7; i >= 0; i--){
    printf("%d", (value >> i) & 1);
    if (i % 8 == 0){
      printf(" ");
    }
  }
  printf("\n");
}

uint64_t key_permutation(uint64_t key)
{
  uint64_t permutated_key = 0;
  for (int i = 0; i < 56; i++){
    permutated_key |= ((( key >> (64 - PC1[i]) )&0x01) << (63 - i));
  }
  printf("[+] PC1 - ");
  print_binary64(permutated_key);
  return permutated_key;
}
void shifting_key_pairs(uint32_t shifted_key_pairs[16][2], uint64_t key_permutated)
{
  uint32_t keyleft = 0;
  uint32_t keyright = 0;

  keyleft = (key_permutated >> 36)<<4;
  keyright = (key_permutated << 28)>>32;

  printf("[+] key left - ");
  print_binary32(keyleft);
  printf("[+] key right - ");
  print_binary32(keyright);

  shifted_key_pairs[0][0] = ((keyleft << iteration_shift[0])) | ((keyleft >> (32 - iteration_shift[0]))<<(3+iteration_shift[0]));
  shifted_key_pairs[0][1] = ((keyright << iteration_shift[0])) | ((keyright >> (32 - iteration_shift[0]))<<(3+iteration_shift[0]));
  printf("[+] pair 1\n");
  print_binary32(shifted_key_pairs[0][0]);
  print_binary32(shifted_key_pairs[0][1]);

  printf("[+] Shifted keys - ");
  for (int i = 1; i < 16; i++){
    shifted_key_pairs[i][0] = ((shifted_key_pairs[i-1][0] << iteration_shift[i])) | ((shifted_key_pairs[i-1][0] >> (32 - iteration_shift[i]))<<(3+iteration_shift[0]));
    shifted_key_pairs[i][1] = ((shifted_key_pairs[i-1][1] << iteration_shift[i])) | ((shifted_key_pairs[i-1][1] >> (32 - iteration_shift[i]))<<(3+iteration_shift[0]));
    printf("[+] pair %d\n",i+1);
    print_binary32(shifted_key_pairs[i][0]);
    print_binary32(shifted_key_pairs[i][1]);
  }

}
uint64_t join_half_keys(uint32_t keyleft, uint32_t keyright) 
{
  uint64_t concat_key = 0;
  concat_key = (uint64_t)keyleft;
  concat_key = (uint64_t)(concat_key << 32) | (((uint64_t)keyright)<<4);
  return concat_key;
}
void form_keys48b(uint32_t shifted_key_pairs[16][2], uint64_t *keys48b)
{
  uint64_t half_keys_concated[16]={0};
  for (int i = 0; i < 16; i++){
    half_keys_concated[i]=join_half_keys(shifted_key_pairs[i][0],shifted_key_pairs[i][1]);
    printf("[+] concated key - ");
    print_binary64(half_keys_concated[i]);
  }

  for (int key_i = 0; key_i < 16; key_i++){  
    for (int i = 0; i < 48; i++){
      keys48b[key_i] |= ((( half_keys_concated[key_i] >> (64 - PC2[i]) )&0x01) << (63 - i));
    }
      printf("[+] subkey %d\n",key_i+1);
      print_binary64(keys48b[key_i]);
  }
}
void key_gen(uint64_t *keys48b, uint64_t key)
{

  uint32_t shifted_key_pairs[16][2]= {0};
  uint64_t key_permutated = 0;

  key_permutated = key_permutation(key);
  shifting_key_pairs(shifted_key_pairs,key_permutated);
  form_keys48b(shifted_key_pairs, keys48b);

}



uint64_t initial_permutation(uint64_t data_block)
{
  uint64_t block_IP = 0;

  for (int i = 0; i < 64; i++){
    block_IP |= ((( data_block >> (64 - IP[i]) )&0x01) << (63 - i));
  }
  printf("[+] Initial permutation:\n");
  print_binary64(block_IP);
  return block_IP;
}

void div_block64_to_blocks32(uint32_t* left, uint32_t* right, uint64_t block64)
{
  *left = (uint32_t)(block64 >> 32);
  *right = (uint32_t)(block64 & 0xFFFFFFFF);
  printf("[+] left block half:\n");
  print_binary32(*left);
  printf("[+] right block half:\n");
  print_binary32(*right);
  return;
}

void sblock_fill(uint8_t sblock6b[8], uint64_t block64b)
{
  // for (int i = 8; i > 0; i--){
  //   sblock6b[8-i] = ((uint8_t)((block64b >> (16 + 6*(i-1))) & 0b111111)<<2);
  //   printf("[+] S block: %d\n",9-i);
  //   print_binary8(sblock6b[8-i]);
  // }
  for (int i = 0; i < 8; i++) {
    sblock6b[i] = ((uint8_t)((block64b >> (16 + 6*(7-i))) & 0b111111)<<2);
    printf("[+] S block: %d\n", i+1);
    print_binary8(sblock6b[i]);
  }
}

uint8_t sblock_resolve(uint8_t sblock6b, int i)
{
  uint8_t raw = (((sblock6b & 0b10000000) >> 7)<<1) | ((sblock6b & 0b00000100)>>2);
  printf("raw:%d\n",raw);
  uint8_t col = (sblock6b & 0b01111000) >> 3;
  printf("col:%d\n",col);
  uint8_t res = S[i][raw][col];
  printf("res:%d\n",res);
  return res;
}

uint32_t join4b_to_32b(uint8_t sblock4b[8])
{
  uint32_t res = 0;
  for (int i = 0; i < 8; i++){
    res |= (((uint32_t)sblock4b[i])<<(4*(7-i)));
  }
  printf("[+] Sblock joined to 32b:\n");
  print_binary32(res);
  return res;
  
}

uint32_t permutation(uint32_t sblock_joined)
{
  uint32_t res = 0;
  for (int i = 0; i < 32; i++){
    res |= ((sblock_joined >> (32-P[i])) & 1) << (31-i);
  }
  printf("[+] Block_p:\n");
  print_binary32(res);
  return res;
}

uint64_t concat_blocks32_to64(uint32_t left_block32, uint32_t right_block32)
{
  uint64_t res = 0;
  res = (((uint64_t)right_block32)<<32)|((uint64_t)left_block32);
  return res;
}

uint64_t final_permutation(uint64_t block64)
{
  uint64_t res = 0;
  for (int i = 0; i < 64; i++){
    res |= (((block64 >> (64 - PI[i]))) & 0x1)<<(63 - i);
  }
  return res;
}

uint64_t encrypt_DES(uint64_t data_block, uint64_t keys48b[16])
{
  uint32_t left_data = 0;
  uint32_t right_data = 0;
  uint32_t block_xored = 0;
  uint64_t block_IP;
  uint64_t block_concated = 0;
  uint64_t block_crypted = 0;

  block_IP = initial_permutation(data_block);
  div_block64_to_blocks32(&left_data, &right_data, block_IP);
  

  for (int step = 0; step < 16; step++){
    printf("[+]--------------[ STEP:| %d | ]--------------[+]\n",step+1);
    uint64_t block_E = 0;
    uint8_t sblock6b[8] = {0};
    uint8_t sblock4b[8] = {0};
    uint32_t sblock_joined = 0;
    uint32_t block_P = 0;

    for (int i = 0; i < 48; i++){
      block_E |= (((uint64_t)(right_data >> (32 - E[i]))) & 0x1)<<(63 - i);
    }
    printf("[+] Block_e:\n");
    print_binary64(block_E);

    block_E ^= keys48b[step];

    printf("[+] block xored with key:\n");
    print_binary64(block_E);

    sblock_fill(sblock6b,block_E);
    for (int i = 0; i < 8; i++){
      sblock4b[i] = sblock_resolve(sblock6b[i],i);
      printf("[+] Sblock 4b %d:\n",i);
      print_binary8(sblock4b[i]);
    }

    sblock_joined = join4b_to_32b(sblock4b);
    block_P = permutation(sblock_joined);

    left_data = left_data ^ block_P;
    printf("[+] Left block after XOR:\n");
    print_binary32(left_data);


    //SWAP left and right
    uint32_t temp = left_data;
    left_data = right_data;
    right_data = temp;
  }

  block_concated = concat_blocks32_to64(left_data,right_data);
  printf("[+] Concated block:\n");
  print_binary64(block_concated);

  block_crypted = final_permutation(block_concated);
  printf("[+] Crypted block:\n");
  print_binary64(block_crypted);
  
  return block_crypted;
}

uint64_t* reverse_array(const uint64_t arr[], int size)
{
    uint64_t* reversed = (uint64_t*) malloc(size * sizeof(uint64_t));  // Dynamically allocate a new array for the reversed result
    memset(reversed, 0, size * sizeof(uint64_t));
    if (reversed == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < size; ++i) {
        reversed[i] = arr[size - 1 - i];  // Copy elements from the end of the original array to the new array
    }
    return reversed;  // Return the pointer to the new reversed array
}

void printchars(uint64_t value) 
{
    // Iterate through each of the 8 bytes (characters) starting from the most significant byte
    for (int i = 7; i >= 0; --i) {
        char ch = (char)((value >> (i * 8)) & 0xFF);  // Extract each byte
        printf("%c", ch);  // Print the character
    }
}


