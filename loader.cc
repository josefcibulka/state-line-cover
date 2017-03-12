#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // transform
#include <cctype> // tolower
#include "kml/base/file.h"
#include "kml/engine.h"

#include "loader.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

Loader::Loader(LoaderVisitorInterface *visitor)
      : visitor_(visitor)
{
}

bool Loader::load_borders(string filename)
{
   using std::cerr;
   using std::endl;
   if (filename.length() < 4)
     {
      cerr << "Error: Filename " << filename
          << " does not have a correct suffix, \"kml\" or \"csv\" is required."
          << endl;
      return false;
     }
   string suffix = filename.substr(filename.length() - 3);
   if (suffix == "csv")
      load_borders_csv(filename);
   else if (suffix == "kml")
      load_borders_kml(filename);
   else
     {
      cerr << "Error: Filename " << filename
          << " does not have a correct suffix, \"kml\" or \"csv\" is required."
          << endl;
       return false;
     }
   return true;
}

bool Loader::load_borders_kml(string filename)
{
   using kmlengine::KmlFile;
   using kmlengine::KmlFilePtr;

   std::ifstream input_file(filename);
   std::stringstream buffer;
   buffer << input_file.rdbuf();
   string kml_str = buffer.str();
   if (kml_str.length() < 100)
   {
      cerr << "wrong kml: " << kml_str << endl;
      return false;
   }
   string errors;
   KmlFilePtr kml_file = KmlFile::CreateFromParse(kml_str, &errors);
   if (!kml_file || !errors.empty())
   {
      cerr << "parse failed: " << errors << endl;
      return false;
   }

   const kmldom::ElementPtr root_el = kml_file->get_root();
   const kmldom::KmlPtr kml_el = kmldom::AsKml(root_el);
   if (!kml_el)
   {
      cerr << "Not a kml: " << root_el->get_char_data() << endl;
      return false;
   }
   const kmldom::DocumentPtr docu_el = kmldom::AsDocument(
         kml_el->get_feature());
   if (!docu_el)
   {
      cerr << "Not a Document: " << kml_el->get_char_data() << endl;
      return false;
   }
   for (unsigned i = 0; i < docu_el->get_feature_array_size(); i++)
   {
      const kmldom::FeaturePtr feat_el = docu_el->get_feature_array_at(i);
      const kmldom::FolderPtr fol_el = kmldom::AsFolder(feat_el);
      if (fol_el)
      {
         string reg_name;
         if (fol_el->get_feature_array_size() == 1)
            reg_name = fol_el->get_name();
         for (unsigned j = 0; j < fol_el->get_feature_array_size(); j++)
         {
            kmldom::FeaturePtr feat_el = fol_el->get_feature_array_at(j);
            bool success = process_placemark(feat_el, reg_name);
            if (!success)
               return false;
         }

      }
      else
      {
         const kmldom::PlacemarkPtr pm_el = kmldom::AsPlacemark(feat_el);
         if (pm_el)
            process_placemark(feat_el, "");
         else
         {
            cerr << "Neither a Folder nor a Placemark: "
                 << feat_el->get_char_data() << endl;
            return false;
         }
      }
   }
   return true;
}

