#include <iostream>
#include <sstream>
#include <fstream>
#include "kml/base/file.h"
#include "kml/dom.h"
#include "kml/engine.h"

#include "loader.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

class Point
{
public:
  Point (double x_in, double y_in) :
      x (x_in), y (y_in)
  {
  }
  double x, y;
};

class RegionData
{
public:
  string id;
  vector<vector<Point> > boundaries;
};

bool
fill_polygon (vector<Point> *pts, const kmldom::ElementPtr &polygon_el)
{
  pts->clear ();
  const kmldom::PolygonPtr polygon = kmldom::AsPolygon (polygon_el);
  if (!polygon)
    {
      cerr << "Not a Polygon: " << polygon_el->get_char_data () << endl;
      return false;
    }
  if (!polygon->has_outerboundaryis ())
    {
      cerr << "Not a Polygon with an outer boundary: "
          << polygon->get_char_data () << endl;
      return false;
    }
  const kmldom::OuterBoundaryIsPtr &boundary = polygon->get_outerboundaryis ();
  if (!boundary->has_linearring ())
    {
      cerr << "Not a Boundary with a linear ring: "
          << boundary->get_char_data () << endl;
      return false;
    }
  const kmldom::LinearRingPtr &ring = boundary->get_linearring ();
  if (!ring->has_coordinates ())
    {
      cerr << "Not a Linear Ring with Coordinates: " << ring->get_char_data ()
          << endl;
      return false;
    }
  const kmldom::CoordinatesPtr &coord_list = ring->get_coordinates ();

  unsigned coord_cnt = coord_list->get_coordinates_array_size ();
  for (unsigned i = 0; i < coord_cnt; i++)
    {
      kmlbase::Vec3 coords = coord_list->get_coordinates_array_at (i);
      if (coords.has_altitude () && coords.get_altitude () != 0)
        {
          cerr << "Nonzero altitude: " << coords.get_altitude () << endl;
          return false;
        }
      pts->push_back (Point (coords.get_longitude (), coords.get_latitude ()));
    }
  return true;
}

bool
load_borders (string filename)
{
  using kmlengine::KmlFile;
  using kmlengine::KmlFilePtr;

  std::ifstream csv_file (filename);

  vector<RegionData> regs;

  while (true)
    {
      string area_str;
      bool success = std::getline (csv_file, area_str);
      if (!success)
        break;
      if (regs.size () == 0 && area_str == "Name,Code,Geometry")
        continue;
      std::istringstream istr (area_str);
      string reg_id;
      for (int i = 0; i < 2; i++)
        {
          bool success = std::getline (istr, reg_id, ',');
          if (!success)
            {
              cerr << "wrong line" << area_str << endl;
              return false;
            }
        }
      string kml_str_quoted;
      success = std::getline (istr, kml_str_quoted);
      if (!success || kml_str_quoted.length () <= 1 || kml_str_quoted[0] != '"'
          || kml_str_quoted[kml_str_quoted.length () - 1] != '"')
        {
          cerr << "wrong kml " << kml_str_quoted << endl;
          return false;
        }
      string kml_str = kml_str_quoted.substr (1, kml_str_quoted.length () - 2);
      string errors;
      KmlFilePtr kml_file = KmlFile::CreateFromParse (kml_str, &errors);
      if (!kml_file || !errors.empty ())
        {
          cerr << "parse failed: " << errors << endl;
          return false;
        }
      kmldom::ElementPtr root = kml_file->get_root ();
      const kmldom::MultiGeometryPtr geom = kmldom::AsMultiGeometry (root);
      if (geom)
        {
          unsigned geom_size = geom->get_geometry_array_size ();
          cout << "MultiGeometry has " << geom_size << " polygons " << endl;
          for (unsigned i = 0; i < geom_size; i++)
            {

            }
        }
      else
        {
          vector<Point> new_vec;
          fill_polygon (&new_vec, root);
          cerr << "Polygon with : " << new_vec.size() << " vertices " << endl;
        }

    }
  cout << "Read data for " << regs.size () << " areas" << endl;
  return true;
}
