#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
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

class RegionLine
{
public:
  virtual
  ~RegionLine ()
  {
  }
  virtual void
  add_geometry (const kmldom::PolygonPtr polygon) = 0;
};

class RegionLineCsv : public RegionLine
{
public:
  vector<string> info_list;
  string name;
  kmldom::MultiGeometryPtr geom;

  RegionLineCsv (vector<string> info_list_in, string name_in) :
      info_list (info_list_in), name (name_in)
  {
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory ();
    geom = factory->CreateMultiGeometry ();
  }

  void
  add_geometry (const kmldom::PolygonPtr polygon)
  {
    geom->add_geometry (polygon);
  }

  void
  print (std::ostream & ostr)
  {
    for (string info : info_list)
      ostr << info << ";";
    ostr << "\"" << kmldom::SerializeRaw (geom) << "\"" << endl;
  }
};

class RegionLineKml : public RegionLine
{
public:
  const kmldom::ExtendedDataPtr ext_data;
  string name;
  kmldom::MultiGeometryPtr geom;

  RegionLineKml (const kmldom::ExtendedDataPtr ext_data_in, string name_in) :
      name (name_in)
  {
    const kmldom::ExtendedDataPtr ext_data = kmldom::AsExtendedData (
        kmlengine::Clone (ext_data_in));
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory ();

    geom = factory->CreateMultiGeometry ();
  }

  void
  add_geometry (const kmldom::PolygonPtr polygon)
  {
    geom->add_geometry (polygon);
  }
};

class RegionLineMapCsv
{
public:
  std::map<string, RegionLineCsv> reg_map;

  RegionLineCsv *
  get_geom_or_create (string reg_id, vector<string> reg_data)
  {
    if (reg_map.find (reg_id) == reg_map.end ())
      reg_map.insert (
          std::make_pair (reg_id, RegionLineCsv (reg_data, reg_id)));
    RegionLineCsv *reg_line = &(reg_map.find (reg_id)->second);
    return reg_line;
  }

  void
  print_big_kml (std::ostream &ostr)
  {
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory ();
    kmldom::DocumentPtr docu = factory->CreateDocument ();
    for (auto reg_pair : reg_map)
      {
        kmldom::PlacemarkPtr placemark = factory->CreatePlacemark ();
        const auto &reg = reg_pair.second;
        placemark->set_name (reg.name);
        placemark->set_geometry (reg.geom);
        docu->add_feature (placemark);
      }
    kmldom::KmlPtr kml = factory->CreateKml ();
    kml->set_feature (docu);
    ostr << kmldom::SerializeRaw (kml) << endl;
  }
};

class RegionLineMapKml
{
public:
  std::map<string, RegionLineKml> reg_map;

  RegionLineKml *
  get_geom_or_create (string reg_id, const kmldom::ExtendedDataPtr &ext_data)
  {
    if (reg_map.find (reg_id) == reg_map.end ())
      reg_map.insert (
          std::make_pair (reg_id, RegionLineKml (ext_data, reg_id)));
    RegionLineKml *reg_line = &(reg_map.find (reg_id)->second);
    return reg_line;
  }

  void
  print_big_kml (std::ostream &ostr)
  {
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory ();
    kmldom::DocumentPtr docu = factory->CreateDocument ();
    for (auto reg_pair : reg_map)
      {
        kmldom::PlacemarkPtr placemark = factory->CreatePlacemark ();
        const auto &reg = reg_pair.second;
        placemark->set_name (reg.name);
        placemark->set_extendeddata (reg.ext_data);
        placemark->set_geometry (reg.geom);
        docu->add_feature (placemark);
      }
    kmldom::KmlPtr kml = factory->CreateKml ();
    kml->set_feature (docu);
    ostr << kmldom::SerializeRaw (kml) << endl;
  }
};

bool
fill_polygon (RegionLine *out_geom, const kmldom::ElementPtr &polygon_el)
{
  const kmldom::PolygonPtr polygon = kmldom::AsPolygon (
      kmlengine::Clone (polygon_el));
  if (!polygon)
    {
      cerr << "Not a Polygon: " << polygon_el->get_char_data () << endl;
      return false;
    }
  out_geom->add_geometry (polygon);
  return true;
}

bool
fill_region_data (RegionLine *out_geom, const kmldom::ElementPtr &root_el)
{
  const kmldom::MultiGeometryPtr geom = kmldom::AsMultiGeometry (root_el);
  if (geom)
    {
      unsigned geom_size = geom->get_geometry_array_size ();
      for (unsigned i = 0; i < geom_size; i++)
        {
          bool success = fill_polygon (out_geom,
                                       geom->get_geometry_array_at (i));
          if (!success)
            return false;
        }
    }
  else
    {
      bool success = fill_polygon (out_geom, root_el);
      if (!success)
        return false;
    }
  return true;
}

