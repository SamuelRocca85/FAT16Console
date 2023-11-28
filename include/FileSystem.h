#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
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
  bool isValid() { return !(name[0] == 0xE5 || name[0] == 0x00); }

  DirectoryEntry(const char *_name, unsigned int _cluster) {
    byte highEmpty[2] = {0, 0};
    memcpy(name, _name, 11);
    memcpy(clusterLow, &_cluster, 2);
    memcpy(clusterHigh, highEmpty, 2);
    attributes = 0x10;
    setDates();
    setTimes();
  }

  void setDates() {
    time_t currTime;
    currTime = time(NULL);
    tm *time = localtime(&currTime);
    uint16_t fecha;
    uint16_t year = time->tm_year - 80;
    fecha |= static_cast<uint16_t>(time->tm_mday);
    fecha |= static_cast<uint16_t>(time->tm_mon << 5);
    fecha |= static_cast<uint16_t>(year << 9);
    memcpy(createdDate, &fecha, 2);
    memcpy(accesedDate, &fecha, 2);
    memcpy(modifiedDate, &fecha, 2);
  }

  void setTimes() {
    time_t currTime;
    currTime = time(NULL);
    tm *time = localtime(&currTime);
    uint16_t tiempo;
    tiempo |= static_cast<uint16_t>(time->tm_sec);
    tiempo |= static_cast<uint16_t>(time->tm_min << 5);
    tiempo |= static_cast<uint16_t>(time->tm_hour << 11);
    memcpy(createdTime, &tiempo, 2);
    memcpy(modifiedTime, &tiempo, 2);
  }

  void printDate() {
    uint16_t date;
    memcpy(&date, createdDate, 2);
    uint16_t y = ((date & 0b1111111000000000) >> 9);
    uint16_t m = ((date & 0b0000000111100000) >> 5);
    uint16_t d = date & 0b0000000000011111;

    printf("%u/%u/%u", d, m, y + 1980);
  }

  void printTime() {
    uint16_t time;
    memcpy(&time, createdTime, 2);
    uint16_t h = ((time & 0b1111100000000000) >> 11);
    uint16_t m = ((time & 0b0000011111100000) >> 5);
    uint16_t s = (time & 0b0000000000011111);

    printf("%u:%u:%u", h, m, s);
  }

  DirectoryEntry() {
    //
    name[0] = 0x00;
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
  string parseFileName(string filename);
  void changePath(string dirname);

  // Metodos para los comandos
  void listFiles();
  void changeDir(const char *dirname);
  void makeDir(string name);
  void catFile(string filename);
};
