#ifndef LINE_FINDER_H__
#define LINE_FINDER_H__

#include <cmath>
#include <string>
#include <vector>

#include "loader.h"

class SetOfRegions
{
public:
  std::vector<int> regs;
};

void
find_lines (std::vector<SetOfRegions> *lines,
            const std::vector<RegionData> &regs);

#endif
