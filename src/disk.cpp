#include "Disk.h"

Disk::Disk(const char *imageName) {
  file = fstream(imageName, std::ios::in | std::ios::out | std::ios::binary);

  if (!file.is_open()) {
    std::cout << "No se pudo abrir la imagen " << imageName << "\n";
    exit(1);
  }

  readBootSector();
}

Disk::~Disk() { file.close(); }

void Disk::readBootSector() {
  file.read(reinterpret_cast<char *>(&bs), sizeof(BootSector));
  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file.close();
    exit(1);
  }
}

bool Disk::readSectors(int lba, int sectors, void *buffer) {
  file.seekg(lba * bytes16ToInt(bs.bytesPerSector), file.beg);

  unsigned int bytesToRead = sectors * bytes16ToInt(bs.bytesPerSector);
  file.read(reinterpret_cast<char *>(buffer), bytesToRead);

  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file.close();
    exit(1);
  }

  return true;
}

bool Disk::writeSectors(int lba, int sectors, void *buffer) {
  file.seekp(lba * bytes16ToInt(bs.bytesPerSector), file.beg);

  unsigned int bytesToWrite = sectors * bytes16ToInt(bs.bytesPerSector);
  // printf("Reading %u bytes...\n", bytesToRead);
  file.write(reinterpret_cast<char *>(buffer), bytesToWrite);

  if (!file) {
    std::cerr << "Error al escribir el archivo" << std::endl;
    file.close();
    exit(1);
  }

  return true;
}
