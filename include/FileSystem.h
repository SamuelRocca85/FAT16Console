#pragma once
#include "Directory.h"
#include "Disk.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

using std::fstream;
using std::string;

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
  Disk &disk;
  Directory *currentDirectory;
  Fat fat;
  string currentPath;
  unsigned int rootDirEnd;

public:
  FileSystem(Disk &disk);
  ~FileSystem();
  void readFat();
  void readRootDir();

  unsigned int getFreeCluster();
  unsigned int getClusterSector(unsigned int cluster);

  void print();
  string getPath() { return currentPath; }
  void returnPath();
  string parseFileName(string filename);
  void changePath(string dirname);

  // Metodos para los comandos
  void listFiles();
  void changeDir(const char *dirname);
  void makeDir(const char *name);
  void catFile(const char *filename);
  void createFile(const char *filename);
};
