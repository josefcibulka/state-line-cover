#ifndef MAX_SET_LIST_H__
#define MAX_SET_LIST_H__

#include <string>
#include <vector>

#include "line_finder.h" // just because of SetOfRegions

class RegionData;

class MaxSetList
{
public:
  MaxSetList (const std::vector<RegionData> &regs);
  MaxSetList (const std::vector<std::string> &list_descs);

  static bool
  is_super (const std::vector<unsigned> &super_set,
            const std::vector<unsigned> &sub_set);

  bool
  add_set (SetOfRegions new_set);

  void
  print_sets ();

  std::vector<std::string> names;
  std::vector<SetOfRegions> set_list;
};

#endif
