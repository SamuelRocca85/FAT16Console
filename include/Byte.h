#pragma once
#include <cstring>
#include <string>

using std::string;

typedef unsigned char byte;

unsigned int bytesToInt(byte *bytes, int size);

unsigned int bytes16ToInt(byte *bytes);

unsigned int bytes32ToInt(byte *bytes);

string bytesToString(byte *bytes);
