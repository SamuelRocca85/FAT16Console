#include "FileSystem.h"

FileSystem::FileSystem(string filename) {
  file = new ifstream(filename, std::ios::binary);

  if (!file->is_open()) {
    std::cerr << "Error al abrir el archivo" << std::endl;
    exit(1);
  }
  readBootSector();
  readFat();
  readRootDir();
}

FileSystem::~FileSystem() {
  file->close();
  delete file;
  delete[] fat;
  delete[] rootDirectory;
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
  unsigned int length = bytes16ToInt(bootSector.sectorsPerFat) *
                        bytes16ToInt(bootSector.bytesPerSector);
  fat = new byte[length];
  readSectors(bytes16ToInt(bootSector.reservedAreaSectors),
              bytes16ToInt(bootSector.sectorsPerFat), fat);
}

void FileSystem::readRootDir() {
  unsigned int lba =
      bytes16ToInt(bootSector.reservedAreaSectors) +
      bytes16ToInt(bootSector.sectorsPerFat) * bootSector.fatCount;
  unsigned int size =
      sizeof(DirectoryEntry) * bytes16ToInt(bootSector.rootEntriesMax);
  unsigned int sectors = (size / bytes16ToInt(bootSector.bytesPerSector));
  if (size % bytes16ToInt(bootSector.bytesPerSector) > 0) {
    sectors++;
  }
  rootDirectory =
      new DirectoryEntry[sectors * bytes16ToInt(bootSector.bytesPerSector)];
  readSectors(lba, sectors, rootDirectory);
}

bool FileSystem::readSectors(int lba, int sectors, void *buffer) {
  file->seekg(lba * bytes16ToInt(bootSector.bytesPerSector), file->beg);

  unsigned int bytesToRead = sectors * bytes16ToInt(bootSector.bytesPerSector);
  printf("Reading %u bytes...\n", bytesToRead);
  file->read(reinterpret_cast<char *>(buffer), bytesToRead);

  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file->close();
    exit(1);
  }

  return true;
}

unsigned int FileSystem::bytesToInt(byte *bytes, size_t size) {
  unsigned int resultado = 0;
  std::memcpy(&resultado, bytes, size);
  return resultado;
}

unsigned int FileSystem::bytes16ToInt(byte *bytes) {
  return bytesToInt(bytes, 2);
}

unsigned int FileSystem::bytes32ToInt(byte *bytes) {
  return bytesToInt(bytes, 4);
}

void FileSystem::print() {
  printf("\n--- Boot Sector ---\n");
  printf("JumpInstruction: %u\n", bytesToInt(bootSector.bootJumpIns, 3));
  printf("OEM Name: %s\n", bootSector.oemName);
  printf("Bytes per Sector: %u\n", bytesToInt(bootSector.bytesPerSector, 2));
  printf("Sectors per Cluster: %u\n", bootSector.sectorsPerCluster);
  printf("Reserved Area: %u\n", bytesToInt(bootSector.reservedAreaSectors, 2));
  printf("Fat Count: %u\n", bootSector.fatCount);
  printf("Max root entries: %u\n", bytesToInt(bootSector.rootEntriesMax, 2));
  printf("Total Sectors: %u\n", bytesToInt(bootSector.totalSectors, 2));
  printf("Media Type: %u\n", bootSector.mediaType);
  printf("Sectors per Fat: %u\n", bytesToInt(bootSector.sectorsPerFat, 2));
  printf("Sectors Per Track: %u\n", bytesToInt(bootSector.sectorsPerTrack, 2));
  printf("Heads: %u\n", bytesToInt(bootSector.heads, 2));
  printf("Hidden Sectors: %u\n", bytesToInt(bootSector.hiddenSectors, 4));
  printf("Large Sector Count: %u\n",
         bytesToInt(bootSector.largeSectorCount, 4));
  printf("Drive Number: %u\n", bootSector.driveNumber);
  printf("Not used: %u\n", bootSector._notUsed);
  printf("Signature: %#08x\n", bootSector.signature);
  printf("Serial Number: %#08x\n", bytesToInt(bootSector.serialNumber, 4));
  printf("Volume Label: %s\n", bootSector.volumeLabel);
  printf("FS Type: %s\n", bootSector.systemId);
  printf("--- Boot Sector ---\n");
}
