#include <iostream>
#include <vector>
#include <string>

#include "max_set_list.h"

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

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

  unsigned set_cnt = max_set_list.set_list.size ();
  unsigned reg_cnt = max_set_list.names.size ();

  unsigned max_used_cnt = 0;

  for (unsigned i = 0; i < set_cnt; i++)
    {
      for (unsigned j = i + 1; j < set_cnt; j++)
        for (unsigned k = j + 1; k < set_cnt; k++)
          for (unsigned l = k + 1; l < set_cnt; l++)
            {
              vector<bool> reg_used (reg_cnt, false);
              for (unsigned reg : max_set_list.set_list[i].ids)
                reg_used[reg] = true;
              for (unsigned reg : max_set_list.set_list[j].ids)
                reg_used[reg] = true;
              for (unsigned reg : max_set_list.set_list[k].ids)
                reg_used[reg] = true;
              for (unsigned reg : max_set_list.set_list[l].ids)
                reg_used[reg] = true;
              unsigned used_cnt = 0;
              for (bool cur_used : reg_used)
                if (cur_used)
                  used_cnt++;
              max_used_cnt = std::max (max_used_cnt, used_cnt);
              if (used_cnt >= reg_cnt - 2)
                {
                  bool first_output = true;
                  cout << "Unused: ";
                  for (unsigned i = 0; i < reg_used.size (); i++)
                    if (!reg_used[i])
                      {
                        if(!first_output)
                          cout << ",";
                        cout << max_set_list.names[i];
                        first_output = false;
                      }
                  cout << endl;
                }
            }
      cerr << i << "\r" << std::flush;
    }
  cout << "Highest number of regions crossed by four lines is " << max_used_cnt
      << " out of " << reg_cnt << endl;
  return 0;
}
