#include "FileSystem.h"

FileSystem::FileSystem(Disk &_disk) : currentDirectory(nullptr), disk(_disk) {
  readFat();
  readRootDir();
}

FileSystem::~FileSystem() {
  delete currentDirectory;
  delete[] fat.table;
}

void FileSystem::readFat() {
  unsigned int length = bytes16ToInt(disk.bs.sectorsPerFat) *
                        bytes16ToInt(disk.bs.bytesPerSector);
  fat = Fat(length, bytes16ToInt(disk.bs.reservedAreaSectors),
            bytes16ToInt(disk.bs.sectorsPerFat));

  disk.readSectors(fat.startSector, fat.totalSectors, fat.table);
}

void FileSystem::readRootDir() {
  unsigned int lba = bytes16ToInt(disk.bs.reservedAreaSectors) +
                     bytes16ToInt(disk.bs.sectorsPerFat) * disk.bs.fatCount;
  unsigned int size =
      sizeof(DirectoryEntry) * bytes16ToInt(disk.bs.rootEntriesMax);
  unsigned int sectors = (size / bytes16ToInt(disk.bs.bytesPerSector));
  if (size % bytes16ToInt(disk.bs.bytesPerSector) > 0) {
    sectors++;
  }
  rootDirEnd = lba + sectors;
  printf("%u, %u\n", lba, rootDirEnd);
  currentPath = "/";
  // allocDirectory(sectors);
  if (currentDirectory != nullptr)
    delete currentDirectory;
  currentDirectory =
      new Directory(sectors, 0, bytes16ToInt(disk.bs.bytesPerSector));
  disk.readSectors(lba, sectors, currentDirectory->entries);
}

unsigned int FileSystem::getFreeCluster() {
  for (int i = 3; i < fat.size; i++) {
    // printf("Fat %u: %04x\n", i, fat.get(i));
    if (fat.get(i) == NOT_ALLOCATED) {
      return i;
    }
  }
  return -1;
}

unsigned int FileSystem::getClusterSector(unsigned int cluster) {
  if (cluster == 0) {
    return bytes16ToInt(disk.bs.reservedAreaSectors) +
           bytes16ToInt(disk.bs.sectorsPerFat) * disk.bs.fatCount;
  }
  return (cluster - 2) * disk.bs.sectorsPerCluster + rootDirEnd;
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
    currentDirectory = new Directory(disk.bs.sectorsPerCluster, cluster,
                                     bytes16ToInt(disk.bs.bytesPerSector));
    disk.readSectors(lba, disk.bs.sectorsPerCluster, currentDirectory->entries);
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

    fat.set(cluster, EOF_MARK);
    Directory *dir = new Directory(disk.bs.sectorsPerCluster, cluster,
                                   bytes16ToInt(disk.bs.bytesPerSector),
                                   currentDirectory->getCluster());

    disk.writeSectors(getClusterSector(currentDirectory->getCluster()),
                      currentDirectory->getSectors(),
                      currentDirectory->entries);
    disk.writeSectors(fat.startSector, fat.totalSectors, fat.table);
    disk.writeSectors(getClusterSector(dir->getCluster()), dir->getSectors(),
                      dir->entries);
    delete newDir;
    delete dir;
  }
}

void FileSystem::createFile(const char *filename) {
  string name = parseFileName(filename);

  string input;
  std::getline(std::cin, input);

  DirectoryEntry *newFile = currentDirectory->createFile(
      name.c_str(), getFreeCluster(), input.length());

  unsigned int bytesPerCluster =
      bytes16ToInt(disk.bs.bytesPerSector) * disk.bs.sectorsPerCluster;

  byte *data = new byte[bytesPerCluster];

  for (int i = 0; i < input.length(); i++) {
    data[i] = input[i];
  }

  fat.set(bytes16ToInt(newFile->clusterLow), EOF_MARK);

  disk.writeSectors(getClusterSector(currentDirectory->getCluster()),
                    currentDirectory->getSectors(), currentDirectory->entries);
  disk.writeSectors(fat.startSector, fat.totalSectors, fat.table);
  disk.writeSectors(getClusterSector(bytes16ToInt(newFile->clusterLow)),
                    disk.bs.sectorsPerCluster, data);

  delete newFile;
  return;
}

void FileSystem::catFile(const char *name) {
  DirectoryEntry *dirFile =
      currentDirectory->findEntry(parseFileName(name).c_str());

  if (dirFile == NULL || dirFile->isDir()) {
    printf("El archivo %s no existe\n", parseFileName(name).c_str());
    return;
  }
  Archive archivo(dirFile->clusterLow, dirFile->size, disk, fat, rootDirEnd);
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
  delete[] ext;
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
  printf("JumpInstruction: %u\n", bytesToInt(disk.bs.bootJumpIns, 3));
  printf("OEM Name: %s\n", disk.bs.oemName);
  printf("Bytes per Sector: %u\n", bytesToInt(disk.bs.bytesPerSector, 2));
  printf("Sectors per Cluster: %u\n", disk.bs.sectorsPerCluster);
  printf("Reserved Area: %u\n", bytesToInt(disk.bs.reservedAreaSectors, 2));
  printf("Fat Count: %u\n", disk.bs.fatCount);
  printf("Max root entries: %u\n", bytesToInt(disk.bs.rootEntriesMax, 2));
  printf("Total Sectors: %u\n", bytesToInt(disk.bs.totalSectors, 2));
  printf("Media Type: %u\n", disk.bs.mediaType);
  printf("Sectors per Fat: %u\n", bytesToInt(disk.bs.sectorsPerFat, 2));
  printf("Sectors Per Track: %u\n", bytesToInt(disk.bs.sectorsPerTrack, 2));
  printf("Heads: %u\n", bytesToInt(disk.bs.heads, 2));
  printf("Hidden Sectors: %u\n", bytesToInt(disk.bs.hiddenSectors, 4));
  printf("Large Sector Count: %u\n", bytesToInt(disk.bs.largeSectorCount, 4));
  printf("Drive Number: %u\n", disk.bs.driveNumber);
  printf("Not used: %u\n", disk.bs._notUsed);
  printf("Signature: %#08x\n", disk.bs.signature);
  printf("Serial Number: %#08x\n", bytesToInt(disk.bs.serialNumber, 4));
  printf("Volume Label: %s\n", disk.bs.volumeLabel);
  printf("FS Type: %s\n", disk.bs.systemId);
  printf("--- Boot Sector ---\n");
}
