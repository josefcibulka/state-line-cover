#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include "max_set_list.h"
#include "line_finder.h" // just because of SetOfRegions
#include "region_data.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

MaxSetList::MaxSetList(const std::vector<SingleSet> &sets)
{
   std::map<string, unsigned> name2id;

   for (const SingleSet &set : sets)
   {
      SetOfRegions cur_set(set.geodesic_);
      for (const string &item : set.names_)
      {
         auto it = name2id.find(item);
         if (it == name2id.end())
         {
            name2id[item] = names.size();
            names.push_back(item);
         }
         it = name2id.find(item);
         assert(it != name2id.end());
         cur_set.ids.push_back(it->second);
      }
      set_list.push_back(std::move(cur_set));
   }
}

/**
 * Creates an empty list of sets of regions.
 * @param regs Provides the names of the regions that will be part of later added sets.
 */
MaxSetList::MaxSetList(const vector<RegionData> &regs)
{
   for (const RegionData &reg : regs)
      names.push_back(reg.name);
}


bool MaxSetList::is_super(const vector<unsigned> &super_set,
                          const vector<unsigned> &sub_set)
{
   auto super_it = super_set.cbegin();
   for (unsigned new_el : sub_set)
   {
      while (super_it != super_set.cend() && (*super_it) < new_el)
         ++super_it;
      if (super_it == super_set.cend() || (*super_it) > new_el)
         return false;
      // Now we know that (*old_it) == new_el, so we can move old_it forward.
      ++super_it;
   }
   return true;
}

/// The elements in new_set must be sorted.
///@return true iff neither the set nor some its supersets was in the list. In such a case, the set was added just now.
bool MaxSetList::add_set(SetOfRegions new_set)
{
   for (unsigned i = 0; i < set_list.size(); i++)
   {
      const vector<unsigned> &old_set = set_list[i].ids;
      bool old_is_super = is_super(old_set, new_set.ids);
      if (old_is_super)
      {
         // Move the superset to the first place, because it is likely that it will be a superset for the upcoming sets.
         // This will speed up the process since we will quickly find the superset for the upcoming sets.
         SetOfRegions new_first = std::move(set_list[i]);
         std::move_backward(set_list.begin(), set_list.begin() + i,
                            set_list.begin() + i + 1);
         set_list[0] = std::move(new_first);
         return false;
      }
   }
   // Set not found -> add it.
   cout << "New set: ";
   for (unsigned id : new_set.ids)
   {
      assert(id < names.size());
      cout << names[id] << " ";
   }
   cout << endl;

   /// So far, it was only checked that the new set is not a subset (or identical copy) of an already added set.
   /// Now we will remove the subsets of the newly set that are already in.
   vector<SetOfRegions> reduced_set_list;
   for (unsigned i = 0; i < set_list.size(); i++)
      if (!is_super(new_set.ids, set_list[i].ids))
         reduced_set_list.push_back(std::move(set_list[i]));
   set_list = std::move(reduced_set_list);

   set_list.push_back(std::move(new_set));
   return true;
}

void MaxSetList::print_sets(std::ostream &ostr)
{
   ostr << "<kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document><Style id=\"default\"><PolyStyle><outline>1</outline><fill>0</fill></PolyStyle></Style>";

   for (const SetOfRegions &line : set_list)
   {
      std::ostringstream ostr_set;
      bool first_output = true;
      for (unsigned id : line.ids)
      {
         assert(id < names.size());
         if (!first_output)
            ostr_set << ",";
         ostr_set << names[id];
         first_output = false;
      }

      string set_string = ostr_set.str();
      ostr << "<Placemark><name>" << set_string << "</name>";
      ostr << "<styleUrl>#default</styleUrl><MultiGeometry>";
      ostr << "<Polygon><tessellate>1</tessellate>";
      ostr << "<outerBoundaryIs><LinearRing><coordinates>";
      for(SphericalPoint pt: line.geodesic)
        ostr << pt.lon << "," << pt.lat << ",0 ";
      ostr << "</coordinates></LinearRing></outerBoundaryIs>";
      ostr << "</Polygon>";
      ostr << "</MultiGeometry></Placemark>";
   }
   ostr << "</Document></kml>";
}
