#pragma once
#include "Byte.h"
#include "Disk.h"

class Archive {
private:
public:
  Archive(Disk &disk, byte startCluster[2]);
};
