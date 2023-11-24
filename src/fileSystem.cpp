#include "FileSystem.h"
#include <cstdio>
#include <cstring>

FileSystem::FileSystem(string filename) {
  file = new fstream(filename, std::ios::in | std::ios::out | std::ios::binary);

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
  fatLength = length;
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
  rootDirEnd = lba + sectors;
  printf("Aqui termina el root %u\n", rootDirEnd);
  rootDirectory =
      new DirectoryEntry[sectors * bytes16ToInt(bootSector.bytesPerSector)];
  readSectors(lba, sectors, rootDirectory);
}

bool FileSystem::readSectors(int lba, int sectors, void *buffer) {
  file->seekg(lba * bytes16ToInt(bootSector.bytesPerSector), file->beg);

  unsigned int bytesToRead = sectors * bytes16ToInt(bootSector.bytesPerSector);
  // printf("Reading %u bytes...\n", bytesToRead);
  file->read(reinterpret_cast<char *>(buffer), bytesToRead);

  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file->close();
    exit(1);
  }

  return true;
}

unsigned int FileSystem::getFreeCluster() {
  for (int i = 0; i < fatLength; i++) {
    if (fat[i] == 0x00) {
      return i;
    }
  }
  return -1;
}

unsigned int FileSystem::getClusterSector(unsigned int cluster) {
  return (cluster - 2) * bootSector.sectorsPerCluster + rootDirEnd;
}

void FileSystem::listFiles() {
  for (unsigned int i = 0; i < bytes16ToInt(bootSector.rootEntriesMax); i++) {
    const char firstByte = rootDirectory[i].name[0];
    if (firstByte == 0xE5 || firstByte == 0x00) {
      continue;
    }
    if (rootDirectory[i].isDir()) {
      unsigned int lowBytes = bytes16ToInt(rootDirectory[i].clusterLow);
      printf("low %u, d: ", lowBytes);
    } else {
      printf("f: ");
    }
    printf("%s \n", rootDirectory[i].name);
  }
  printf("\n");
}

void FileSystem::changeDir(const char *dirname) {
  DirectoryEntry *dir = findFile(dirname);
  if (dir != NULL && dir->isDir()) {
    // printf("Found dir %s\n", dir->name);
    unsigned int cluster = bytes16ToInt(dir->clusterLow);
    unsigned int lba = getClusterSector(cluster);
    // printf("cd to cluster %u, sector %u\n", cluster, lba);
    readSectors(lba, bootSector.sectorsPerCluster, rootDirectory);
  } else {
    printf("directorio %s no existe\n", dirname);
  }
}

void FileSystem::makeDir(string name) {
  // file->seekp()
  unsigned int cluster = getFreeCluster();
  if (cluster != -1) {
    unsigned int lba = getClusterSector(cluster);
    printf("Creando dir en cluster %u, sector %u\n", cluster, lba);
    readSectors(lba, bootSector.sectorsPerCluster, rootDirectory);
  }
}

DirectoryEntry *FileSystem::findFile(const char *filename) {
  for (unsigned int i = 0; i < bytes16ToInt(bootSector.rootEntriesMax); i++) {
    if (std::memcmp(filename, rootDirectory[i].name, 11) == 0) {
      // printf("%s == %s\n", filename, rootDirectory[i].name);
      return &rootDirectory[i];
    }
  }

  return NULL;
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
