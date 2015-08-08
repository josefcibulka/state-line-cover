#ifndef LOADER_H__
#define LOADER_H__

#include <cmath>
#include <string>
#include <vector>

#include "kml/dom.h"

#include "region_data.h"

class LoaderVisitorInterface
{
public:
   virtual ~LoaderVisitorInterface()
   {
   }
   virtual bool visit_multi_geometry(std::string name) = 0;
   virtual bool visit_polygon() = 0;
   virtual bool visit_single_point(double lon, double lat) = 0;
   virtual bool finish_polygon() = 0;
   virtual bool finish_multi_geometry() = 0;
};

/**
 * Fills the provided vector of RegionData.
 */
class LoaderVisitorRegionData : public LoaderVisitorInterface
{
public:
   LoaderVisitorRegionData(std::vector<RegionData> *regions)
         : regions_(regions), new_region_(nullptr), new_bdry_(nullptr)
   {
      regions_->clear();
   }

   ~LoaderVisitorRegionData() override
   {
   }

   bool visit_multi_geometry(std::string name) override
   {
      std::cout << "Visiting MultiGeometry " << name << std::endl;
      assert(regions_);
      regions_->push_back(RegionData());
      new_region_ = &(*(regions_->end() - 1));  // the element we've just added
      new_region_->name = name;
      new_bdry_ = nullptr;
      return true;
   }

   bool visit_polygon() override
   {
      std::cout << "Visiting polygon." << std::endl;
      assert(new_region_);
      new_region_->boundaries.push_back(std::vector<Point>());
      new_bdry_ = &(*(new_region_->boundaries.end() - 1));  // the element we've just added
      return true;
   }

   bool visit_single_point(double lon, double lat) override
   {
      std::cout << "Visiting Point: " << lon << ", " << lat << std::endl;
      assert(new_bdry_);
      Point new_pt = Point::create_gnomonic_from_spherical(lon, lat);
      new_bdry_->push_back(new_pt);
      return true;
   }

   bool finish_polygon() override
   {
      std::cout << "Finishing polygon." << std::endl;
      assert(new_bdry_);
      if (new_bdry_->size() == 0)
      {
         std::cerr << "Error: Empty polygon." << std::endl;
         return false;
      }
      return true;
   }

   bool finish_multi_geometry() override
   {
      assert(new_region_);
      std::size_t ver_total = 0;
      for (auto &bdry : new_region_->boundaries)
         ver_total += bdry.size();
      std::cout << new_region_->name << " has "
                << new_region_->boundaries.size()
                << " polygons with the total number of " << ver_total
                << " vertices " << std::endl;
      return true;
   }

private:
   std::vector<RegionData> *regions_;
   RegionData *new_region_;
   std::vector<Point> *new_bdry_;
};

class Loader
{
public:
   Loader(LoaderVisitorInterface *visitor);

   bool load_borders_csv(std::string filename);

   bool load_borders_kml(std::string filename);

   bool load_borders(std::string filename);

private:
   bool process_placemark(const kmldom::ElementPtr &elem, std::string reg_name);

   bool fill_region_data(std::string &reg_name,
                         const kmldom::ElementPtr &root_el);

   bool fill_polygon(const kmldom::ElementPtr &polygon_el);

   bool fill_polygon_from_line_string(const kmldom::ElementPtr &line_el);

   bool process_coord_list(const kmldom::CoordinatesPtr &coord_list,
                           bool is_ring);

   LoaderVisitorInterface *visitor_;
};

#endif
