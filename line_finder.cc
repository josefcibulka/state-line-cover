#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

#include "line_finder.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

vector<unsigned>
get_vertical_line_crossing_cnt (double x, const std::vector<RegionData> &regs)
{
  vector<unsigned> line;
  for (const RegionData &reg : regs)
    {
      unsigned crosses = 0;
      for (const vector<Point> &bdry : reg.boundaries)
        {
          assert(bdry.size () > 0);
          Point prev = *(bdry.end () - 1);
          for (const Point &cur : bdry)
            {
              if ((prev.x <= x && cur.x >= x) || (prev.x >= x && cur.x <= x))
                crosses++;
              prev = cur;
            }
        }
      line.push_back (crosses);
    }
  assert(line.size () == regs.size ());
  cout << "x: " << x << " Regions: ";
  for (unsigned i = 0; i < line.size (); i++)
    {
      if (line[i] > 0)
        cout << regs[i].id << ": " << line[i] << "x  ";
    }
  cout << endl;
  return std::move (line);
}

void
find_lines (vector<SetOfRegions> *lines, const vector<RegionData> &regs)
{
  for (const RegionData &reg : regs)
    {
      cout << "Starting " << reg.id << endl;
      for (const vector<Point> &bdry : reg.boundaries)
        {
          assert(bdry.size () > 0);
          for (const Point &center : bdry)
            {
              get_vertical_line_crossing_cnt (center.x, regs);
            }
        }
    }
}
