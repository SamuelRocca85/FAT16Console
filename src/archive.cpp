#include "Archive.h"
#include "Byte.h"
#include "Fat.h"
#include <cstdio>

Archive::Archive(byte startCluster[2], byte fileSize[4], Disk &disk, Fat &fat,
                 unsigned int rootDirEnd)
    : data(NULL) {
  unsigned int currentCluster = bytes16ToInt(startCluster);
  unsigned int size = bytes32ToInt(fileSize);
  unsigned int bytesPerCluster =
      bytes16ToInt(disk.bs.bytesPerSector) * disk.bs.sectorsPerCluster;

  unsigned int clusterCount = size / bytesPerCluster;

  if (size % bytesPerCluster) {
    clusterCount++;
  }

  byte *firstClusterData = new byte[bytesPerCluster];
  for (int i = 0; i < clusterCount; i++) {
    unsigned int firstSector =
        (currentCluster - 2) * disk.bs.sectorsPerCluster + rootDirEnd;

    disk.readSectors(firstSector, disk.bs.sectorsPerCluster, firstClusterData);

    for (int j = 0; j < bytesPerCluster; j++) {
      if (std::isprint(firstClusterData[j])) {
        printf("%c", firstClusterData[j]);
      }
      unsigned int totalBytesRead = (j + (i * bytesPerCluster));
      if (totalBytesRead == size - 1) {
        break;
      }
    }
    currentCluster = fat.get(currentCluster);
  }

  printf("\n");
  delete[] firstClusterData;
}
