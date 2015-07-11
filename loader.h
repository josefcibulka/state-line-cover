#ifndef LOADER_H__
#define LOADER_H__

#include <cmath>
#include <string>
#include <vector>

#include "region_data.h"

bool
load_borders_csv (std::vector<RegionData> *regs, std::string filename);

bool
load_borders_kml (std::vector<RegionData> *regs, std::string filename);

bool
load_borders (std::vector<RegionData> *regs, std::string filename);

#endif
