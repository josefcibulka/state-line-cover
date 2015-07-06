#ifndef LOADER_H__
#define LOADER_H__

#include <cmath>
#include <string>
#include <vector>

class Point
{
public:
  static Point
  create_gnomonic_from_spherical (double lon, double lat)
  {
    double colat_rad = M_PI * (90. - lat) / 180.;
    double lon_rad = M_PI * lon / 180.;
    double tancl = tan (colat_rad);
    double x = tancl * sin (lon_rad);
    double y = tancl * cos (lon_rad);
    return Point (x, y);
  }

  Point (double x_in, double y_in) :
      x (x_in), y (y_in)
  {
  }
  double x, y;
};

struct RegionData
{
public:
  std::string id;
  std::vector<std::vector<Point> > boundaries;
};

bool
load_borders (std::vector<RegionData> *regs, std::string filename);

#endif
