#include "FileSystem.h"
#include "Directory.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
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
  // print();
}

FileSystem::~FileSystem() {
  file->close();
  delete file;
  delete currentDirectory;
  delete[] fat.table;
}

void FileSystem::readBootSector() {
  file->read(reinterpret_cast<char *>(&bootSector), sizeof(BootSector));
  if (!file) {
    std::cerr << "Error al leer desde el archivo" << std::endl;
    file->close();
    exit(1);
  }
}

void FileSystem::readFat() {
  unsigned int length = bytes16ToInt(bootSector.sectorsPerFat) *
                        bytes16ToInt(bootSector.bytesPerSector);
  fat = Fat(length, bytes16ToInt(bootSector.reservedAreaSectors),
            bytes16ToInt(bootSector.sectorsPerFat));

  readSectors(fat.startSector, fat.totalSectors, fat.table);
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
  printf("%u, %u\n", lba, rootDirEnd);
  currentPath = "/";
  // allocDirectory(sectors);
  if (currentDirectory != nullptr)
    delete currentDirectory;
  currentDirectory =
      new Directory(sectors, 0, bytes16ToInt(bootSector.bytesPerSector));
  readSectors(lba, sectors, currentDirectory->entries);
}

bool FileSystem::readSectors(int lba, int sectors, void *buffer) {
  file->seekg(lba * bytes16ToInt(bootSector.bytesPerSector), file->beg);

  unsigned int bytesToRead = sectors * bytes16ToInt(bootSector.bytesPerSector);
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
  for (int i = 0; i < fat.length; i++) {
    if (fat.table[i] == 0x00) {
      return i;
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
  for (unsigned int i = 0; i < currentDirectory->getSize(); i++) {
    if (!currentDirectory->entries[i].isValid()) {
      break;
    } else if (currentDirectory->entries[i].isVolumeLabel()) {
      continue;
    }
    string name = "";
    if (currentDirectory->entries[i].isDir()) {
      printf("d: ");
      name = bytesToString(currentDirectory->entries[i].name);
    } else {
      printf("f: ");
      name = bytesToString(currentDirectory->entries[i].name);
    }
    currentDirectory->entries[i].printDate();
    printf(" ");
    currentDirectory->entries[i].printTime();
    printf("\t%u\t%s\n", bytes16ToInt(currentDirectory->entries[i].clusterLow),
           name.c_str());
  }
  printf("\n");
}

void FileSystem::changeDir(const char *dirname) {
  DirectoryEntry *dir = currentDirectory->findEntry(dirname);
  if (dir != NULL && dir->isDir()) {
    unsigned int cluster = bytes16ToInt(dir->clusterLow);
    if (cluster == 0) {
      readRootDir();
      return;
    }
    unsigned int lba = getClusterSector(cluster);
    if (currentDirectory != nullptr) {
      delete currentDirectory;
    }
    currentDirectory = new Directory(bootSector.sectorsPerCluster, cluster,
                                     bytes16ToInt(bootSector.bytesPerSector));
    readSectors(lba, bootSector.sectorsPerCluster, currentDirectory->entries);
    changePath(dirname);
  } else {
    printf("directorio %s no existe\n", dirname);
  }
}

void FileSystem::makeDir(const char *name) {
  unsigned int cluster = getFreeCluster();
  if (cluster != -1) {
    unsigned int lba = getClusterSector(cluster);
    DirectoryEntry *newDir = currentDirectory->createSubDir(name, cluster);

    if (newDir == NULL) {
      printf("No se pudo crear el directorio\n");
      return;
    }

    fat.table[cluster] = 0xf8;
    Directory *dir = new Directory(bootSector.sectorsPerCluster, cluster,
                                   bytes16ToInt(bootSector.bytesPerSector),
                                   currentDirectory->getCluster());

    writeSectors(getClusterSector(currentDirectory->getCluster()),
                 currentDirectory->getSectors(), currentDirectory->entries);
    writeSectors(fat.startSector, fat.totalSectors, fat.table);
    writeSectors(getClusterSector(dir->getCluster()), dir->getSectors(),
                 dir->entries);
    delete newDir;
    delete dir;
  }
}

void FileSystem::createFile(const char *filename) {
  //
  string name = parseFileName(filename);

  // printf("Archivo %s: %s\n", filename, name.c_str());
  // std::cout << filename << "," << name << "\n";
  currentDirectory->createFile(name.c_str(), getFreeCluster());
  return;
}

void FileSystem::catFile(const char *name) {
  DirectoryEntry *dirFile =
      currentDirectory->findEntry(parseFileName(name).c_str());

  if (dirFile != NULL && !dirFile->isDir()) {
    printf("Encontre el archivo %s\n", dirFile->name);
  }
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

string FileSystem::bytesToString(byte *bytes) {
  return string(reinterpret_cast<char *>(bytes), 11);
}

string FileSystem::parseFileName(string filename) {
  string parsedName = "";
  char *ext = new char[3];

  ext[0] = filename[filename.length() - 3];
  ext[1] = filename[filename.length() - 2];
  ext[2] = filename[filename.length() - 1];
  // ext[3] = '\0';
  for (int i = 0; i < filename.length(); i++) {
    if (filename[i] == '.') {
      parsedName[i] = ' ';
      break;
    }
    parsedName.append(1, filename[i]);
  }
  if (parsedName.length() < 8) {
    parsedName.append(8 - parsedName.length(), ' ');
  }
  parsedName.append(string(ext));
  return parsedName;
}

string FileSystem::prettyFileName(string filename) {
  string parsedName = "";
  char *ext = new char[3];
  // printf("Pretty %s, %u\n", filename.c_str(), filename.length());
  ext[0] = filename[filename.length() - 3];
  ext[1] = filename[filename.length() - 2];
  ext[2] = filename[filename.length() - 1];
  // ext[3] = '\0';
  for (int i = 0; i < filename.length(); i++) {
    if (filename[i] == ' ') {
      parsedName.append(1, '.');
      break;
    }
    parsedName.append(1, filename[i]);
  }
  parsedName.append(string(ext));
  return parsedName;
}

void FileSystem::changePath(string dirname) {
  if (string(dirname) == "..         ") {
    returnPath();
  } else if (string(dirname) == ".          ") {
    return;
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
  currentPath += "/";
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
