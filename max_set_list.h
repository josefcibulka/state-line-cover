#ifndef MAX_SET_LIST_H__
#define MAX_SET_LIST_H__

#include <string>
#include <vector>
#include <iostream>

#include "line_finder.h" // just because of SetOfRegions

class RegionData;

class MaxSetList
{
public:
   struct SingleSet
   {
   public:
      std::vector<std::string> names_;
      std::vector<SphericalPoint> geodesic_;
   };

   MaxSetList(const std::vector<SingleSet> &sets);
   MaxSetList(const std::vector<RegionData> &regs);
//   MaxSetList(const std::vector<std::string> &list_descs);

   static bool
   is_super(const std::vector<unsigned> &super_set,
            const std::vector<unsigned> &sub_set);

   bool
   add_set(SetOfRegions new_set);

   void
   print_sets(std::ostream &ostr);

   std::vector<std::string> names;
   std::vector<SetOfRegions> set_list;
};

/**
 * Fills the provided MaxSetList.
 */
class LoaderVisitorMaxSetList : public LoaderVisitorInterface
{
public:
   LoaderVisitorMaxSetList(std::vector<MaxSetList::SingleSet> *colli_sets)
         : colli_sets_(colli_sets), new_colli_set_(nullptr)
   {
      colli_sets->clear();
   }

   ~LoaderVisitorMaxSetList() override
   {
   }

   bool visit_multi_geometry(std::string name) override
   {
      std::cout << "Visiting MultiGeometry " << name << std::endl;
      assert(colli_sets_);
      colli_sets_->push_back(MaxSetList::SingleSet());
      new_colli_set_ = &(*(colli_sets_->end() - 1));  // the element we've just added

      std::istringstream ss(name);
      std::vector<std::string> result;
      while (ss.good())
      {
         string substr;
         std::getline(ss, substr, ',');
         new_colli_set_->names_.push_back(substr);
      }
      return true;
   }

   bool visit_polygon() override
   {
      std::cout << "Visiting polygon." << std::endl;
      assert(new_colli_set_);
      if (new_colli_set_->geodesic_.size() > 0)
      {
         std::cerr
               << "Error: Only one polygon for a single collinear set is allowed."
               << std::endl;
         return false;
      }
      return true;
   }

   bool visit_single_point(double lon, double lat) override
   {
      std::cout << "Visiting Point: " << lon << ", " << lat << std::endl;
      assert(new_colli_set_);
      Point new_pt = Point::create_gnomonic_from_spherical(lon, lat);
      new_colli_set_->geodesic_.push_back(new_pt);
      return true;
   }

   bool finish_polygon() override
   {
      std::cout << "Finishing polygon." << std::endl;
      assert(new_colli_set_);
      if (new_colli_set_->geodesic_.size() == 0)
      {
         std::cerr << "Error: Empty polygon." << std::endl;
         return false;
      }
      return true;
   }

   bool finish_multi_geometry() override
   {
      assert(new_colli_set_);
      std::cout << "Read collinear set containing: ";
      for (const string &name : new_colli_set_->names_)
         std::cout << name << ", ";
      std::cout << std::endl;
      return true;
   }

private:
   std::vector<MaxSetList::SingleSet> *colli_sets_;
   MaxSetList::SingleSet *new_colli_set_;
};

#endif
