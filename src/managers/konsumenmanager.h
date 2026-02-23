#pragma once

#include "basemanager.h"

class KonsumenManager : public BaseManager
{
  public:
    KonsumenManager() : BaseManager("konsumen", false) {}
    ~KonsumenManager() {}
};