bool Loader::load_borders_csv(string filename)
{
   using kmlengine::KmlFile;
   using kmlengine::KmlFilePtr;

   std::ifstream csv_file(filename);

   int col_cnt = 0;
   int where_is_name = -1;
   int where_is_geo = -1;

   while (true)
   {
      string area_str;
      bool success = bool(std::getline(csv_file, area_str));
      if (!success)
         break;
      std::istringstream istr(area_str);
      if (col_cnt == 0)
      {
         string cur_col;
         while (std::getline(istr, cur_col, ','))
         {
            std::transform(cur_col.begin(), cur_col.end(), cur_col.begin(),
                           ::tolower);
            if (cur_col.length() >= 3 && cur_col.substr(0, 3) == "geo")
               where_is_geo = col_cnt;
            if (cur_col.length() >= 4 && cur_col.substr(0, 4) == "name")
               where_is_name = col_cnt;
            col_cnt++;
         }
         if (col_cnt <= 0 || where_is_name < 0 || where_is_geo != col_cnt - 1)
         {
            cerr << "bad columns (must have both name and geo and geo must be the very last)"
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
         bool success = bool(std::getline(istr, cur_line, ','));
         if (!success)
         {
            cerr << "wrong line" << area_str << endl;
            return false;
         }
         if (i == where_is_name)
            reg_id = cur_line;
      }
      success = bool(std::getline(istr, kml_str_quoted));
      if (!success || kml_str_quoted.length() <= 1 || kml_str_quoted[0] != '"'
            || kml_str_quoted[kml_str_quoted.length() - 1] != '"')
      {
         cerr << "wrong kml " << kml_str_quoted << endl;
         return false;
      }
      string kml_str = kml_str_quoted.substr(1, kml_str_quoted.length() - 2);
      string errors;
      KmlFilePtr kml_file = KmlFile::CreateFromParse(kml_str, &errors);
      if (!kml_file || !errors.empty())
      {
         cerr << "parse failed: " << errors << endl;
         return false;
      }
      const kmldom::ElementPtr root = kml_file->get_root();
      bool success2 = fill_region_data(reg_id, root);
      if (!success2)
         return false;
   }
   return true;
}

bool Loader::process_placemark(const kmldom::ElementPtr &elem, string reg_name)
{
   const kmldom::PlacemarkPtr pm_el = kmldom::AsPlacemark(elem);
   if (!pm_el)
   {
      cerr << "Not a Placemark: " << elem->get_char_data() << endl;
      return false;
   }
   if(reg_name == "")
     reg_name = pm_el->get_name();
   bool success = fill_region_data(reg_name, pm_el->get_geometry());
   return success;
}

bool Loader::fill_region_data(string &reg_name,
                              const kmldom::ElementPtr &root_el)
{
   const kmldom::MultiGeometryPtr geom = kmldom::AsMultiGeometry(root_el);
   bool success = visitor_->visit_multi_geometry(reg_name);
   if (!success)
      return false;
   if (geom)
   {
      unsigned geom_size = geom->get_geometry_array_size();
      for (unsigned i = 0; i < geom_size; i++)
      {
         bool success = fill_polygon(geom->get_geometry_array_at(i));
         if (!success)
            return false;
      }
   }
   else
   {
      bool success = fill_polygon(root_el);
      if (!success)
         return false;
   }
   success = visitor_->finish_multi_geometry();
   return success;
}

bool Loader::fill_polygon(const kmldom::ElementPtr &polygon_el)
{
   const kmldom::PolygonPtr polygon = kmldom::AsPolygon(polygon_el);
   if (!polygon)
   {
      bool success = fill_polygon_from_line_string(polygon_el);
      if (!success)
         cerr << "Not a Polygon: " << polygon_el->get_char_data() << endl;
      return success;
   }
   if (!polygon->has_outerboundaryis())
   {
      cerr << "Not a Polygon with an outer boundary: "
           << polygon->get_char_data() << endl;
      return false;
   }
   const kmldom::OuterBoundaryIsPtr &boundary = polygon->get_outerboundaryis();
   if (!boundary->has_linearring())
   {
      cerr << "Not a Boundary with a linear ring: " << boundary->get_char_data()
           << endl;
      return false;
   }
   const kmldom::LinearRingPtr &ring = boundary->get_linearring();
   if (!ring->has_coordinates())
   {
      cerr << "Not a Linear Ring with Coordinates: " << ring->get_char_data()
           << endl;
      return false;
   }
   const kmldom::CoordinatesPtr &coord_list = ring->get_coordinates();

   bool success = process_coord_list(coord_list, true);
   return success;
}

bool Loader::fill_polygon_from_line_string(const kmldom::ElementPtr &line_el)
{
   const kmldom::LineStringPtr line_str = kmldom::AsLineString(line_el);
   if (!line_str)
   {
      cerr << "Not a Line String: " << line_el->get_char_data() << endl;
      return false;
   }
   if (!line_str->has_coordinates())
   {
      cerr << "Not a Line String with Coordinates: "
           << line_str->get_char_data() << endl;
      return false;
   }
   const kmldom::CoordinatesPtr &coord_list = line_str->get_coordinates();

   bool success = process_coord_list(coord_list, false);
   return success;
}

bool Loader::process_coord_list(const kmldom::CoordinatesPtr &coord_list,
                                bool is_ring)
{
   bool success = visitor_->visit_polygon();
   if (!success)
      return false;
   unsigned coord_cnt = coord_list->get_coordinates_array_size();
   double first_lon, first_lat;
   for (unsigned i = 0; i < coord_cnt; i++)
   {
      kmlbase::Vec3 coords = coord_list->get_coordinates_array_at(i);
      double lon = coords.get_longitude();
      double lat = coords.get_latitude();
      if (i == 0)
      {
         first_lon = lon;
         first_lat = lat;
      }
      bool success;
      if (is_ring || i < coord_cnt - 1)
      {
         success = visitor_->visit_single_point(lon, lat);
      }
      else
      {
         success = (lon == first_lon && lat == first_lat);
         if (!success)
            cerr << "Error: First and last point in a linear ring are different."
                 << endl;
      }
      if (!success)
         return false;
   }
   success = visitor_->finish_polygon();
   return success;
}
