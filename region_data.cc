#include <cstddef>
#include <algorithm>
#include <iostream>

#include "region_data.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

/// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
void
RegionData::reduce_to_convex_hulls ()
{
  std::size_t boundaries_size_orig = 0;
  for (const vector<Point> &bdry : boundaries)
    boundaries_size_orig += bdry.size ();
  vector<vector<Point> > new_boundaries;
  for (vector<Point> &bdry : boundaries)
    {
      int n = bdry.size ();
      vector<Point> hull (2 * n, Point (0, 0));

      // Sort points lexicographically
      std::sort (bdry.begin (), bdry.end ());

      int k = 0;
      // Build lower hull
      for (const Point &p : bdry)
        {
          while (k >= 2
              && Point::cross_product (hull[k - 1] - hull[k - 2],
                                       p - hull[k - 1]) <= 0)
            k--;
          hull[k++] = p;
        }

      // Build upper hull
      for (int i = n - 2, t = k + 1; i >= 0; i--)
        {
          Point p = bdry[i];
          while (k >= t
              && Point::cross_product (hull[k - 1] - hull[k - 2],
                                       p - hull[k - 2]) <= 0)
            k--;
          hull[k++] = p;
        }

      hull.resize (k, Point (0, 0));
      /*for(const Point &p:H)
       cout << p.x <<", " << p.y << endl;*/
      new_boundaries.push_back (std::move (hull));
    }
  boundaries = std::move (new_boundaries);
  std::size_t boundaries_size = 0;
  for (const vector<Point> &bdry : boundaries)
    boundaries_size += bdry.size ();
  cout << "After the reduction to convex hulls, " << name << " kept "
      << boundaries_size << " out of the " << boundaries_size_orig
      << " original vertices in " << boundaries.size () << " polygons." << endl;
}
