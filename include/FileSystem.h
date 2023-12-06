#pragma once
#include "Directory.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

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

enum FatMark {
  NOT_ALLOCATED = 0x00,
  EOF_MARK = 0xffff,
  BAD_CLUSTER = 0xfff8,
};

struct Fat {
  byte *table;
  unsigned int length; // En bytes
  unsigned int size;   // En clusters
  unsigned int startSector;
  unsigned int totalSectors;

  Fat() {
    length = 0;
    size = 0;
    startSector = 0;
    totalSectors = 0;
    table = NULL;
  }
  Fat(unsigned int _length, unsigned int _startSector,
      unsigned int _totalSectors)
      : length(_length), startSector(_startSector),
        totalSectors(_totalSectors) {
    table = new byte[length];
    size = length / 2;
  }

  unsigned short get(unsigned int cluster) {
    unsigned int idx = cluster * 2;
    return (table[idx] << 8) | table[idx + 1];
  }

  void set(unsigned int cluster, FatMark mark) {
    unsigned int idx = cluster * 2;
    table[idx] = static_cast<unsigned char>(mark >> 8);
    table[idx + 1] = static_cast<unsigned char>(mark);
  }
};

class FileSystem {
private:
  BootSector bootSector;
  Directory *currentDirectory;
  Fat fat;
  fstream *file;
  string currentPath;
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

  unsigned int bytesToInt(byte *bytes, size_t size);
  unsigned int bytes16ToInt(byte *bytes);
  unsigned int bytes32ToInt(byte *bytes);
  string bytesToString(byte *bytes);
  void print();
  string getPath() { return currentPath; }
  void returnPath();
  string parseFileName(string filename);
  string prettyFileName(string filename);
  void changePath(string dirname);

  // Metodos para los comandos
  void listFiles();
  void changeDir(const char *dirname);
  void makeDir(const char *name);
  void catFile(const char *filename);
  void createFile(const char *filename);
};
