#ifndef LINE_FINDER_H__
#define LINE_FINDER_H__

#include <cmath>
#include <string>
#include <vector>

#include "loader.h"
#include "region_data.h"

class MaxSetList;

class SetOfRegions
{
public:
  std::vector<unsigned> ids;

  static SetOfRegions
  create_from_crossing_counts (const std::vector<unsigned> &crossings,
                               const Point &p1_in, const Point &p2_in);

  SetOfRegions (const Point &p1_in, const Point &p2_in);

  Point p1, p2;

private:
};

void
find_lines (MaxSetList *lines, const std::vector<RegionData> &regs);

#endif
