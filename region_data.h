#ifndef REGION_DATA_H__
#define REGION_DATA_H__

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

  static double
  cross_product (const Point &p, const Point &q)
  {
    return p.x * q.y - p.y * q.x;
  }

  double x, y;

  Point (double x_in, double y_in) :
      x (x_in), y (y_in)
  {
  }

  bool
  operator < (const Point &p) const
  {
    return x < p.x || (x == p.x && y < p.y);
  }

  Point
  operator - (const Point &p) const
  {
    return Point (x - p.x, y - p.y);
  }

};

class RegionData
{
public:
  std::string name;
  std::vector<std::vector<Point> > boundaries;

  void
  reduce_to_convex_hulls ();
};

#endif
