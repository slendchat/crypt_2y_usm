#pragma once
#include <stdio.h>

#define okay(msg, ...) printf("[+]" msg,__VA_ARGS__);
#define info(msg, ...) printf("[i]" msg,__VA_ARGS__);
#define warn(msg, ...) printf("[!]" msg,__VA_ARGS__);