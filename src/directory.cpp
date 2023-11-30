#include "Directory.h"
#include <cstdio>

Directory::Directory(unsigned int _sectors, unsigned int _cluster,
                     unsigned int _bps)
    : cluster(_cluster), sectors(_sectors) {
  int totalBytes = _bps * sectors;
  size = totalBytes / sizeof(DirectoryEntry);
  entries = new DirectoryEntry[size];
}

Directory::Directory(unsigned int _sectors, unsigned int _cluster,
                     unsigned int _bps, unsigned int parentCluster)
    : cluster(_cluster), sectors(_sectors) {
  int totalBytes = _bps * sectors;
  size = totalBytes / sizeof(DirectoryEntry);
  entries = new DirectoryEntry[size];

  DirectoryEntry self(".          ", cluster, DIRECTORY);
  DirectoryEntry parent("..         ", parentCluster, DIRECTORY);

  entries[0] = self;
  entries[1] = parent;
}

Directory::~Directory() { delete[] entries; }

DirectoryEntry *Directory::findEntry(const char *entryName) {
  for (unsigned int i = 0; i < size; i++) {
    if (std::memcmp(entryName, entries[i].name, 11) == 0) {
      // printf("%s == %s\n", filename, currentDirectory[i].name);
      return &entries[i];
    }
  }
  return NULL;
}

DirectoryEntry *Directory::createEntry(const char *name, unsigned int cluster) {
  DirectoryEntry *newDir = new DirectoryEntry(name, cluster, DIRECTORY);

  for (int i = 0; i < size; i++) {
    if (!entries[i].isValid()) {
      entries[i] = *newDir;
      return newDir;
    }
  }

  return NULL;
}
