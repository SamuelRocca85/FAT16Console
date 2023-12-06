#include "Archive.h"
#include "Byte.h"
#include "Fat.h"
#include <cstdio>

Archive::Archive(byte startCluster[2], byte fileSize[4], Disk &disk, Fat &fat,
                 unsigned int rootDirEnd)
    : data(NULL) {
  unsigned int currentCluster = bytes16ToInt(startCluster);

  unsigned int bytesPerCluster =
      bytes16ToInt(disk.bs.bytesPerSector) * disk.bs.sectorsPerCluster;

  byte *firstClusterData = new byte[bytesPerCluster];

  unsigned int firstSector =
      (currentCluster - 2) * disk.bs.sectorsPerCluster + rootDirEnd;

  disk.readSectors(firstSector, disk.bs.sectorsPerCluster, firstClusterData);

  for (int i = 0; i < bytes32ToInt(fileSize); i++) {
    if (std::isprint(firstClusterData[i])) {
      printf("%c", firstClusterData[i]);
    }
  }
  printf("\n");
  // unsigned short clusterValue = fat.get(currentCluster);
  delete[] firstClusterData;
}
