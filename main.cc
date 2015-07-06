#include <iostream>

#include "loader.h"
#include "line_finder.h"

using std::vector;

int main(void)
{
  vector<RegionData> regs;
  load_borders(&regs, "data/KML-US_states.csv");
  vector<SetOfRegions> lines;
  find_lines(&lines, regs);
  return 0;
}
