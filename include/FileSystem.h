#pragma once
#include "Archive.h"
#include "Byte.h"
#include "Directory.h"
#include "Disk.h"
#include "Fat.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

class FileSystem {
private:
  Disk &disk;
  Fat fat;
  Directory *currentDirectory;
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
