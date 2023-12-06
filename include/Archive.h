#pragma once
#include "Byte.h"
#include "Disk.h"
#include "Fat.h"

class Archive {
private:
  byte *data;

public:
  Archive(byte startCluster[2], byte fileSize[4], Disk &disk, Fat &fat,
          unsigned int rootDirEnd);
};
