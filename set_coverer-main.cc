#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include "max_set_list.h"

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

vector<string> best_sets;
vector<bool> reg_used;
unsigned set_cnt;
unsigned max_used_cnt = 0;

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
        {
          bool first_output = true;
          std::ostringstream ostr;
          ostr << "Unused: ";
          for (unsigned reg = 0; reg < reg_used.size (); reg++)
            if (!reg_used[reg])
              {
                if (!first_output)
                  ostr << ",";
                ostr << max_set_list.names[reg];
                first_output = false;
              }
          best_sets.push_back (ostr.str ());
        }
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
        back_track (max_set_list, i + 1, used_cnt_new, dep_rem - 1);

      for (unsigned reg : new_used)
        reg_used[reg] = false;
    }
  if (dep_rem == 4)
    cerr << min_avail << "\r" << std::flush;
}

int
main (void)
{
  vector<string> input_lines;
  string read_line;
  while (std::getline (std::cin, read_line))
    if (read_line.length () > 0)
      input_lines.push_back (read_line);

  cout << "Read " << input_lines.size () << " lines." << endl;

  MaxSetList max_set_list (input_lines);

  set_cnt = max_set_list.set_list.size ();
  unsigned reg_cnt = max_set_list.names.size ();
  reg_used.resize (reg_cnt, false);

  back_track (max_set_list, 0, 0, 5);

  cout << "Highest number of regions crossed by four lines is " << max_used_cnt
      << " out of " << reg_cnt << endl;
  for (const string &best_set : best_sets)
    cout << best_set << endl;
  return 0;
}
