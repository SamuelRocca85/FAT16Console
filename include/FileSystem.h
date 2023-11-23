#pragma once
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

typedef unsigned char byte;

using std::ifstream;
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
  byte hiddenSectors[3];
  byte largeSectorCount[4];

  // Remainder of boot sector

  byte driveNumber;
  byte _notUsed;
  byte signature;
  byte serialNumber[4];
  byte volumeLabel[11];
  byte systemId[8];

  void print() {
    printf("\n--- Boot Sector ---\n");
    printf("JumpInstruction: %u\n", bootJumpIns);
    printf("OEM Name: %s\n", oemName);
    printf("Bytes per Sector: %u\n", bytesPerSector);
    printf("Sectors per Cluster: %u\n", sectorsPerCluster);
    printf("Reserved Area: %u\n", reservedAreaSectors);
    printf("Fat Count: %u\n", fatCount);
    printf("Max root entries: %u\n", rootEntriesMax);
    printf("Total Sectors: %u\n", totalSectors);
    printf("Media Type: %u\n", mediaType);
    printf("Sectors per Fat: %u\n", sectorsPerFat);
    printf("Sectors Per Track: %u\n", sectorsPerTrack);
    printf("Heads: %u\n", heads);
    printf("Hidden Sectors: %u\n", hiddenSectors);
    printf("Large Sector Count: %u\n", largeSectorCount);
    printf("Drive Number: %u\n", driveNumber);
    printf("Not used: %u\n", _notUsed);
    printf("Signature: %#08x\n", signature);
    printf("Serial Number: %#08x\n", serialNumber);
    printf("Volume Label: %s\n", volumeLabel);
    printf("FS Type: %s\n", systemId);
    printf("--- Boot Sector ---\n");
  }
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
  byte size[3];
};

class FileSystem {

private:
  BootSector bootSector;
  ifstream *file;
  char *fat;

public:
  FileSystem(string file);
  ~FileSystem();
  void readBootSector();
  void readFat();
  bool readSectors(int lba, int count, char *buffer);
  unsigned int bytesToInt(byte *bytes);
  void print();
};
