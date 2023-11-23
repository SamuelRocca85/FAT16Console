#include "FileSystem.h"

FileSystem::FileSystem(string filename) {
  file = new ifstream(filename, std::ios::binary);

  if (!file->is_open()) {
    std::cerr << "Error al abrir el archivo" << std::endl;
    exit(1);
  }
  readBootSector();
  readFat();

  // for (int i = 0; i < 100; i++) {
  //   printf("%04x\n", fat[i]);
  // }
}

FileSystem::~FileSystem() {
  file->close();
  delete file;
  delete[] fat;
}

void FileSystem::readBootSector() {
  file->read(reinterpret_cast<char *>(&bootSector), sizeof(bootSector));

  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file->close();
    exit(1);
  }
}

void FileSystem::readFat() {
  unsigned int length = bytesToInt(bootSector.sectorsPerFat) *
                        bytesToInt(bootSector.bytesPerSector);
  fat = new char[length];
  readSectors(bytesToInt(bootSector.reservedAreaSectors),
              bytesToInt(bootSector.sectorsPerFat), fat);
}

bool FileSystem::readSectors(int lba, int count, char *buffer) {
  file->seekg(lba * bytesToInt(bootSector.bytesPerSector), file->beg);

  file->read(buffer, count);

  return true;
}

unsigned int FileSystem::bytesToInt(byte *bytes) {
  unsigned int value = *reinterpret_cast<unsigned int *>(bytes);

  return value;
}

void FileSystem::print() { bootSector.print(); }
