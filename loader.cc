#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // transform
#include <cctype> // tolower
#include "kml/base/file.h"
#include "kml/dom.h"
#include "kml/engine.h"

#include "loader.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

bool
fill_polygon_from_line_string (vector<Point> *pts,
                               const kmldom::ElementPtr &line_el)
{
  pts->clear ();
  const kmldom::LineStringPtr line_str = kmldom::AsLineString (line_el);
  if (!line_str)
    {
      cerr << "Not a Line String: " << line_el->get_char_data () << endl;
      return false;
    }
  if (!line_str->has_coordinates ())
    {
      cerr << "Not a Line String with Coordinates: "
          << line_str->get_char_data () << endl;
      return false;
    }
  const kmldom::CoordinatesPtr &coord_list = line_str->get_coordinates ();

  unsigned coord_cnt = coord_list->get_coordinates_array_size ();
  for (unsigned i = 0; i < coord_cnt; i++)
    {
      kmlbase::Vec3 coords = coord_list->get_coordinates_array_at (i);
      double lon = coords.get_longitude ();
      double lat = coords.get_latitude ();
      Point new_pt = Point::create_gnomonic_from_spherical (lon, lat);
      if (i < coord_cnt - 1)
        pts->push_back (new_pt);
      else
        assert(new_pt.x == (*pts)[0].x && new_pt.y == (*pts)[0].y);
      // cout << lon << ", " << lat << " == " << new_pt.x << ", " << new_pt.y << endl;
    }
  return true;
}

bool
fill_polygon (vector<Point> *pts, const kmldom::ElementPtr &polygon_el)
{
  pts->clear ();
  const kmldom::PolygonPtr polygon = kmldom::AsPolygon (polygon_el);
  if (!polygon)
    {
      bool success = fill_polygon_from_line_string (pts, polygon_el);
      if (!success)
        cerr << "Not a Polygon: " << polygon_el->get_char_data () << endl;
      return success;
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
      /*if (coords.has_altitude () && coords.get_altitude () != 0)
       {
       cerr << "Nonzero altitude: " << coords.get_altitude () << endl;
       return false;
       }*/
      double lon = coords.get_longitude ();
      double lat = coords.get_latitude ();
      Point new_pt = Point::create_gnomonic_from_spherical (lon, lat);
      pts->push_back (new_pt);
      // cout << lon << ", " << lat << " == " << new_pt.x << ", " << new_pt.y << endl;
    }
  return true;
}

bool
fill_region_data (RegionData *reg_data, const kmldom::ElementPtr &root_el)
{
  const kmldom::MultiGeometryPtr geom = kmldom::AsMultiGeometry (root_el);
  if (geom)
    {
      unsigned geom_size = geom->get_geometry_array_size ();
      unsigned ver_total = 0;
      for (unsigned i = 0; i < geom_size; i++)
        {
          reg_data->boundaries.push_back (vector<Point> ());
          vector<Point> &new_vec = *(reg_data->boundaries.end () - 1);
          bool success = fill_polygon (&new_vec,
                                       geom->get_geometry_array_at (i));
          if (!success)
            return false;
          ver_total += new_vec.size ();
        }
      cout << reg_data->name << " has " << geom_size
          << " polygons with the total number of " << ver_total << " vertices "
          << endl;
    }
  else
    {
      reg_data->boundaries.push_back (vector<Point> ());
      vector<Point> &new_vec = *(reg_data->boundaries.end () - 1);
      bool success = fill_polygon (&new_vec, root_el);
      if (!success)
        return false;
      cout << reg_data->name << " has one polygon with " << new_vec.size ()
          << " vertices " << endl;
    }

  /* for(const vector<Point> &bdry : reg_data->boundaries)
   {
   for(const Point &pt: bdry)
   cout << pt.x << ", " << pt.y << "   ";
   }
   cout << endl;*/
  return true;
}

