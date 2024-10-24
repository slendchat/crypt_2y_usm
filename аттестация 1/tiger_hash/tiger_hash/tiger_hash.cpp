#include "outputs.h"
#include "tigerhash.h"
#include "sboxes.h"
#include <string>
#include <iostream>

using namespace std;
int main()
{
    string input;
    info("input string to hash for example your initials:\n");
    getline(cin, input);

    string hash_in_hex = tiger::hash(input);

    okay("Your hashed text\n");
    printf("\t\\____[%s]", hash_in_hex.data());

    return EXIT_SUCCESS;
}