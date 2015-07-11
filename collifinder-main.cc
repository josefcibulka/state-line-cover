#include <string>
#include <iostream>

#include "loader.h"
#include "line_finder.h"
#include "max_set_list.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

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
  if (argc != 2)
    {
      print_help ();
      return 0;
    }
  vector<RegionData> regs;
  // load_borders_csv (&regs, "data/KML-US_states-no_alabama.csv",1);
  // load_borders_csv (&regs, "data/US_states_USE_THIS.csv",0);
  // load_borders_kml (&regs, "data/gz_2010_us_040_00_500k.kml");
  string filename = string (argv[1]);
  bool success = load_borders (&regs, filename);
  if (!success)
    {
      cerr << "Failed to load data from " << filename << endl;
      return 0;
    }
  MaxSetList lines (regs);
  find_lines (&lines, regs);
  lines.print_sets ();
  return 0;
}