bool
load_borders_csv (vector<RegionData> *regs, string filename)
{
  using kmlengine::KmlFile;
  using kmlengine::KmlFilePtr;

  std::ifstream csv_file (filename);
  regs->clear ();

  int col_cnt = 0;
  int where_is_name = -1;
  int where_is_geo = -1;

  while (true)
    {
      string area_str;
      bool success = std::getline (csv_file, area_str);
      if (!success)
        break;
      std::istringstream istr (area_str);
      if (col_cnt == 0)
        {
          string cur_col;
          while (std::getline (istr, cur_col, ','))
            {
              std::transform (cur_col.begin (), cur_col.end (),
                              cur_col.begin (), ::tolower);
              if (cur_col.length () >= 3 && cur_col.substr (0, 3) == "geo")
                where_is_geo = col_cnt;
              if (cur_col.length () >= 4 && cur_col.substr (0, 4) == "name")
                where_is_name = col_cnt;
              col_cnt++;
            }
          if (col_cnt <= 0 || where_is_name < 0 || where_is_geo != col_cnt - 1)
            {
              cerr
                  << "bad columns (must have both name and geo and geo must be the very last)"
                  << area_str << endl;
              return false;
            }
          continue;
        }
      string reg_id;
      string kml_str_quoted;
      for (int i = 0; i < where_is_geo; i++)
        {
          string cur_line;
          bool success = std::getline (istr, cur_line, ',');
          if (!success)
            {
              cerr << "wrong line" << area_str << endl;
              return false;
            }
          if (i == where_is_name)
            reg_id = cur_line;
        }
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
      const kmldom::ElementPtr root = kml_file->get_root ();
      RegionData new_reg;
      new_reg.name = reg_id;
      bool success2 = fill_region_data (&new_reg, root);
      if (!success2)
        return false;
      regs->push_back (std::move (new_reg));
    }
  cout << "Read data for " << regs->size () << " areas" << endl;
  return true;
}

bool
load_borders_kml (vector<RegionData> *regs, string filename)
{
  using kmlengine::KmlFile;
  using kmlengine::KmlFilePtr;

  std::ifstream input_file (filename);
  std::stringstream buffer;
  buffer << input_file.rdbuf ();
  string kml_str = buffer.str ();
  if (kml_str.length () < 100)
    {
      cerr << "wrong kml: " << kml_str << endl;
      return false;
    }
  string errors;
  KmlFilePtr kml_file = KmlFile::CreateFromParse (kml_str, &errors);
  if (!kml_file || !errors.empty ())
    {
      cerr << "parse failed: " << errors << endl;
      return false;
    }

  regs->clear ();

  const kmldom::ElementPtr root_el = kml_file->get_root ();
  const kmldom::KmlPtr kml_el = kmldom::AsKml (root_el);
  if (!kml_el)
    {
      cerr << "Not a kml: " << root_el->get_char_data () << endl;
      return false;
    }
  const kmldom::DocumentPtr docu_el = kmldom::AsDocument (
      kml_el->get_feature ());
  if (!docu_el)
    {
      cerr << "Not a Document: " << kml_el->get_char_data () << endl;
      return false;
    }
  for (unsigned i = 0; i < docu_el->get_feature_array_size (); i++)
    {
      const kmldom::FolderPtr fol_el = kmldom::AsFolder (
          docu_el->get_feature_array_at (i));
      if (!fol_el)
        {
          cerr << "Not a Folder: "
              << docu_el->get_feature_array_at (i)->get_char_data () << endl;
          return false;
        }
      string reg_name;
      if (fol_el->get_feature_array_size () == 1)
        reg_name = fol_el->get_name ();
      for (unsigned j = 0; j < fol_el->get_feature_array_size (); j++)
        {
          const kmldom::PlacemarkPtr pm_el = kmldom::AsPlacemark (
              fol_el->get_feature_array_at (j));
          if (!pm_el)
            {
              cerr << "Not a Placemark: "
                  << fol_el->get_feature_array_at (j)->get_char_data () << endl;
              return false;
            }

          RegionData new_reg;
          if (fol_el->get_feature_array_size () > 1)
            reg_name = pm_el->get_name ();
          new_reg.name = reg_name;
          bool success = fill_region_data (&new_reg, pm_el->get_geometry ());
          if (!success)
            return false;
          regs->push_back (std::move (new_reg));
        }
    }
  cout << "Read data for " << regs->size () << " areas" << endl;
  return true;
}

bool
load_borders (vector<RegionData> *regs, string filename)
{
  if (filename.length () < 4)
    return false;
  string suffix = filename.substr (filename.length () - 3);
  if (suffix == "csv")
    load_borders_csv (regs, filename);
  else if (suffix == "kml")
    load_borders_kml (regs, filename);
  else
    return false;
  return true;
}
