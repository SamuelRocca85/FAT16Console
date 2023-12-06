#pragma once
#include "Byte.h"
#include <fstream>
#include <iostream>

typedef unsigned char byte;

using std::fstream;

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

class Disk {
private:
  fstream file;

public:
  BootSector bs;
  Disk(const char *imageName);
  ~Disk();
  void readBootSector();
  bool readSectors(int lba, int count, void *buffer);
  bool writeSectors(int lba, int count, void *buffer);
};
