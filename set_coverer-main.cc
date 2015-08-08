#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include "max_set_list.h"

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

class Results
{
public:
  vector<std::pair<string, string>> best_sets;

  void
  add_set (const MaxSetList &max_set_list, const vector<bool> reg_used,
           const vector<const SetOfRegions *> lines_used)
  {
    bool first_output = true;
    std::ostringstream ostr_sets;
    ostr_sets << "Unused: ";
    for (unsigned reg = 0; reg < reg_used.size (); reg++)
      if (!reg_used[reg])
        {
          if (!first_output)
            ostr_sets << ",";
          ostr_sets << max_set_list.names[reg];
          first_output = false;
        }
    string new_set = ostr_sets.str ();
    std::ostringstream ostr_lines;
    ostr_lines << "<Placemark><name>" << new_set << "</name>";
    ostr_lines << "<styleUrl>#default</styleUrl><MultiGeometry>";
    for (const SetOfRegions *line : lines_used)
      {
        ostr_lines << "<Polygon><tessellate>1</tessellate>";
        ostr_lines << "<outerBoundaryIs><LinearRing><coordinates>";
        for(SphericalPoint pt: line->geodesic)
          ostr_lines << pt.lon << "," << pt.lat << ",0 ";
        ostr_lines << "</coordinates></LinearRing></outerBoundaryIs>";
        ostr_lines << "</Polygon>";
      }
    ostr_lines << "</MultiGeometry></Placemark>";
    string new_lines = ostr_lines.str ();
    bool found = false;
    for (const std::pair<string, string> &old_pair : best_sets)
      if (old_pair.second == new_set)
        found = true;
    if (!found)
      best_sets.push_back (std::make_pair (new_lines, new_set));
  }

  void
  clear ()
  {
    best_sets.clear ();
  }

  void
  print (std::ostream &ostr)
  {
    std::sort (best_sets.begin (), best_sets.end ());
    /*for (const std::pair<string, string> &best_set : best_sets)
      ostr << best_set.second << endl;*/
    ostr
        << "<kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document><Style id=\"default\"><PolyStyle><outline>1</outline><fill>0</fill></PolyStyle></Style>";
    for (const std::pair<string, string> &best_set : best_sets)
      ostr << best_set.first << endl;
    ostr << "</Document></kml>";
  }
};

Results best_sets;
vector<bool> reg_used;
vector<const SetOfRegions *> lines_used;
unsigned set_cnt;
unsigned max_used_cnt = 0;
unsigned lines_allowed = 4;

void
back_track (const MaxSetList &max_set_list, unsigned min_avail,
            unsigned used_cnt, unsigned dep_rem)
{
  if (dep_rem == 0)
    {
      if (used_cnt > max_used_cnt)
        best_sets.clear ();
      max_used_cnt = std::max (max_used_cnt, used_cnt);
      if (used_cnt >= max_used_cnt)
        best_sets.add_set (max_set_list, reg_used, lines_used);
      return;
    }
  unsigned max_add = 0;
  if (dep_rem >= 2)
    {
      for (unsigned i = min_avail; i < set_cnt; i++)
        {
          unsigned cur_add = 0;
          for (unsigned reg : max_set_list.set_list[i].ids)
            if (!reg_used[reg])
              cur_add++;
          max_add = std::max (max_add, cur_add);
        }
    }
  for (unsigned i = min_avail; i < set_cnt; i++)
    {
      vector<unsigned> new_used;
      for (unsigned reg : max_set_list.set_list[i].ids)
        if (!reg_used[reg])
          {
            new_used.push_back (reg);
            reg_used[reg] = true;
          }
      unsigned used_cnt_new = used_cnt + new_used.size ();

      if (used_cnt_new + (dep_rem - 1) * max_add >= max_used_cnt)
        {
          lines_used.push_back (&max_set_list.set_list[i]);
          back_track (max_set_list, i + 1, used_cnt_new, dep_rem - 1);
          lines_used.resize (lines_used.size () - 1);
        }

      for (unsigned reg : new_used)
        reg_used[reg] = false;
    }
  if (dep_rem == lines_allowed - 1)
    cerr << min_avail << "\r" << std::flush;
}

void
print_help ()
{
  cerr
      << "This program finds maximal sets of collinear regions (countries, states, ...)"
      << endl;
  cerr
      << "Exactly one argument is required: the file with the region boundaries."
      << endl;
  cerr << "The output is written to stdout." << endl;
}

int
main (int argc, char *argv[])
{
  if (argc < 2 || argc > 3)
    {
      print_help ();
      return 0;
    }
  string filename = string (argv[1]);
  if (argc == 3)
    lines_allowed = atoi (argv[2]);

  cerr << "Using collinear sets of states from file \"" << filename
      << "\". Trying to cover them with " << lines_allowed << " lines." << endl;

  vector<MaxSetList::SingleSet> single_sets;

  std::unique_ptr<LoaderVisitorInterface> loader_visitor(
        new LoaderVisitorMaxSetList(&single_sets));
  std::unique_ptr<Loader> loader(new Loader(loader_visitor.get()));
  bool success = loader->load_borders(filename);
  loader.reset(nullptr); // destroy loader first
  loader_visitor.reset(nullptr); // then the visitor
  cout << "Read " << single_sets.size() << " collinear sets" << endl;

  if (!success)
  {
     cerr << "Failed to load data from " << filename << endl;
     return 0;
  }

  MaxSetList max_set_list (single_sets);
  if (lines_allowed == 0 || lines_allowed > max_set_list.set_list.size ())
    {
      cerr << "Unexpected number of lines: " << lines_allowed << endl;
      print_help ();
      return 0;
    }

  set_cnt = max_set_list.set_list.size ();
  unsigned reg_cnt = max_set_list.names.size ();
  reg_used.resize (reg_cnt, false);

  back_track (max_set_list, 0, 0, lines_allowed);

 /* cout << "Highest number of regions crossed by four lines is " << max_used_cnt
      << " out of " << reg_cnt << endl;*/
  best_sets.print (cout);
  return 0;
}
