#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "line_finder.h"
#include "max_set_list.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

SetOfRegions::SetOfRegions (const Point &p1_in, const Point &p2_in) :
    p1 (p1_in), p2 (p2_in)
{
}

SetOfRegions
SetOfRegions::create_from_crossing_counts (
    const std::vector<unsigned> &crossings, const Point &p1_in,
    const Point &p2_in)
{
  SetOfRegions result (p1_in, p2_in);
  for (unsigned i = 0; i < crossings.size (); i++)
    if (crossings[i] > 0)
      result.ids.push_back (i);
  return std::move (result);
}

/// For each state, returns the number of times its boundary crosses the vertical line determined by the x-coordinate.
/// If the line touches the state in a corner, this counts as two crossings.
/// @result Vector whose i-th value is the number of crossings with the i-th state.
vector<unsigned>
get_horizontal_line_crossing_cnts (double y,
                                   const std::vector<RegionData> &regs)
{
  vector<unsigned> line;
  for (const RegionData &reg : regs)
    {
      unsigned crosses = 0;
      for (const vector<Point> &bdry : reg.boundaries)
        {
          assert(bdry.size () > 0);
          Point prev = *(bdry.end () - 1);
          for (const Point &cur : bdry)
            {
              if ((prev.y <= y && cur.y >= y) || (prev.y >= y && cur.y <= y))
                crosses++;
              prev = cur;
            }
        }
      line.push_back (crosses);
    }
  assert(line.size () == regs.size ());
  cout << "y: " << y << " Regions: ";
  for (unsigned i = 0; i < line.size (); i++)
    {
      if (line[i] > 0)
        cout << regs[i].name << ": " << line[i] << "x  ";
    }
  cout << endl;
  return std::move (line);
}

class PointAngle
{
public:
  static const double kEps;

  PointAngle (const vector<RegionData> &regs, const Point &center_in,
              unsigned reg_id_in, unsigned polygon_id_in, unsigned pt_id_in) :
      center (&center_in), reg_id (reg_id_in), polygon_id (polygon_id_in), pt_id (
          pt_id_in)
  {
    const vector<Point> &polygon = regs[reg_id].boundaries[polygon_id];
    pt = &polygon[pt_id];
    prev = (pt_id != 0 ? &polygon[pt_id - 1] : &polygon[polygon.size () - 1]);
    next = (pt_id != polygon.size () - 1 ? &polygon[pt_id + 1] : &polygon[0]);

    if ((pt->y == center->y && pt->x == center->x)
        || (prev->y == center->y && prev->x == center->x)
        || (next->y == center->y && next->x == center->x))
      {
        polygon_contains_center = true; // the polygon is always crossed -> the point can be ignored later
        return;
      }

    angle = atan2 (pt->y - center->y, pt->x - center->x);
    angle = normalize_angle (angle); // normalization may be useless here, but is needed in the latter occurrences

    double prev_angle = atan2 (prev->y - center->y, prev->x - center->x);
    prev_angle_diff = normalize_angle (prev_angle - angle);

    double next_angle = atan2 (next->y - center->y, next->x - center->x);
    next_angle_diff = normalize_angle (next_angle - angle);

    if (prev_angle_diff == M_PI || next_angle_diff == M_PI)
      {
        polygon_contains_center = true; // the polygon is always crossed -> the point can be ignored later
        return;
      }

    /*cout << reg_id << ", " << polygon_id << ", " << pt_id << " prev_angle_diff: "
     << prev_angle_diff << " next_angle_diff: " << next_angle_diff
     << " this: " << pt->x << ", " << pt->y << " prev: " << prev->x << ", "
     << prev->y << endl;*/

    if (prev_angle_diff >= 0.0 && next_angle_diff > 0)
      event = kEnter;
    else if (prev_angle_diff < 0.0 && next_angle_diff <= 0)
      event = kLeave;
    else
      event = kNoChange;

    fill_alt_angle ();
  }

  bool
  operator< (const PointAngle &other) const
  {
    return (alt_angle < other.alt_angle);
  }

  const Point *center;
  const Point *pt;
  const Point *prev;
  const Point *next;
  unsigned reg_id;
  unsigned polygon_id;
  unsigned pt_id;
  bool polygon_contains_center = false;
  double angle;
  double alt_angle;

