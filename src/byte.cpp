#include "Byte.h"

unsigned int bytesToInt(byte *bytes, int size) {
  unsigned int resultado = 0;
  std::memcpy(&resultado, bytes, size);
  return resultado;
}

unsigned int bytes16ToInt(byte *bytes) { return bytesToInt(bytes, 2); }

unsigned int bytes32ToInt(byte *bytes) { return bytesToInt(bytes, 4); }

string bytesToString(byte *bytes) {
  return string(reinterpret_cast<char *>(bytes), 11);
}
