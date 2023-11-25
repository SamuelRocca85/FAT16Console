#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

typedef unsigned char byte;

using std::fstream;
using std::string;

struct BootSector {
  byte bootJumpIns[3];
  byte oemName[8];
  byte bytesPerSector[2];
  byte sectorsPerCluster;
  byte reservedAreaSectors[2];
  byte fatCount;
  byte rootEntriesMax[2];
  byte totalSectors[2];
  byte mediaType;
  byte sectorsPerFat[2];
  byte sectorsPerTrack[2];
  byte heads[2];
  byte hiddenSectors[4];
  byte largeSectorCount[4];

  byte driveNumber;
  byte _notUsed;
  byte signature;
  byte serialNumber[4];
  byte volumeLabel[11];
  byte systemId[8];
};

struct DirectoryEntry {
  byte name[11];
  byte attributes;
  byte reserved;
  byte createdTimeTenths;
  byte createdTime[2];
  byte createdDate[2];
  byte accesedDate[2];
  byte clusterHigh[2];
  byte modifiedTime[2];
  byte modifiedDate[2];
  byte clusterLow[2];
  byte size[4];

  bool isDir() { return (attributes & 0x10) != 0; }
  bool isValid() {
    char firstByte = name[0];
    return !(firstByte == 0xE5 || firstByte == 0x00);
  }
};

class FileSystem {

private:
  BootSector bootSector;
  DirectoryEntry *currentDirectory;
  fstream *file;
  byte *fat;
  string currentPath;
  unsigned int currentCluster;
  unsigned int fatLength;
  unsigned int rootDirEnd;

public:
  FileSystem(string file);
  ~FileSystem();
  void readBootSector();
  void readFat();
  void readRootDir();
  bool readSectors(int lba, int count, void *buffer);
  bool writeSectors(int lba, int count, void *buffer);

  unsigned int getFreeCluster();
  unsigned int getClusterSector(unsigned int cluster);
  void allocDirectory(unsigned int sectors);

  DirectoryEntry *findFile(const char *filename);

  unsigned int bytesToInt(byte *bytes, size_t size);
  unsigned int bytes16ToInt(byte *bytes);
  unsigned int bytes32ToInt(byte *bytes);
  void print();
  string getPath() { return currentPath; }
  void returnPath();
  void changePath(string dirname);

  // Metodos para los comandos
  void listFiles();
  void changeDir(const char *dirname);
  void makeDir(string name);
};
