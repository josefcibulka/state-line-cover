#ifndef LINE_FINDER_H__
#define LINE_FINDER_H__

#include <cmath>
#include <string>
#include <vector>

#include "loader.h"
#include "region_data.h"

class MaxSetList;

struct SphericalPoint
{
public:
   SphericalPoint (double lon_in, double lat_in) :
       lon (lon_in), lat (lat_in)
   {
   }
   SphericalPoint (const Point &pt) :
       lon (pt.lon), lat (pt.lat)
   {
      assert(pt.has_lon_lat);
   }
   double lon, lat;
};

class SetOfRegions
{
public:
   std::vector<unsigned> ids;

   static SetOfRegions
   create_from_crossing_counts(const std::vector<unsigned> &crossings,
                               const Point &p1, const Point &p2);

   SetOfRegions(const std::vector<SphericalPoint> &geodesic_in);

   std::vector<SphericalPoint> geodesic;

private:
};

void
find_lines(MaxSetList *lines, const std::vector<RegionData> &regs);

#endif
