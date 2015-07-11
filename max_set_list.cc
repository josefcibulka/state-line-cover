#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include "max_set_list.h"
#include "loader.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

MaxSetList::MaxSetList (const vector<RegionData> &regs)
{
  for (const RegionData &reg : regs)
    names.push_back (reg.name);
}

MaxSetList::MaxSetList (const vector<string> &list_descs)
{
  vector<vector<string> > items;

  for (const string &one_set_desc : list_descs)
    {
      std::istringstream istr (one_set_desc);
      vector<string> cur_set;
      string cur_item;
      while (std::getline (istr, cur_item, ','))
        cur_set.push_back (cur_item);
      items.push_back (std::move (cur_set));
    }

  std::map<string, unsigned> name2id;

  for (const vector<string> &one_set : items)
    {
      SetOfRegions cur_set;
      for (const string &item : one_set)
        {
          auto it = name2id.find (item);
          if (it == name2id.end ())
            {
              name2id[item] = names.size ();
              names.push_back (item);
            }
          it = name2id.find (item);
          assert(it != name2id.end ());
          cur_set.ids.push_back (it->second);
        }
      set_list.push_back (std::move (cur_set));
    }
}

bool
MaxSetList::is_super (const vector<unsigned> &super_set,
                      const vector<unsigned> &sub_set)
{
  auto super_it = super_set.cbegin ();
  for (unsigned new_el : sub_set)
    {
      while (super_it != super_set.cend () && (*super_it) < new_el)
        ++super_it;
      if (super_it == super_set.cend () || (*super_it) > new_el)
        return false;
      // Now we know that (*old_it) == new_el, so we can move old_it forward.
      ++super_it;
    }
  return true;
}

/// The elements in new_set must be sorted.
///@return true iff neither the set nor some its supersets was in the list. In such a case, the set was added just now.
bool
MaxSetList::add_set (SetOfRegions new_set)
{
  for (unsigned i = 0; i < set_list.size (); i++)
    {
      const vector<unsigned> &old_set = set_list[i].ids;
      bool old_is_super = is_super (old_set, new_set.ids);
      if (old_is_super)
        {
          // Move the superset to the first place, because it is likely that it will be a superset for the upcoming sets.
          // This will speed up the process since we will quickly find the superset for the upcoming sets.
          SetOfRegions new_first = std::move (set_list[i]);
          std::move_backward (set_list.begin (), set_list.begin () + i,
                              set_list.begin () + i + 1);
          set_list[0] = std::move (new_first);
          return false;
        }
    }
  // Set not found -> add it.
  cout << "New set: ";
  for (unsigned id : new_set.ids)
    {
      assert(id < names.size ());
      cout << names[id] << " ";
    }
  cout << endl;

  /// So far, it was only checked that the new set is not a subset (or identical copy) of an already added set.
  /// Now we will remove the subsets of the newly set that are already in.
  vector<SetOfRegions> reduced_set_list;
  for (unsigned i = 0; i < set_list.size (); i++)
    if (!is_super (new_set.ids, set_list[i].ids))
      reduced_set_list.push_back (std::move (set_list[i]));
  set_list = std::move (reduced_set_list);

  set_list.push_back (std::move (new_set));
  return true;
}

void
MaxSetList::print_sets ()
{
  cout << "The sets: " << endl;
  for (auto line : set_list)
    {
      bool first_output = true;
      for (unsigned id : line.ids)
        {
          assert(id < names.size ());
          if (!first_output)
            cout << ",";
          cout << names[id];
          first_output = false;
        }
      cout << endl;
    }
}
