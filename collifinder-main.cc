#include <cstddef>
#include <string>
#include <iostream>
#include <fstream>

#include "loader.h"
#include "line_finder.h"
#include "max_set_list.h"

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

void print_help()
{
   cerr << "This program finds maximal sets of collinear regions (countries, states, ...)"
        << endl;
   cerr << "Exactly two arguments are required: "
        "the file with the region boundaries and the file where the output will be written."
        << endl;
}

std::size_t count_vertices(const vector<RegionData> &regs)
{
   std::size_t total_vertices = 0;
   for (const RegionData &reg : regs)
      for (const vector<Point> &pt : reg.boundaries)
         total_vertices += pt.size();
   return total_vertices;
}

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
      print_help();
      return 0;
   }
   string filename = string(argv[1]);
   string filename_out = string(argv[2]);
   vector<RegionData> regs;

   std::unique_ptr<LoaderVisitorInterface> loader_visitor(
         new LoaderVisitorRegionData(&regs));
   std::unique_ptr<Loader> loader(new Loader(loader_visitor.get()));
   bool success = loader->load_borders(filename);
   loader.reset(nullptr); // destroy loader first
   loader_visitor.reset(nullptr); // then the visitor
   cout << "Read data for " << regs.size() << " areas" << endl;

   if (!success)
   {
      cerr << "Failed to load data from " << filename << endl;
      return 0;
   }

   cout << "Before reduction to convex hulls: " << count_vertices(regs)
        << " vertices." << endl;
   for (RegionData &reg : regs)
      reg.reduce_to_convex_hulls();
   cout << "After reduction to convex hulls: " << count_vertices(regs)
        << " vertices." << endl;

   MaxSetList lines(regs);
   find_lines(&lines, regs);
   std::ofstream result_stream(filename_out);
   lines.print_sets(result_stream);
   return 0;
}
