#pragma once
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iterator>
#include <string>

typedef unsigned char byte;

enum DirectoryAttribute {
  READ_ONLY = 0x01,
  HIDDEN = 0x02,
  SYSTEM_FILE = 0x04,
  VOLUME_LABEL = 0x08,
  LONG_NAME = 0x0f,
  DIRECTORY = 0x10,
  ARCHIVE = 0x20,
};
#pragma pack(push, 1)
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

  bool isDir() { return (attributes & DIRECTORY) != 0; }
  bool isVolumeLabel() { return (attributes & VOLUME_LABEL) != 0; }
  bool isValid() { return !(name[0] == 0xE5 || name[0] == 0x00); }

  DirectoryEntry(const char *_name, unsigned int _cluster, byte _attributes) {
    byte highEmpty[2] = {0, 0};
    memcpy(name, _name, 11);
    memcpy(clusterLow, &_cluster, 2);
    memcpy(clusterHigh, highEmpty, 2);
    attributes = _attributes;
    time_t currTime;
    currTime = time(NULL);
    tm *time = localtime(&currTime);
    setDates(time);
    setTimes(time);
  }

  void setDates(tm *time) {
    uint16_t fecha = 0;
    uint16_t year = time->tm_year - 80;
    fecha |= static_cast<uint16_t>(time->tm_mday);
    fecha |= static_cast<uint16_t>((time->tm_mon + 1) << 5);
    fecha |= static_cast<uint16_t>(year << 9);
    memcpy(createdDate, &fecha, 2);
    memcpy(accesedDate, &fecha, 2);
    memcpy(modifiedDate, &fecha, 2);
  }

  void setTimes(tm *time) {
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
    // uint16_t s = (time & 0b0000000000011111);

    printf("%u:%u", h, m);
  }

  DirectoryEntry() {
    //
    name[0] = 0x00;
  }
};
#pragma pack(pop)

class Directory {

private:
  unsigned int sectors;
  unsigned int size; // Cantidad de entries
  unsigned int cluster;

public:
  DirectoryEntry *entries;
  Directory(unsigned int sectors, unsigned int cluster, unsigned int bps);
  Directory(unsigned int sectors, unsigned int cluster, unsigned int bps,
            unsigned int parentCluster);
  ~Directory();
  DirectoryEntry *findEntry(const char *entryName);
  DirectoryEntry *createEntry(const char *name, unsigned int cluster);

  unsigned int getSize() { return size; }
  unsigned int getCluster() { return cluster; }
  unsigned int getSectors() { return sectors; }
};