  double prev_angle_diff;
  double next_angle_diff;

  enum EventType
  {
    kEnter = 0, kLeave, kNoChange
  };

  EventType event = kNoChange;

private:
  static double
  normalize_angle (double alpha);

  void
  fill_alt_angle ();
};

const double PointAngle::kEps = 0.000000001;

double
PointAngle::normalize_angle (double alpha)
{
  while (alpha < -M_PI)
    alpha += 2 * M_PI;
  while (alpha >= M_PI)
    alpha -= 2 * M_PI;
  return alpha;
}

void
PointAngle::fill_alt_angle ()
{
  alt_angle = angle;
  if (alt_angle < 0)
    alt_angle += M_PI;
  switch (event)
    {
    case kEnter:
      alt_angle -= kEps;
      break;
    case kLeave:
      alt_angle += kEps;
      break;
    case kNoChange:
      break;
    }
}

void
find_lines_from_center (MaxSetList *set_list, const Point &center,
                        const vector<RegionData> &regs)
{
  vector<unsigned> cross_cnts = get_horizontal_line_crossing_cnts (center.y,
                                                                   regs);
  vector<unsigned> cross_cnts_orig = cross_cnts;

  vector<PointAngle> event_list;
  vector<bool> always_crossed (regs.size (), false);

  for (unsigned reg_id = 0; reg_id < regs.size (); reg_id++)
    {
      const RegionData &reg = regs[reg_id];
      vector<PointAngle> new_events;
      for (unsigned bdry_id = 0; bdry_id < reg.boundaries.size (); bdry_id++)
        {
          const vector<Point> &bdry = reg.boundaries[bdry_id];
          assert(bdry.size () > 0);
          for (unsigned pt_id = 0; pt_id < bdry.size (); pt_id++)
            {
              PointAngle new_event = PointAngle (regs, center, reg_id, bdry_id,
                                                 pt_id);
              if (new_event.polygon_contains_center)
                {
                  assert(cross_cnts[reg_id] > 0);
                  always_crossed[reg_id] = true;
                  break;
                }
              if (new_event.event != PointAngle::kNoChange)
                new_events.push_back (std::move (new_event));
            }
          if (always_crossed[reg_id])
            break;
        }
      if (!always_crossed[reg_id])
        event_list.insert (event_list.end (), new_events.begin (),
                           new_events.end ());
    }

  std::sort (event_list.begin (), event_list.end ());

/*  SetOfRegions
  new_reg = SetOfRegions::create_from_crossing_counts (cross_cnts, center, Point::create_gnomonic_from_spherical(center.lon));
  set_list->add_set (std::move (new_reg));*/

  for (const PointAngle &event : event_list)
    {
      int reg_id = event.reg_id;
      /*cout << event.reg_id << ", " << event.polygon_id << ", " << event.pt_id
       << " prev_angle_diff: " << event.prev_angle_diff
       << " next_angle_diff: " << event.next_angle_diff << " this: "
       << event.pt->x << ", " << event.pt->y << " prev: " << event.prev->x
       << ", " << event.prev->y << endl;
       cout << "Event " << std::setprecision (10) << event.angle << " "
       << event.alt_angle << endl;*/
      if (!always_crossed[reg_id])
        {
          if (event.event == PointAngle::kEnter)
            {
              cross_cnts[reg_id] += 2;
              SetOfRegions new_reg = SetOfRegions::create_from_crossing_counts (
                  cross_cnts, center, *event.pt);
              set_list->add_set (std::move (new_reg));
            }
          else if (event.event == PointAngle::kLeave)
            {
              assert(cross_cnts[reg_id] >= 2);
              cross_cnts[reg_id] -= 2;
            }
        }
    }
  assert(cross_cnts.size () == cross_cnts_orig.size ());
  for (unsigned i = 0; i < cross_cnts.size (); i++)
    assert(cross_cnts[i] == cross_cnts_orig[i]);
}

void
find_lines (MaxSetList *set_list, const vector<RegionData> &regs)
{
  for (const RegionData &reg : regs)
    {
      cout << "Starting " << reg.name << endl;
      for (const vector<Point> &bdry : reg.boundaries)
        {
          assert(bdry.size () > 0);
          for (const Point &center : bdry)
            find_lines_from_center (set_list, center, regs);
        }
    }
}
