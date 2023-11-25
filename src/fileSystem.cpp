#include "FileSystem.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

FileSystem::FileSystem(string filename) : currentDirectory(nullptr) {
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
  delete[] currentDirectory;
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
  currentPath = "/";
  currentCluster = 0;
  printf("Sectores del root %u\n", sectors);
  allocDirectory(sectors);
  readSectors(lba, sectors, currentDirectory);
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

bool FileSystem::writeSectors(int lba, int sectors, void *buffer) {
  file->seekp(lba * bytes16ToInt(bootSector.bytesPerSector), file->beg);

  unsigned int bytesToWrite = sectors * bytes16ToInt(bootSector.bytesPerSector);
  // printf("Reading %u bytes...\n", bytesToRead);
  file->write(reinterpret_cast<char *>(buffer), bytesToWrite);

  if (!file) {
    std::cerr << "Error al escribir el archivo" << std::endl;
    file->close();
    exit(1);
  }

  return true;
}

unsigned int FileSystem::getFreeCluster() {
  for (int i = 0; i < fatLength; i++) {
    if (fat[i] == 0x00) {
      return i;
    } else {
      printf("Cluster %u cupado con %02x\n", i, fat[i]);
    }
  }
  return -1;
}

unsigned int FileSystem::getClusterSector(unsigned int cluster) {
  if (cluster == 0) {
    return bytes16ToInt(bootSector.reservedAreaSectors) +
           bytes16ToInt(bootSector.sectorsPerFat) * bootSector.fatCount;
  }
  return (cluster - 2) * bootSector.sectorsPerCluster + rootDirEnd;
}

void FileSystem::listFiles() {
  for (unsigned int i = 0; i < bytes16ToInt(bootSector.rootEntriesMax); i++) {
    if (!currentDirectory[i].isValid()) {
      continue;
    }
    if (currentDirectory[i].isDir()) {
      printf("d: ");
    } else {
      printf("f: ");
    }
    printf("%s - %u\n", currentDirectory[i].name,
           bytes16ToInt(currentDirectory[i].clusterLow));
  }
  printf("\n");
}

void FileSystem::changeDir(const char *dirname) {
  DirectoryEntry *dir = findFile(dirname);
  if (dir != NULL && dir->isDir()) {
    // printf("Found dir %s\n", dir->name);
    unsigned int cluster = bytes16ToInt(dir->clusterLow);
    currentCluster = cluster;
    if (cluster == 0) {
      readRootDir();
      return;
    }
    unsigned int lba = getClusterSector(cluster);
    // printf("cd to cluster %u, sector %u\n", cluster, lba);
    allocDirectory(bootSector.sectorsPerCluster);
    readSectors(lba, bootSector.sectorsPerCluster, currentDirectory);
    changePath(dirname);
  } else {
    printf("directorio %s no existe\n", dirname);
  }
}

void FileSystem::makeDir(string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  unsigned int cluster = getFreeCluster();
  if (cluster != -1) {
    unsigned int lba = getClusterSector(cluster);
    // printf("Creando dir en cluster %u, sector %u\n", cluster, lba);
    DirectoryEntry newDir;
    std::memcpy(newDir.name, name.c_str(), 11);
    newDir.attributes = 0x10;
    std::memcpy(newDir.clusterLow, &cluster, 2);
    for (int i = 0; i < bytes16ToInt(bootSector.rootEntriesMax); i++) {
      if (!currentDirectory[i].isValid()) {
        currentDirectory[i] = newDir;
        fat[cluster] = 0xf8;
        break;
      }
    }

    writeSectors(getClusterSector(currentCluster), bootSector.sectorsPerCluster,
                 currentDirectory);
    writeSectors(bytes16ToInt(bootSector.reservedAreaSectors),
                 bytes16ToInt(bootSector.sectorsPerFat), fat);
  }
}

DirectoryEntry *FileSystem::findFile(const char *filename) {
  for (unsigned int i = 0; i < bytes16ToInt(bootSector.rootEntriesMax); i++) {
    if (std::memcmp(filename, currentDirectory[i].name, 11) == 0) {
      // printf("%s == %s\n", filename, currentDirectory[i].name);
      return &currentDirectory[i];
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

void FileSystem::allocDirectory(unsigned int sectors) {
  if (currentDirectory != nullptr) {
    // printf("Haciendo realloc con %u sectores\n", sectors);
    DirectoryEntry *tmp =
        new DirectoryEntry[sectors * bytes16ToInt(bootSector.bytesPerSector)];
    delete[] currentDirectory;
    currentDirectory = tmp;
    return;
  }
  currentDirectory =
      new DirectoryEntry[sectors * bytes16ToInt(bootSector.bytesPerSector)];
}

void FileSystem::changePath(string dirname) {
  if (string(dirname) == "..         ") {
    returnPath();
  } else {
    string resultado = dirname;
    resultado.erase(
        std::remove_if(resultado.begin(), resultado.end(), ::isspace),
        resultado.end());
    currentPath += resultado + string("/");
  }
}

void FileSystem::returnPath() {
  std::stringstream ss(currentPath);
  std::vector<string> segmentos;

  std::string segmento;
  while (std::getline(ss, segmento, '/')) {
    segmentos.push_back(segmento);
  }

  if (segmentos.size() >= 2) {
    segmentos.pop_back();
  }

  currentPath = segmentos[0];
  for (size_t i = 1; i < segmentos.size(); ++i) {
    currentPath += "/" + segmentos[i];
  }
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