bool
load_borders_csv (RegionLineMapCsv *reg_map, string filename)
{
  using kmlengine::KmlFile;
  using kmlengine::KmlFilePtr;

  std::ifstream csv_file (filename);

  int col_cnt = 0;
  int where_is_name = -1;
  int where_is_geo = -1;

  for (unsigned lines_read = 0;; lines_read++)
    {
      string area_str;
      bool success = bool(std::getline (csv_file, area_str));
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
                  << "bad columns (must have both name and geo and geo must be the very last): "
                  << area_str << endl << "col_cnt: " << col_cnt
                  << " where_is_name: " << where_is_name << " where_is_geo: "
                  << where_is_geo << endl;
              return false;
            }
          // cout << area_str << endl;
          continue;
        }
      string reg_id;
      vector<string> reg_data;
      for (int i = 0; i < where_is_geo; i++)
        {
          string cur_column;
          bool success = bool(std::getline (istr, cur_column, ','));
          if (!success)
            {
              cerr << "wrong line: " << area_str << endl;
              return false;
            }
          reg_data.push_back (cur_column);
          if (i == where_is_name)
            reg_id = cur_column;
        }
      string kml_str_quoted;
      success = bool(std::getline (istr, kml_str_quoted));
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

      RegionLine *reg_line = reg_map->get_geom_or_create (reg_id, reg_data);
      const kmldom::ElementPtr root = kml_file->get_root ();
      success = fill_region_data (reg_line, root);
      if (!success)
        return false;
    }
  return true;
}

string
get_file_contents (string filename)
{
  std::ifstream input_file (filename);
  std::stringstream buffer;
  buffer << input_file.rdbuf ();
  return buffer.str ();
}

bool
process_placemark (RegionLineMapKml *reg_map, const kmldom::ElementPtr &elem,
                   string reg_name)
{
  const kmldom::PlacemarkPtr pm_el = kmldom::AsPlacemark (elem);
  if (!pm_el)
    {
      cerr << "Not a Placemark: " << elem->get_char_data () << endl;
      return false;
    }

  RegionData new_reg;
  if (reg_name == "")
    reg_name = pm_el->get_name ();
  new_reg.name = reg_name;
  const kmldom::ExtendedDataPtr ext_data = pm_el->get_extendeddata ();
  RegionLine *reg_line = reg_map->get_geom_or_create (reg_name, ext_data);
  bool success = fill_region_data (reg_line, pm_el->get_geometry ());
  return success;
}

bool
load_borders_kml (RegionLineMapKml *reg_map, string filename)
{
  using kmlengine::KmlFile;
  using kmlengine::KmlFilePtr;

  string kml_str = get_file_contents (filename);
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

  cerr << "parse finished: " << errors << endl;

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
      const kmldom::FeaturePtr feat_el = docu_el->get_feature_array_at (i);
      const kmldom::FolderPtr fol_el = kmldom::AsFolder (feat_el);
      if (fol_el)
        {
          string reg_name;
          if (fol_el->get_feature_array_size () == 1)
            reg_name = fol_el->get_name ();
          for (unsigned j = 0; j < fol_el->get_feature_array_size (); j++)
            {
              kmldom::FeaturePtr feat_el = fol_el->get_feature_array_at (j);
              bool success = process_placemark (reg_map, feat_el, reg_name);
              if (!success)
                return false;
            }

        }
      else
        {
          const kmldom::PlacemarkPtr pm_el = kmldom::AsPlacemark (feat_el);
          if (pm_el)
            process_placemark (reg_map, feat_el, "");
          else
            {
              cerr << "Neither a Folder nor a Placemark: "
                  << feat_el->get_char_data () << endl;
              return false;
            }
        }
    }
  return true;
}

bool
load_borders (string filename, std::ostream &ostr)
{
  if (filename.length () < 4)
    return false;
  string suffix = filename.substr (filename.length () - 3);
  if (suffix == "csv")
    {
      RegionLineMapCsv reg_map;
      load_borders_csv (&reg_map, filename);
      cerr << "Read data for " << reg_map.reg_map.size () << " regions" << endl;
      reg_map.print_big_kml (ostr);
    }
  else if (suffix == "kml")
    {
      RegionLineMapKml reg_map;
      load_borders_kml (&reg_map, filename);
      cerr << "Read data for " << reg_map.reg_map.size () << " regions" << endl;
      reg_map.print_big_kml (ostr);
    }
  else
    return false;

  return true;
}

void
print_help ()
{
  cerr << "\n"
      << " This program combines placemarks with identical name into a single placemark with multigeometry."
      << endl;
  cerr
      << " This is useful for the HIU data (https://hiu.state.gov/data/data.aspx) "
          "where, for example, Finland has more than ten thousand placemarks "
          "(every island has its own placemark).\n" << endl;
  cerr << " Exactly two arguments are required:\n"
      "1) The input kml or csv file.\n"
      "2) Output kml file." << endl;
}

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      print_help ();
      return 0;
    }
  std::ofstream ostr;
  ostr.open (argv[2]);
  if (!ostr.good ())
    {
      cerr << "Failed to open output file." << endl;
      return 0;
    }
  load_borders (string (argv[1]), ostr);

  return 0;
}
