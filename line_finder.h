#ifndef LINE_FINDER_H__
#define LINE_FINDER_H__

#include <cmath>
#include <string>
#include <vector>

#include "loader.h"

class MaxSetList;

class SetOfRegions
{
public:
  std::vector<unsigned> ids;

  static SetOfRegions
  create_from_crossing_counts (const std::vector<unsigned> &crossings);
};

void
find_lines (MaxSetList *lines, const std::vector<RegionData> &regs);

#endif
