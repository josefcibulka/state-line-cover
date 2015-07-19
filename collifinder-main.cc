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
  string filename = string (argv[1]);
  vector<RegionData> regs;
  bool success = load_borders (&regs, filename);
  if (!success)
    {
      cerr << "Failed to load data from " << filename << endl;
      return 0;
    }
  for (RegionData &reg : regs)
    reg.reduce_to_convex_hulls ();
  MaxSetList lines (regs);
  find_lines (&lines, regs);
  lines.print_sets ();
  return 0;
}
