// The main program (provided by the course), COMP.CS.300
//
// DO ****NOT**** EDIT THIS FILE!
// (Preferably do not edit this even temporarily. And if you still decide to do so
//  (for debugging, for example), DO NOT commit any changes to git, but revert all
//  changes later. Otherwise you won't be able to get any updates/fixes to this
//  file from git!)

#include <string>
using std::string;
using std::getline;

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::flush;
using std::noskipws;

#include <iomanip>
using std::setfill;
using std::setw;

#include <istream>
using std::istream;

#include <ostream>
using std::ostream;

#include <fstream>
using std::ifstream;

#include <sstream>
using std::istringstream;
using std::ostringstream;
using std::stringstream;

#include <iomanip>
using std::setw;

#include <tuple>
using std::tuple;
using std::make_tuple;
using std::get;

#include <regex>
using std::regex_match;
using std::regex_search;
using std::smatch;
using std::regex;
using std::sregex_token_iterator;

#include <algorithm>
using std::find_if;
using std::find;
using std::reverse;
using std::binary_search;
using std::max_element;
using std::max;
using std::min;
using std::sort;
using std::all_of;

#include <random>
using std::minstd_rand;
using std::uniform_int_distribution;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <set>
using std::set;

#include <array>
using std::array;

#include <ctime>
using std::time;

#include <utility>
using std::pair;
using std::swap;

#include <cstdlib>
using std::div;

#include <limits>
using std::numeric_limits;

#include <cassert>


#include "mainprogram.hh"

#include "datastructures.hh"

#ifdef GRAPHICAL_GUI
#include "mainwindow.h"
#endif

string const MainProgram::PROMPT = "> ";

MainProgram::CmdResult MainProgram::cmd_place_count(std::ostream& output, MatchIter begin, MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    output << "Number of places: " << ds_.place_count() << endl;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_clear_all(std::ostream& output, MatchIter begin, MatchIter end)
{
    assert(begin == end && "Invalid number of parameters");

    ds_.clear_all();
    init_primes();

    output << "Cleared everything." << endl;

    view_dirty = true;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_all_places(std::ostream& output, MatchIter begin, MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    auto places = ds_.all_places();
    if (places.empty())
    {
        output << "No places!" << endl;
    }

    sort(places.begin(), places.end());
    return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, places}};
}

MainProgram::CmdResult MainProgram::cmd_add_place(std::ostream& output, MatchIter begin, MatchIter end)
{
    string idstr = *begin++;
    string name = *begin++;
    string typestr = *begin++;
    string xstr = *begin++;
    string ystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceID id = convert_string_to<PlaceID>(idstr);
    PlaceType type = convert_string_to_placetype(typestr);
    if (type == PlaceType::NO_TYPE)
    {
        output << "Impossible place type: " << typestr << endl;
        return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, {NO_PLACE}}};
    }
    Coord xy = {convert_string_to<int>(xstr), convert_string_to<int>(ystr)};

    bool success = ds_.add_place(id, name, type, xy);
    if (!success) { id = NO_PLACE; }

    view_dirty = true;
    return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, {id}}};
}

MainProgram::CmdResult MainProgram::cmd_place_name_type(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string placeidstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceID placeid = convert_string_to<PlaceID>(placeidstr);

    auto [name, type] = ds_.get_place_name_type(placeid);
    if (name == NO_NAME)
    {
        return {ResultType::PLACEIDLIST, MainProgram::CmdResultPlaceIDs{NO_AREA, {NO_PLACE}}};
    }
    else
    {
        output << "Place ID " << placeid << " has name '" << name << "' and type '" << convert_placetype_to_string(type) << "'" << endl;
        return {ResultType::PLACEIDLIST, MainProgram::CmdResultPlaceIDs{NO_AREA, {placeid}}};
    }
}

MainProgram::CmdResult MainProgram::cmd_place_coord(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string placeidstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceID placeid = convert_string_to<PlaceID>(placeidstr);

    auto coord = ds_.get_place_coord(placeid);
    if (coord == NO_COORD)
    {
        return {ResultType::PLACEIDLIST, MainProgram::CmdResultPlaceIDs{NO_AREA, {NO_PLACE}}};
    }
    else
    {
        output << "Place ID " << placeid << " is in position ";
        print_coord(coord, output);
        return {ResultType::PLACEIDLIST, MainProgram::CmdResultPlaceIDs{NO_AREA, {placeid}}};
    }
}

MainProgram::CmdResult MainProgram::cmd_add_area(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    string name = *begin++;
    string coordsstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id = convert_string_to<AreaID>(idstr);

    vector<Coord> coords;
    smatch coord;
    auto sbeg = coordsstr.cbegin();
    auto send = coordsstr.cend();
    for ( ; regex_search(sbeg, send, coord, coords_regex_); sbeg = coord.suffix().first)
    {
        coords.push_back({convert_string_to<int>(coord[1]),convert_string_to<int>(coord[2])});
    }

    if (coords.size() < 3)
    {
        output << "An area must have at least 3 coords, only " << coords.size() << " coords given!" << endl;
        return {};
    }

    bool success = ds_.add_area(id, name, coords);

    if (success)
    {
        view_dirty = true;
        return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{id, {}}};
    }
    else
    {
        output << "Adding area failed." << endl;
        return {};
    }
}

MainProgram::CmdResult MainProgram::cmd_area_name(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id = convert_string_to<AreaID>(idstr);

    auto result = ds_.get_area_name(id);
    if (result == NO_NAME)
    {
        return {ResultType::AREAIDLIST, MainProgram::CmdResultAreaIDs{NO_AREA}};
    }
    else
    {
        output << "Area ID " << id << " has name '" << result << "'" << endl;
        return {ResultType::AREAIDLIST, MainProgram::CmdResultAreaIDs{id}};
    }
}

MainProgram::CmdResult MainProgram::cmd_area_coords(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id = convert_string_to<AreaID>(idstr);

    auto coords = ds_.get_area_coords(id);

    if (coords.empty())
    {
        output << "No coords returned!" << endl;
        return {};
    }

    if (coords.size() == 1 && coords.front() == NO_COORD)
    {
        return {ResultType::AREAIDLIST, MainProgram::CmdResultAreaIDs{NO_AREA}};
    }

    output << "Area "; print_area(id,output,false); output << " has coords:" << endl;
    std::for_each(coords.begin(), coords.end(), [&output,this](auto const& coord){ print_coord(coord,output); });
    output << endl;

    return {ResultType::AREAIDLIST, MainProgram::CmdResultAreaIDs{id}};
}

MainProgram::CmdResult MainProgram::cmd_find_places_name(std::ostream& output, MatchIter begin, MatchIter end)
{
    string name = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    auto result = ds_.find_places_name(name);
    if (result.empty())
    {
        output << "No Places!" << std::endl;
    }

    sort(result.begin(), result.end());
    return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, result}};
}

MainProgram::CmdResult MainProgram::cmd_find_places_type(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string typestr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceType type = convert_string_to_placetype(typestr);
    if (type == PlaceType::NO_TYPE)
    {
        output << "Impossible place type: " << typestr << endl;
        return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, {NO_PLACE}}};
    }

    auto result = ds_.find_places_type(type);
    if (result.empty())
    {
        output << "No Places!" << std::endl;
    }

    sort(result.begin(), result.end());
    return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, result}};
}

MainProgram::CmdResult MainProgram::cmd_change_place_name(std::ostream& /*output*/, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    string newname = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceID id = convert_string_to<PlaceID>(idstr);

    bool success = ds_.change_place_name(id, newname);
    if (!success) { id = NO_PLACE; }

    view_dirty = true;
    return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, {id}}};
}

MainProgram::CmdResult MainProgram::cmd_all_areas(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    auto areas = ds_.all_areas();
    if (areas.empty())
    {
        output << "No areas!" << endl;
    }

    sort(areas.begin(), areas.end());
    return {ResultType::AREAIDLIST, areas};
}

MainProgram::CmdResult MainProgram::cmd_add_subarea_to_area(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string sourceidstr = *begin++;
    string targetidstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID sourceid = convert_string_to<AreaID>(sourceidstr);
    AreaID targetid = convert_string_to<AreaID>(targetidstr);

    view_dirty = true;

    bool ok = ds_.add_subarea_to_area(sourceid, targetid);
    if (ok)
    {
        auto sourcename = ds_.get_area_name(sourceid);
        auto targetname = ds_.get_area_name(targetid);
        output << "Added subarea " << sourcename << " to area " << targetname << endl;
    }
    else
    {
        output << "Adding subarea to area failed!" << endl;
    }

    return {};
}

MainProgram::CmdResult MainProgram::cmd_subarea_in_areas(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id = convert_string_to<AreaID>(idstr);

    auto result = ds_.subarea_in_areas(id);
    if (result.empty())
    {
        output << "Area ";
        print_area(id, output, false);
        output << "is not a subarea of any area." << endl;
    }
    else
    {
        output << "Area hierarchy for area ";
        print_area(id, output);
        result.insert(result.begin(), id);
    }
    return {ResultType::AREAIDLIST, result};
}

MainProgram::CmdResult MainProgram::cmd_all_subareas_in_area(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id = convert_string_to<AreaID>(idstr);

    auto result = ds_.all_subareas_in_area(id);
    sort(result.begin(), result.end());
    if (result.empty())
    {
        output << "No subareas found for area "; print_area(id, output);
    }
    else
    {
        result.insert(result.begin(), id);
        output << "The area 1. below has areas 2.-... as subareas." << endl;
    }
    return {ResultType::AREAIDLIST, result};
}

MainProgram::CmdResult MainProgram::cmd_places_closest_to(std::ostream& /*output*/, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
  string xstr = *begin++;
  string ystr = *begin++;
  string typestr = *begin++;
  assert( begin == end && "Impossible number of parameters!");

  Coord coord = {convert_string_to<int>(xstr),convert_string_to<int>(ystr)};
  PlaceType type = PlaceType::NO_TYPE;
  if (!typestr.empty())
  {
      type = convert_string_to_placetype(typestr);
  }

  auto result = ds_.places_closest_to(coord, type);
  return {ResultType::PLACEIDLIST, CmdResultPlaceIDs{NO_AREA, result}};
}

MainProgram::CmdResult MainProgram::cmd_common_area_of_subareas(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string id1str = *begin++;
    string id2str = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    AreaID id1 = convert_string_to<PlaceID>(id1str);
    AreaID id2 = convert_string_to<PlaceID>(id2str);

    auto result = ds_.common_area_of_subareas(id1, id2);
    if (result == NO_AREA)
    {
        output << "No common area found!" << endl;
        return {};
    }

    output << "The area 1. below is the common area of areas 2. and 3." << endl;
    CmdResultAreaIDs resultarea({result, id1, id2});
    return {ResultType::AREAIDLIST, resultarea};
}

MainProgram::CmdResult MainProgram::cmd_all_ways(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    auto wayids = ds_.all_ways();
    if (wayids.empty())
    {
        output << "No ways!" << endl;
    }

    sort(wayids.begin(), wayids.end());

    unsigned int i = 1;
    for (auto const& wayid : wayids)
    {
        output << i <<". " << wayid << endl;
        ++i;
    }

    return {};
}

MainProgram::CmdResult MainProgram::cmd_add_way(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    string coordsstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    WayID id = idstr;

    vector<Coord> coords;
    smatch coord;
    auto sbeg = coordsstr.cbegin();
    auto send = coordsstr.cend();
    for ( ; regex_search(sbeg, send, coord, coords_regex_); sbeg = coord.suffix().first)
    {
        coords.push_back({convert_string_to<int>(coord[1]),convert_string_to<int>(coord[2])});
    }

    if (coords.size() < 2)
    {
        output << "A way must have at least 2 points, only " << coords.size() << " points given!" << endl;
        return {};
    }

    bool ok = ds_.add_way(id, coords);
    if (ok)
    {
        output << "Added way " << id << " with coords:";
        std::for_each(coords.begin(), coords.end(), [&output,this](auto const& coord){ output << ' '; print_coord(coord,output,false); });
        output << endl;

        std::vector<std::tuple<Coord, Coord, WayID, Distance>> result;
        result.emplace_back(coords.front(), coords.back(), id, NO_DISTANCE);
        result.emplace_back(coords.back(), NO_COORD, NO_WAY, NO_DISTANCE);
        return {ResultType::ROUTE, CmdResultRoute{result}};
    }
    else
    {
        output << "Adding way failed!" << endl;
        return {};
    }
}

MainProgram::CmdResult MainProgram::cmd_ways_from(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string xstr = *begin++;
    string ystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    Coord coord = {convert_string_to<int>(xstr),convert_string_to<int>(ystr)};

    auto ways = ds_.ways_from(coord);
    if (ways.empty())
    {
        output << "No ways from coord ";
        print_coord(coord, output);
    }

    sort(ways.begin(), ways.end());
    vector<tuple<Coord, Coord, WayID, Distance>> result;
    transform(ways.begin(), ways.end(), back_inserter(result),
              [coord](auto way)mutable{ return make_tuple(coord, way.second, way.first, NO_DISTANCE); });

    return {ResultType::WAYS, CmdResultRoute{result}};
}

MainProgram::CmdResult MainProgram::cmd_way_coords(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    WayID id = idstr;

    auto coords = ds_.get_way_coords(id);

    if (coords.empty())
    {
        output << "No coords returned!" << endl;
        return {};
    }

    output << "Way "; print_way(id, output, false); output << " has coords:" << endl;
    std::for_each(coords.begin(), coords.end(), [&output,this](auto const& coord){ print_coord(coord,output); });
    output << endl;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_remove_place(std::ostream& output, MatchIter begin, MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    PlaceID id = convert_string_to<PlaceID>(idstr);
    auto [name,type] = ds_.get_place_name_type(id);
    bool success = ds_.remove_place(id);
    if (success)
    {
        output << "Place " << name << "(" << convert_placetype_to_string(type) << ") removed." << endl;
        view_dirty = true;
        return {};
    }
    else
    {
        return {ResultType::PLACEIDLIST, MainProgram::CmdResultPlaceIDs{NO_AREA, {NO_PLACE}}};
    }
}

MainProgram::CmdResult MainProgram::cmd_clear_ways(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    ds_.clear_ways();
    output << "All ways removed." << std::endl;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_route_any(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string fromxstr = *begin++;
    string fromystr = *begin++;
    string toxstr = *begin++;
    string toystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    Coord fromxy = {convert_string_to<int>(fromxstr),convert_string_to<int>(fromystr)};
    Coord toxy = {convert_string_to<int>(toxstr),convert_string_to<int>(toystr)};

    auto steps = ds_.route_any(fromxy, toxy);

    vector<tuple<Coord, Coord, WayID, Distance>> result;

    if (steps.empty())
    {
        output << "No route found!" << endl;
    }
    else if (steps.front() == make_tuple(NO_COORD, NO_WAY, NO_DISTANCE))
    {
        output << "Starting or destination coord has no ways!" << endl;
    }
    else
    {
        auto [coord, wayid, dist] = steps.front();
        for (auto iter = steps.begin()+1; iter != steps.end(); ++iter)
        {
            auto& [ncoord, nwayid, ndist] = *iter;
            result.emplace_back(coord, ncoord, wayid, dist);
            coord = ncoord; wayid = nwayid; dist = ndist;
        }
        result.emplace_back(coord, NO_COORD, NO_WAY, dist);
    }

    return {ResultType::ROUTE, result};
}

MainProgram::CmdResult MainProgram::cmd_remove_way(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string idstr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    WayID id = idstr;

    bool ok = ds_.remove_way(id);
    if (ok)
    {
        output << "Removed way " << id << endl;
    }
    else
    {
        output << "Removing way failed!" << endl;
    }

    return {};
}

MainProgram::CmdResult MainProgram::cmd_route_least_crossroads(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string fromxstr = *begin++;
    string fromystr = *begin++;
    string toxstr = *begin++;
    string toystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    Coord fromxy = {convert_string_to<int>(fromxstr),convert_string_to<int>(fromystr)};
    Coord toxy = {convert_string_to<int>(toxstr),convert_string_to<int>(toystr)};

    auto steps = ds_.route_least_crossroads(fromxy, toxy);

    vector<tuple<Coord, Coord, WayID, Distance>> result;

    if (steps.empty())
    {
        output << "No route found!" << endl;
    }
    else if (steps.front() == make_tuple(NO_COORD, NO_WAY, NO_DISTANCE))
    {
        output << "Starting or destination coord has no ways!" << endl;
    }
    else
    {
        auto [coord, wayid, dist] = steps.front();
        for (auto iter = steps.begin()+1; iter != steps.end(); ++iter)
        {
            auto& [ncoord, nwayid, ndist] = *iter;
            result.emplace_back(coord, ncoord, wayid, dist);
            coord = ncoord; wayid = nwayid; dist = ndist;
        }
        result.emplace_back(coord, NO_COORD, NO_WAY, dist);
    }

    return {ResultType::ROUTE, result};
}

MainProgram::CmdResult MainProgram::cmd_route_shortest_distance(std::ostream& output, MatchIter begin, MatchIter end)
{
    string fromxstr = *begin++;
    string fromystr = *begin++;
    string toxstr = *begin++;
    string toystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    Coord fromxy = {convert_string_to<int>(fromxstr),convert_string_to<int>(fromystr)};
    Coord toxy = {convert_string_to<int>(toxstr),convert_string_to<int>(toystr)};

    auto steps = ds_.route_shortest_distance(fromxy, toxy);

    vector<tuple<Coord, Coord, WayID, Distance>> result;

    if (steps.empty())
    {
        output << "No route found!" << endl;
    }
    else if (steps.front() == make_tuple(NO_COORD, NO_WAY, NO_DISTANCE))
    {
        output << "Starting or destination coord has no ways!" << endl;
    }
    else
    {
        auto [coord, wayid, dist] = steps.front();
        for (auto iter = steps.begin()+1; iter != steps.end(); ++iter)
        {
            auto& [ncoord, nwayid, ndist] = *iter;
            result.emplace_back(coord, ncoord, wayid, dist);
            coord = ncoord; wayid = nwayid; dist = ndist;
        }
        result.emplace_back(coord, NO_COORD, NO_WAY, dist);
    }

    return {ResultType::ROUTE, result};
}

MainProgram::CmdResult MainProgram::cmd_route_with_cycle(std::ostream& output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    string fromxstr = *begin++;
    string fromystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    Coord fromxy = {convert_string_to<int>(fromxstr),convert_string_to<int>(fromystr)};

    auto steps = ds_.route_with_cycle(fromxy);

    if (steps.empty())
    {
        output << "No route found!" << endl;
        return {};
    }

    if (steps.front() == make_tuple(NO_COORD, NO_WAY/*, NO_DISTANCE*/))
    {
        output << "Starting coord has no ways!" << endl;
        return {};
    }

    if (steps.size() < 2)
    {
        output << "Too short route (" << steps.size() << ") to contain cycles!" << endl;
        return {};
    }

    auto lastcoord = std::get<0>(steps.back());
    auto cycbeg = std::find_if(steps.begin(), steps.end()-1, [lastcoord](auto const& e){ return std::get<0>(e) == lastcoord; });
    if (cycbeg == steps.end())
    {
        output << "No cycle found in returned route!";
        return {};
    }

    // If necessary, swap cycle so that it starts with smaller wayid
    if ((cycbeg+1) < (steps.end()-2))
    {
        auto wayfirst = std::get<1>(*cycbeg);
        auto waylast = std::get<1>(*(steps.end()-2));
        if (waylast < wayfirst)
        {
           std::reverse(cycbeg+1, steps.end()-1);
           // Rotate the wayids to fix the reversed order
           auto firstid = std::get<1>(*cycbeg);
           for (auto i = cycbeg; i != steps.end()-2; ++i)
           {
               std::get<1>(*i) = std::get<1>(*(i+1));
           }
           std::get<1>(*(steps.end()-2)) = firstid;
        }
    }

    vector<tuple<Coord, Coord, WayID, Distance>> result;
    // Don't put returned way ids or distances in the result
    auto [coord, wayid/*, dist*/] = steps.front();
    for (auto iter = steps.begin()+1; iter != steps.end(); ++iter)
    {
        auto& [ncoord, nwayid/*, ndist*/] = *iter;
        result.emplace_back(coord, ncoord, wayid, NO_DISTANCE);
        coord = ncoord; wayid = nwayid;/* dist = ndist;*/
    }
    result.emplace_back(coord, NO_COORD, NO_WAY, NO_DISTANCE);

    return {ResultType::ROUTE, result};
}

MainProgram::CmdResult MainProgram::cmd_trim_ways(std::ostream &output, MainProgram::MatchIter begin, MainProgram::MatchIter end)
{
    assert( begin == end && "Impossible number of parameters!");

    auto result = ds_.trim_ways();

    output << "The remaining ways have a total length of " << result << endl;

    view_dirty = true;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_random_add(std::ostream& output, MatchIter begin, MatchIter end)
{
    string sizestr = *begin++;
    string minxstr = *begin++;
    string minystr = *begin++;
    string maxxstr = *begin++;
    string maxystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    unsigned int size = convert_string_to<unsigned int>(sizestr);

    Coord def_min{1, 1};
    Coord def_max{100, 100};
    Coord min = def_min;
    Coord max = def_max;
    if (!minxstr.empty() && !minystr.empty() && !maxxstr.empty() && !maxystr.empty())
    {
        min.x = convert_string_to<unsigned int>(minxstr);
        min.y = convert_string_to<unsigned int>(minystr);
        max.x = convert_string_to<unsigned int>(maxxstr);
        max.y = convert_string_to<unsigned int>(maxystr);
    }
    else
    {
        auto places = ds_.all_places();
        if (!places.empty())
        {
            // Find out bounding box
            min = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
            max = {std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
            for (auto  place : places)
            {
                auto [x,y] = ds_.get_place_coord(place);
                if (x < min.x) { min.x = x; }
                if (y < min.y) { min.y = y; }
                if (x > max.x) { max.x = x; }
                if (y > max.y) { max.y = y; }
            }
        }
    }

    if (min == max)
    {
        min = def_min;
        max = def_max;
    }

    add_random_places_areas(size, nullptr, min, max);

    output << "Added: " << size << " places." << endl;

    view_dirty = true;

    return {};
}

void MainProgram::add_random_places_areas(unsigned int size, Stopwatch* watchp, Coord min, Coord max)
{
    for (unsigned int i = 0; i < size; ++i)
    {
        auto name = n_to_name(random_places_added_);
        PlaceID id = n_to_placeid(random_places_added_);
        PlaceType type{random(0, static_cast<int>(PlaceType::NO_TYPE))};

        int x = random<int>(min.x, max.x);
        int y = random<int>(min.y, max.y);

        if (watchp) { watchp->start(); }
        ds_.add_place(id, name, type, {x, y});
        if (watchp) { watchp->stop(); }

        // Add a new area for every 10 places
        if (random_places_added_ % 10 == 0)
        {
            auto areaid = n_to_areaid(random_areas_added_);
            vector<Coord> coords;
            for (int j=0; j<3; ++j)
            {
                coords.push_back({random<int>(min.x, max.x),random<int>(min.y, max.y)});
            }
            if (watchp) { watchp->start(); }
            ds_.add_area(areaid, convert_to_string(areaid), std::move(coords));
            if (watchp) { watchp->stop(); }
            // Add area as subarea so that we get a binary tree
            if (random_areas_added_ > 0)
            {
//                auto parentid = random<decltype(random_areas_added_)>(0, random_areas_added_);
                auto parentid = n_to_areaid(random_areas_added_ / 2);
                if (watchp) { watchp->start(); }
                ds_.add_subarea_to_area(areaid, parentid);
                if (watchp) { watchp->stop(); }
            }
            ++random_areas_added_;
        }

        ++random_places_added_;
    }
}

void MainProgram::add_random_ways(unsigned int n, Stopwatch *watchp)
{
    for (unsigned int i=0; i<n; ++i)
    {
        ++random_ways_added_;

        WayID id = n_to_wayid(random_ways_added_);
        Coord c1 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
        Coord c2 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
        if (c1.x != c2.x || c1.y != c2.y)
        {
            if (watchp) { watchp->start(); }
            ds_.add_way(id, {c1,c2});
            if (watchp) { watchp->stop(); }
        }
    }
}

MainProgram::CmdResult MainProgram::cmd_random_way_network(std::ostream& output, MatchIter begin, MatchIter end)
{
    string xroadstr = *begin++;
    string extrawaystr = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    unsigned int xroadnum = convert_string_to<unsigned int>(xroadstr);
    unsigned int extrawaynum = convert_string_to<unsigned int>(extrawaystr);

    create_random_way_network(output, xroadnum, extrawaynum);

    return {};
}

//
// Test commands
//

void MainProgram::test_random_add(Stopwatch& watch)
{
    add_random_places_areas(1, &watch);
}

void MainProgram::test_place_name_type(Stopwatch& watch)
{
    if (random_places_added_ > 0) // Don't do anything if there's no places
    {
        PlaceID id = n_to_placeid(random<decltype(random_places_added_)>(0, random_places_added_));
        watch.start();
        ds_.get_place_name_type(id);
        watch.stop();
    }
}

void MainProgram::test_place_coord(Stopwatch& watch)
{
    if (random_places_added_ > 0) // Don't do anything if there's no places
    {
        PlaceID id = n_to_placeid(random<decltype(random_places_added_)>(0, random_places_added_));
        watch.start();
        ds_.get_place_coord(id);
        watch.stop();
    }
}

void MainProgram::test_find_places_name(Stopwatch& watch)
{
    // Choose random number to remove
    if (random_places_added_ > 0) // Don't find if there's nothing to find
    {
        auto name = n_to_name(random<decltype(random_places_added_)>(0, random_places_added_));
        watch.start();
        ds_.find_places_name(name);
        watch.stop();
    }
}

void MainProgram::test_find_places_type(Stopwatch& watch)
{
    // Choose random number to remove
    if (random_places_added_ > 0) // Don't find if there's nothing to find
    {
        PlaceType type{random<int>(0, static_cast<int>(PlaceType::NO_TYPE))};
        watch.start();
        ds_.find_places_type(type);
        watch.stop();
    }
}

void MainProgram::test_change_place_name(Stopwatch& watch)
{
  if (random_places_added_ > 0) // Don't do anything if there's no places
  {
      PlaceID id = n_to_placeid(random<decltype(random_places_added_)>(0, random_places_added_));
      auto newname = n_to_name(random<decltype(random_places_added_)>(0, random_places_added_));
      watch.start();
      ds_.change_place_name(id, newname);
      watch.stop();
  }
}

void MainProgram::test_area_name(Stopwatch& watch)
{
    if (random_areas_added_ > 0)
    {
        auto id = n_to_areaid(random<decltype(random_areas_added_)>(0, random_areas_added_));
        watch.start();
        ds_.get_area_name(id);
        watch.stop();
    }
}

void MainProgram::test_subarea_in_areas(Stopwatch& watch)
{
    if (random_areas_added_ > 0) // Don't do anything if there's no places
    {
        auto id = n_to_areaid(random<decltype(random_areas_added_)>(0, random_areas_added_));
        watch.start();
        ds_.subarea_in_areas(id);
        watch.stop();
    }
}

void MainProgram::test_all_subareas_in_area(Stopwatch& watch)
{
    if (random_areas_added_ > 0) // Don't do anything if there's no places
    {
        auto id = n_to_areaid(random<decltype(random_areas_added_)>(0, random_areas_added_));
        watch.start();
        ds_.all_subareas_in_area(id);
        watch.stop();
    }
}

void MainProgram::test_places_closest_to(Stopwatch& watch)
{
    if (random_places_added_ > 0) // Don't do anything if there's no places
    {
        auto x = random<int>(0, 1000);
        auto y = random<int>(0, 1000);
        PlaceType type{random(0, static_cast<int>(PlaceType::NO_TYPE))};
        watch.start();
        ds_.places_closest_to({x,y}, type);
        watch.stop();
    }
}

void MainProgram::test_remove_place(Stopwatch& watch)
{
    // Choose random number to remove
    if (random_places_added_ > 0) // Don't remove if there's nothing to remove
    {
        auto id = n_to_placeid(random<decltype(random_places_added_)>(0, random_places_added_));
        watch.start();
        ds_.remove_place(id);
        watch.stop();
    }
}

void MainProgram::test_common_area_of_subareas(Stopwatch& watch)
{
    if (random_areas_added_ > 0) // Don't do anything if there's no places
    {
        auto id1 = n_to_areaid(random<decltype(random_areas_added_)>(0, random_areas_added_));
        auto id2 = n_to_areaid(random<decltype(random_areas_added_)>(0, random_areas_added_));
        watch.start();
        ds_.common_area_of_subareas(id1, id2);
        watch.stop();
    }
}

void MainProgram::test_ways_from(Stopwatch& watch)
{
    if (random_ways_added_ > 0) // Don't do anything if there's no ways
    {
         auto coord = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
        watch.start();
        ds_.ways_from(coord);
         watch.stop();
    }
}

void MainProgram::test_way_coords(Stopwatch& watch)
{
    if (random_ways_added_ > 0)
    {
        WayID id = n_to_wayid(random<decltype(random_ways_added_)>(0, random_ways_added_));
        watch.start();
        ds_.get_way_coords(id);
        watch.stop();
    }
}

void MainProgram::test_remove_way(Stopwatch& watch)
{
    if (random_ways_added_ > 0)
    {
        WayID id = n_to_wayid(random<decltype(random_ways_added_)>(0, random_ways_added_));
        watch.start();
        ds_.remove_way(id);
        watch.stop();
    }
}

void MainProgram::test_route_any(Stopwatch& watch)
{
    // Choose two random places
    Coord coord1 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
    Coord coord2 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));

    watch.start();
    ds_.route_any(coord1, coord2);
    watch.stop();
}

void MainProgram::test_route_least_crossroads(Stopwatch& watch)
{
    // Choose two random places
    Coord coord1 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
    Coord coord2 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));

    watch.start();
    ds_.route_least_crossroads(coord1, coord2);
    watch.stop();
}

void MainProgram::test_route_shortest_distance(Stopwatch& watch)
{
    // Choose two random places
    Coord coord1 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));
    Coord coord2 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));

    watch.start();
    ds_.route_shortest_distance(coord1, coord2);
    watch.stop();
}

void MainProgram::test_route_with_cycle(Stopwatch& watch)
{
    // Choose two random places
    Coord coord1 = n_to_coord(random(decltype(random_ways_added_)(0),random_ways_added_));

    watch.start();
    ds_.route_with_cycle(coord1);
    watch.stop();
}

void MainProgram::test_trim_ways(Stopwatch& watch)
{
    watch.start();
    ds_.trim_ways();
    watch.stop();
}

// Can be used in the perftest command to skip the actual command (only test adding)
void MainProgram::test_comment(Stopwatch& /*watch*/)
{
}



MainProgram::CmdResult MainProgram::cmd_randseed(std::ostream& output, MatchIter begin, MatchIter end)
{
    string seedstr = *begin++;
    assert(begin == end && "Invalid number of parameters");

    unsigned long int seed = convert_string_to<unsigned long int>(seedstr);

    rand_engine_.seed(seed);
    init_primes();

    output << "Random seed set to " << seed << endl;

    return {};
}

MainProgram::CmdResult MainProgram::cmd_read(std::ostream& output, MatchIter begin, MatchIter end)
{
    string filename = *begin++;
    string silentstr =  *begin++;
    assert( begin == end && "Impossible number of parameters!");

    bool silent = !silentstr.empty();
    ostream* new_output = &output;

    ostringstream dummystr; // Given as output if "silent" is specified, the output is discarded
    if (silent)
    {
        new_output = &dummystr;
    }

    ifstream input(filename);
    if (input)
    {
        output << "** Commands from '" << filename << "'" << endl;
        command_parser(input, *new_output, PromptStyle::NORMAL);
        if (silent) { output << "...(output discarded in silent mode)..." << endl; }
        output << "** End of commands from '" << filename << "'" << endl;
    }
    else
    {
        output << "Cannot open file '" << filename << "'!" << endl;
    }

    return {};
}


MainProgram::CmdResult MainProgram::cmd_testread(std::ostream& output, MatchIter begin, MatchIter end)
{
    string infilename = *begin++;
    string outfilename = *begin++;
    assert( begin == end && "Impossible number of parameters!");

    ifstream input(infilename);
    if (input)
    {
        ifstream expected_output(outfilename);
        if (output)
        {
            stringstream actual_output;
            command_parser(input, actual_output, PromptStyle::NO_NESTING);

            vector<string> actual_lines;
            while (actual_output)
            {
                string line;
                getline(actual_output, line);
                if (!actual_output) { break; }
                actual_lines.push_back(line);
            }

            vector<string> expected_lines;
            while (expected_output)
            {
                string line;
                getline(expected_output, line);
                if (!expected_output) { break; }
                expected_lines.push_back(line);
            }

            string heading_actual = "Actual output";
            unsigned int actual_max_length = heading_actual.length();
            auto actual_max_iter = max_element(actual_lines.cbegin(), actual_lines.cend(),
                                               [](string s1, string s2){ return s1.length() < s2.length(); });
            if (actual_max_iter != actual_lines.cend())
            {
                actual_max_length = actual_max_iter->length();
            }

            string heading_expected = "Expected output";
            unsigned int expected_max_length = heading_expected.length();
            auto expected_max_iter = max_element(expected_lines.cbegin(), expected_lines.cend(),
                                                    [](string s1, string s2){ return s1.length() < s2.length(); });
            if (expected_max_iter != expected_lines.cend())
            {
                expected_max_length = expected_max_iter->length();
            }

            // New diff, algorithm converted from https://github.com/alexdzyoba/diff
            size_t expsize = expected_lines.size(), actsize = actual_lines.size();
            vector<vector<int>> c(expsize + 1, vector<int>(actsize + 1, 0));

            // Create LCS length matrix
            for (size_t i = 1; i <= expsize; ++i)
            {
                for (size_t j = 1; j <= actsize; ++j)
                {
                    if (expected_lines[i - 1] == actual_lines[j - 1])
                    {
                        c[i][j] = 1 + c[i - 1][j - 1];
                    }
                    else
                    {
                        c[i][j] = max(c[i - 1][j], c[i][j - 1]);
                    }
                }
            }

            size_t i = expsize, j = actsize;
            vector<tuple<char, string, string>> result;

            while (i > 0 || j > 0)
            {
                if (i > 0 && j > 0 && expected_lines[i - 1] == actual_lines[j - 1])
                {
                    result.emplace_back(' ', expected_lines[i - 1], actual_lines[j - 1]);
                    --i; --j;
                }
                else if (i > 0 && j > 0 && c[i][j] == c[i - 1][j - 1])
                {
                    // Lines differ but not part of LCS
                    result.emplace_back('*', expected_lines[i - 1], actual_lines[j - 1]);
                    --i; --j;
                }
                else if (j > 0 && (i == 0 || c[i][j - 1] >= c[i - 1][j]))
                {
                    result.emplace_back('+', "", actual_lines[j - 1]);
                    --j;
                }
                else
                {
                    result.emplace_back('-', expected_lines[i - 1], "");
                    --i;
                }
            }

            reverse(result.begin(), result.end());

            bool lines_ok = all_of(result.begin(), result.end(), [](auto const& res){ return std::get<0>(res) == ' ';});

            if (!lines_ok)
            {
                output << "Lines beginning with:" << endl;
                output << "  '+' are extra lines in actual output (not found in expected output)" << endl;
                output << "  '-' are missing lines in actual output (found in expected output)" << endl;
                output << "  '*' are different lines in actual output (compared to expected output)" << endl;
                output << "  " << heading_actual << string(actual_max_length - heading_actual.length(), ' ') << " | " << heading_expected << endl;
                output << "--" << string(actual_max_length, '-') << "-|-" << string(expected_max_length, '-') << endl;
                for (auto const& [diff, expected, actual] : result)
                {
                    output << diff << ' ' << actual << string(actual_max_length - actual.length(), ' ')
                           << " | " << expected << endl;
                }
                output << "**Differences found! (Lines beginning with '+', '-', or '*')**" << endl;
                test_status_ = TestStatus::DIFFS_FOUND;
            }
            else
            {
                output << "**No differences in output.**" << endl;
                if (test_status_ == TestStatus::NOT_RUN)
                {
                    test_status_ = TestStatus::NO_DIFFS;
                }
            }

            std::string commandstr = std::string("testread \"")+infilename+"\" \""+outfilename+"\"";
            test_history_.push_back({commandstr,lines_ok});
        }
        else
        {
            output << "Cannot open file '" << outfilename << "'!" << endl;
        }
    }
    else
    {
        output << "Cannot open file '" << infilename << "'!" << endl;
    }

    return {};
}

MainProgram::CmdResult MainProgram::cmd_stopwatch(std::ostream& output, MatchIter begin, MatchIter end)
{
    string on = *begin++;
    string off = *begin++;
    string next = *begin++;
    assert(begin == end && "Invalid number of parameters");

    if (!on.empty())
    {
        stopwatch_mode = StopwatchMode::ON;
        output << "Stopwatch: on" << endl;
    }
    else if (!off.empty())
    {
        stopwatch_mode = StopwatchMode::OFF;
        output << "Stopwatch: off" << endl;
    }
    else if (!next.empty())
    {
        stopwatch_mode = StopwatchMode::NEXT;
        output << "Stopwatch: on for the next command" << endl;
    }
    else
    {
        assert(!"Impossible stopwatch mode!");
    }

    return {};
}


string const plcidx = "([0-9]+)";
string const areaidx = "([0-9]+)";
string const wayidx = "([a-zA-Z0-9]+)";
string const namex = "'([a-zA-Z0-9 -]+)'";
string const typex = "([a-zA-Z0-9]+)";
string const numx = "([0-9]+)";
string const optcoordx = "\\([[:space:]]*[0-9]+[[:space:]]*,[[:space:]]*[0-9]+[[:space:]]*\\)";
string const coordx = "\\([[:space:]]*([0-9]+)[[:space:]]*,[[:space:]]*([0-9]+)[[:space:]]*\\)";
string const wsx = "[[:space:]]+";
string const cmdx = "[0-9a-zA-Z_#]+";

vector<MainProgram::CmdInfo> MainProgram::cmds_ =
{
    {"add_place", "ID 'Name' Type (x,y)", plcidx+wsx+namex+wsx+typex+wsx+coordx, &MainProgram::cmd_add_place, nullptr },
    {"random_add", "number_of_places_to_add  [(minx,miny) (maxx,maxy)] (coordinates optional)", numx+"(?:"+wsx+coordx+wsx+coordx+")?",
     &MainProgram::cmd_random_add, &MainProgram::test_random_add },
    {"all_places", "", "", &MainProgram::cmd_all_places, nullptr },
    {"place_name_type", "ID", plcidx, &MainProgram::cmd_place_name_type, &MainProgram::test_place_name_type },
    {"place_coord", "ID", plcidx, &MainProgram::cmd_place_coord, &MainProgram::test_place_coord },
    {"add_area", "ID Name (x,y) (x,y)...", areaidx+wsx+namex+"((?:"+wsx+optcoordx+")+)", &MainProgram::cmd_add_area, nullptr },
    {"all_areas", "", "", &MainProgram::cmd_all_areas, nullptr },
    {"area_name", "AreaID", areaidx, &MainProgram::cmd_area_name, &MainProgram::test_area_name },
    {"area_coords", "AreaID", areaidx, &MainProgram::cmd_area_coords, nullptr },
    {"place_count", "", "", &MainProgram::cmd_place_count, nullptr },
    {"clear_all", "", "", &MainProgram::cmd_clear_all, nullptr },
    {"places_alphabetically", "", "", &MainProgram::NoParPlaceListCmd<&Datastructures::places_alphabetically>, &MainProgram::NoParPlaceListTestCmd<&Datastructures::places_alphabetically> },
    {"places_coord_order", "", "", &MainProgram::NoParPlaceListCmd<&Datastructures::places_coord_order>, &MainProgram::NoParPlaceListTestCmd<&Datastructures::places_coord_order> },
    {"places_closest_to", "Coord [type] (type optional)", coordx+"(?:"+wsx+typex+")?", &MainProgram::cmd_places_closest_to, &MainProgram::test_places_closest_to },
    {"common_area_of_subareas", "ID1 ID2", plcidx+wsx+plcidx, &MainProgram::cmd_common_area_of_subareas, &MainProgram::test_common_area_of_subareas },
    {"remove_place", "ID", plcidx, &MainProgram::cmd_remove_place, &MainProgram::test_remove_place },
    {"find_places_name", "'Name'", namex, &MainProgram::cmd_find_places_name, &MainProgram::test_find_places_name },
    {"find_places_type", "type", typex, &MainProgram::cmd_find_places_type, &MainProgram::test_find_places_type },
    {"change_place_name", "ID 'Newname'", plcidx+wsx+namex, &MainProgram::cmd_change_place_name, &MainProgram::test_change_place_name },
    {"add_subarea_to_area", "SubareaID AreaID", areaidx+wsx+areaidx, &MainProgram::cmd_add_subarea_to_area, nullptr },
    {"all_ways", "", "", &MainProgram::cmd_all_ways, nullptr },
    {"add_way", "WayID (x,y) (x,y)...", wayidx+"((?:"+wsx+optcoordx+")+)", &MainProgram::cmd_add_way, nullptr },
    {"random_way_network", "max_number_of_extra_points number_of_extra_ways", numx+wsx+numx, &MainProgram::cmd_random_way_network, nullptr },
    {"way_coords", "WayID", wayidx, &MainProgram::cmd_way_coords, &MainProgram::test_way_coords },
    {"ways_from", "Coord", coordx, &MainProgram::cmd_ways_from, &MainProgram::test_ways_from },
    {"clear_ways", "", "", &MainProgram::cmd_clear_ways, nullptr },
    {"remove_way", "WayID", wayidx, &MainProgram::cmd_remove_way, &MainProgram::test_remove_way },
    {"subarea_in_areas", "AreaID", areaidx, &MainProgram::cmd_subarea_in_areas, &MainProgram::test_subarea_in_areas },
    {"all_subareas_in_area", "AreaID", areaidx, &MainProgram::cmd_all_subareas_in_area, &MainProgram::test_all_subareas_in_area },
    {"route_any", "CoordFrom CoordTo", coordx+wsx+coordx, &MainProgram::cmd_route_any, &MainProgram::test_route_any },
    {"route_least_crossroads", "CoordFrom CoordTo", coordx+wsx+coordx, &MainProgram::cmd_route_least_crossroads, &MainProgram::test_route_least_crossroads },
    {"route_shortest_distance", "CoordFrom CoordTo", coordx+wsx+coordx, &MainProgram::cmd_route_shortest_distance, &MainProgram::test_route_shortest_distance },
    {"route_with_cycle", "Coordfrom", coordx, &MainProgram::cmd_route_with_cycle, &MainProgram::test_route_with_cycle },
    {"trim_ways", "", "", &MainProgram::cmd_trim_ways, &MainProgram::test_trim_ways },
    {"quit", "", "", nullptr, nullptr },
    {"help", "", "", &MainProgram::help_command, nullptr },
    {"read", "\"in-filename\" [silent]", "\"([-a-zA-Z0-9 ./:_]+)\"(?:"+wsx+"(silent))?", &MainProgram::cmd_read, nullptr },
    {"testread", "\"in-filename\" \"out-filename\"", "\"([-a-zA-Z0-9 ./:_]+)\""+wsx+"\"([-a-zA-Z0-9 ./:_]+)\"", &MainProgram::cmd_testread, nullptr },
    {"perftest", "cmd1[;cmd2...][;extra_add] timeout repeat_count n1[;n2...] (parts in [] are optional, alternatives separated by |)",
     "("+cmdx+"(?:;"+cmdx+")*)"+wsx+numx+wsx+numx+wsx+"([0-9]+(?:;[0-9]+)*)", &MainProgram::cmd_perftest, nullptr },
    {"stopwatch", "on|off|next (alternatives separated by |)", "(?:(on)|(off)|(next))", &MainProgram::cmd_stopwatch, nullptr },
    {"random_seed", "new-random-seed-integer", numx, &MainProgram::cmd_randseed, nullptr },
    {"#", "comment text", ".*", &MainProgram::cmd_comment, &MainProgram::test_comment },
    // {"watchtest", "iterations", numx, &MainProgram::cmd_watchtest, nullptr },
};

void MainProgram::init_cmds()
{
    // Sort commands in alphabetical order
    sort(cmds_.begin(), cmds_.end(), [](auto const& l, auto const& r){ return l.cmd < r.cmd; });
}

// MainProgram::CmdResult MainProgram::cmd_watchtest(std::ostream &output, MatchIter begin, MatchIter end)
// {
//     unsigned int iterations = convert_string_to<unsigned int>(*begin++);
//     assert(begin == end && "Invalid number of parameters");

//     Stopwatch watch(true);
//     for (auto i=0u; i<iterations; ++i)
//     {
//         watch.start();
//         watch.stop();
//     }
//     output << "Stopwatch overhead after " << iterations << " empty iterations: " << endl
//            << "    " << watch.elapsed() << " s, " << watch.elapsed()/iterations << " s/iteration," << endl
//            << "    " << watch.count() << " operations, " << watch.count()/iterations << " operations/iteration" << endl;

//     return {};
// }

MainProgram::CmdResult MainProgram::help_command(std::ostream& output, MatchIter /*begin*/, MatchIter /*end*/)
{
    output << "Commands:" << endl;
    for (auto& i : cmds_)
    {
        output << "  " << i.cmd << " " << i.info << endl;
    }

    return {};
}

MainProgram::CmdResult MainProgram::cmd_perftest(std::ostream& output, MatchIter begin, MatchIter end)
{
#ifdef _GLIBCXX_DEBUG
    output << "WARNING: Debug STL enabled, performance will be worse than expected (maybe also asymptotically)!" << endl;
#endif // _GLIBCXX_DEBUG

    try {
    // Note: everything below is indented too little by one indentation level! (because of try block above)

    vector<string> graph_commands({"add_way", "random_ways", "way_coords", "ways_from", "remove_way",
                                  "route_any", "route_least_crossroads", "route_shortest_distance", "route_with_cycle", "trim_ways"});

    string commandstr = *begin++;
    unsigned int timeout = convert_string_to<unsigned int>(*begin++);
    unsigned int repeat_count = convert_string_to<unsigned int>(*begin++);
    string sizes = *begin++;
    assert(begin == end && "Invalid number of parameters");

    vector<string> testcmds;
    bool extra_add = false;
    bool graph_add = false;
    smatch scmd;
    auto cbeg = commandstr.cbegin();
    auto cend = commandstr.cend();
    for ( ; regex_search(cbeg, cend, scmd, commands_regex_); cbeg = scmd.suffix().first)
    {
        testcmds.push_back(scmd[1]);
        if (find(graph_commands.begin(), graph_commands.end(), scmd[1]) != graph_commands.end())
        {
            graph_add = true;
        }
    }

    auto extra_add_pos = find(testcmds.begin(), testcmds.end(), "extra_add");
    if (extra_add_pos != testcmds.end())
    {
        extra_add = true;
        testcmds.erase(extra_add_pos);
    }

    // Initialize test functions
    vector<void(MainProgram::*)(Stopwatch& watch)> testfuncs;
    output << "Testing commands: ";
    for (auto& i : testcmds)
    {
        auto pos = find_if(cmds_.begin(), cmds_.end(), [&i](auto const& cmd){ return cmd.cmd == i; });
        if (pos != cmds_.end() && pos->testfunc)
        {
            output << i << " ";
            testfuncs.push_back(pos->testfunc);
        }
        else
        {
            output << "(cannot test " << i << ") ";
        }
    }
    output << endl;
    if (graph_add) { output << "Adding also random ways (included in the 'add' column)" << endl; }
    if (extra_add) { output << "Doing 1 random add between commands (not included in measurements)" << endl; }
    output << endl;

    vector<unsigned int> init_ns;
    smatch size;
    auto sbeg = sizes.cbegin();
    auto send = sizes.cend();
    for ( ; regex_search(sbeg, send, size, sizes_regex_); sbeg = size.suffix().first)
    {
        init_ns.push_back(convert_string_to<unsigned int>(size[1]));
    }

    output << "Timeout for each N is " << timeout << " sec. " << endl;
    output << "For each N perform " << repeat_count << " random command(s) from:" << endl;

    if (testfuncs.empty())
    {
        output << "No commands to test!" << endl;
        return {};
    }

#ifdef USE_PERF_EVENT
    output << setw(7) << "N" << ", " << setw(12) << "add (sec)" << ", " << setw(12) << "add (count)" << ", " << setw(12) << "cmds (sec)" << ", "
           << setw(12) << "cmds (count)"  << ", " << setw(12) << "total (sec)" << ", " << setw(12) << "total (count)" << endl;
#else
    output << setw(7) << "N" << ", " << setw(12) << "add (sec)" << ", " << setw(12) << "cmds (sec)" << ", "
           << setw(12) << "total (sec)" << endl;
#endif
    flush_output(output);

    auto stop = false;
    for (unsigned int n : init_ns)
    {
        if (stop) { break; }

        output << setw(7) << n << ", " << flush;

        ds_.clear_all();
        init_primes();

        Stopwatch stopwatch(true); // Stopwatch for actual testing, use also instruction counting, if enabled
        Stopwatch towatch; // Stopwatch for detecting timeouts, no instruction counting
        towatch.start();

        // Add random places (+ areas)
        for (unsigned int i = 0; i < n / 1000; ++i)
        {
            add_random_places_areas(1000, &stopwatch);

            if (towatch.elapsed() >= timeout)
            {
                output << "Timeout!" << endl;
                stop = true;
                break;
            }
            if (check_stop())
            {
                output << "Stopped!" << endl;
                stop = true;
                break;
            }
        }
        if (stop) { break; }

        if (n % 1000 != 0)
        {
            add_random_places_areas(n % 1000, &stopwatch);
        }

        // Add random ways
        if (graph_add)
        {
            for (unsigned int i = 0; i < n / 1000; ++i)
            {
                add_random_ways(1000, &stopwatch);

                if (towatch.elapsed() >= timeout)
                {
                    output << "Timeout!" << endl;
                    stop = true;
                    break;
                }
                if (check_stop())
                {
                    output << "Stopped!" << endl;
                    stop = true;
                    break;
                }
            }
            if (stop) { break; }

            if (n % 1000 != 0)
            {
                add_random_ways(n % 1000, &stopwatch);
            }
        }

#ifdef USE_PERF_EVENT
        auto addcount = stopwatch.count();
#endif
        auto addsec = stopwatch.elapsed();

#ifdef USE_PERF_EVENT
        output << setw(12) << addsec << ", " << setw(12) << addcount << ", " << flush;
#else
        output << setw(12) << addsec << ", " << flush;
#endif

        for (unsigned int repeat = 0; repeat < repeat_count; ++repeat)
        {
            auto cmdpos = random(testfuncs.begin(), testfuncs.end());

            (this->**cmdpos)(stopwatch);
            if (extra_add)
            {
                add_random_places_areas(1, &stopwatch);
            }

            if (repeat % 10 == 0)
            {
                if (towatch.elapsed() >= timeout)
                {
                    output << "Timeout!" << endl;
                    stop = true;
                    break;
                }
                if (check_stop())
                {
                    output << "Stopped!" << endl;
                    stop = true;
                    break;
                }
            }
        }
        if (stop) { break; }

#ifdef USE_PERF_EVENT
        auto totalcount = stopwatch.count();
#endif
        auto totalsec = stopwatch.elapsed();

#ifdef USE_PERF_EVENT
        output << setw(12) << totalsec-addsec << ", " << setw(12) << totalcount-addcount << ", " << setw(12) << totalsec << ", " << setw(12) << totalcount;
#else
        output << setw(12) << totalsec-addsec << ", " << setw(12) << totalsec;
#endif

//        unsigned long int maxmem;
//        string unit;
//        auto [maxmem, unit] = mempeak();
//        maxmem -=  startmem;
//        if (maxmem != 0)
//        {
//            output << ", memory " << maxmem << " " << unit;
//        }
        output << endl;
        flush_output(output);
    }

    output << "Perftest complete." << endl;

    ds_.clear_all();
    init_primes();

    }
    catch (NotImplemented const&)
    {
        // Clean up after NotImplemented
        ds_.clear_all();
        init_primes();
        throw;
    }

#ifdef _GLIBCXX_DEBUG
    output << "WARNING: Debug STL enabled, performance will be worse than expected (maybe also asymptotically)!" << endl;
#endif // _GLIBCXX_DEBUG

    return {};
}

MainProgram::CmdResult MainProgram::cmd_comment(std::ostream& /*output*/, MatchIter /*begin*/, MatchIter /*end*/)
{
    return {};
}

bool MainProgram::command_parse_line(string inputline, ostream& output)
{
//    static unsigned int nesting_level = 0; // UGLY! Remember nesting level to print correct amount of >:s.
//    if (promptstyle != PromptStyle::NO_NESTING) { ++nesting_level; }

    if (inputline.empty()) { return true; }

    smatch match;
    bool matched = regex_match(inputline, match, cmds_regex_);
    if (matched)
    {
        assert(match.size() == 3);
        string cmd = match[1];
        string params = match[2];

        auto pos = find_if(cmds_.begin(), cmds_.end(), [cmd](CmdInfo const& ci) { return ci.cmd == cmd; });
        assert(pos != cmds_.end());

        smatch match2;
        bool matched2 = regex_match(params, match2, pos->param_regex);
        if (matched2)
        {
            if (pos->func)
            {
                assert(!match2.empty());

                Stopwatch stopwatch;
                bool use_stopwatch = (stopwatch_mode != StopwatchMode::OFF);
                // Reset stopwatch mode if only for the next command
                if (stopwatch_mode == StopwatchMode::NEXT) { stopwatch_mode = StopwatchMode::OFF; }

                if (use_stopwatch)
                {
                    stopwatch.start();
                }

                CmdResult result;
                try
                {
                    result = (this->*(pos->func))(output, ++(match2.begin()), match2.end());
                }
                catch (NotImplemented const& e)
                {
                    output << endl << "NotImplemented from cmd " << pos->cmd << " : " << e.what() << endl;
                    cerr << endl << "NotImplemented from cmd " << pos->cmd << " : " << e.what() << endl;
                }

                if (use_stopwatch)
                {
                    stopwatch.stop();
                }

                switch (result.first)
                {
                    case ResultType::NOTHING:
                    {
                        break;
                    }
                    case ResultType::PLACEIDLIST:
                    {
                        auto& [area, places] = std::get<CmdResultPlaceIDs>(result.second);
                        if (area != NO_AREA)
                        {
                            output << "Area: ";
                            print_area(area, output);
                        }
                        if (!places.empty())
                        {
                            if (places.size() == 1 && places.front() == NO_PLACE)
                            {
                                output << "Failed (NO_... returned)!!" << std::endl;
                            }
                            else
                            {
                                unsigned int num = 0;
                                for (PlaceID id : places)
                                {
                                    ++num;
                                    if (places.size() > 1) { output << num << ". "; }
                                    print_place(id, output);
                                }
                            }
                        }
                        break;
                    }
                    case ResultType::AREAIDLIST:
                    {
                        auto& areas = std::get<CmdResultAreaIDs>(result.second);
                        if (!areas.empty())
                        {
                            if (areas.size() == 1 && areas.front() == NO_AREA)
                            {
                                output << "Failed (NO_... returned)!!" << std::endl;
                            }
                            else
                            {
                                unsigned int num = 0;
                                for (auto area : areas)
                                {
                                    ++num;
                                    if (areas.size() > 1) { output << num << ". "; }
                                    print_area(area, output);
                                }
                            }
                        }
                        break;
                    }
                    case ResultType::WAYS:
                    {
                        auto& ways = std::get<CmdResultRoute>(result.second);
                        if (!ways.empty())
                        {
                            if (ways.size() == 1 && get<0>(ways.front()) == NO_COORD)
                            {
                                output << "Failed (NO_... returned)!!" << std::endl;
                            }
                            else
                            {
                                unsigned int num = 1;
                                for (auto& [fromcoord, tocoord, wayid, distance] : ways)
                                {
                                    output << num << ". ";
                                    ++num;
                                    print_coord(tocoord, output, false);
                                    if (wayid != NO_WAY) { output << " way " << wayid << " "; }
                                    if (distance != NO_DISTANCE) { output << "distance " << distance; }
                                    output << endl;
                                }
                            }
                        }
                        break;
                    }
                    case ResultType::ROUTE:
                    {
                        auto& route = std::get<CmdResultRoute>(result.second);
                        if (!route.empty())
                        {
                            if (route.size() == 1 && get<0>(route.front()) == NO_COORD)
                            {
                                output << "Failed (NO_... returned)!!" << std::endl;
                            }
                            else
                            {
                                unsigned int num = 1;
                                for (auto& [coord, nextcoord, wayid, distance] : route)
                                {
                                    output << num << ". ";
                                    ++num;
                                    print_coord(coord, output, false);
                                    if (wayid != NO_WAY) { output << " way " << wayid; }
                                    if (distance != NO_DISTANCE) { output << " distance " << distance; }
                                    output << endl;
                                }
                            }
                        }
                        break;
                    }
                    // case ResultType::CYCLE:
                    // {
                    //     auto& res = get<CmdResultCoords>(result.second);
                    //     if (!res.empty())
                    //     {
                    //         if (res.size() == 1 && res.front() == NO_COORD)
                    //         {
                    //             output << "Failed (...NO_COORD... returned)!" << endl;
                    //         }
                    //         else
                    //         {
                    //             unsigned int num = 0;
                    //             if (res.size() < 2)
                    //             {
                    //                 output << "Too small path for cycle!";
                    //             }
                    //             else
                    //             {
                    //                 auto cycbeg = find(res.begin(), res.end()-1, res.back());
                    //                 if (cycbeg == res.end())
                    //                 {
                    //                     output << "No cycle found in path!";
                    //                 }
                    //                 else
                    //                 {
                    //                     // Swap cycle so that it starts with smaller id
                    //                     if (((cycbeg+1) < (res.end()-2)) && (*(res.end()-2) < *(cycbeg+1)))
                    //                     {
                    //                         reverse(cycbeg+1, res.end()-1);
                    //                     }
                    //                     for (auto i = cycbeg; i != res.end(); ++i)
                    //                     {
                    //                         auto coord = *i;
                    //                         output << num << ". ";
                    //                         if (num > 0) { output << "-> "; }
                    //                         else { output << "   "; }
                    //                         print_coord(coord, output, false);
                    //                         output << endl;

                    //                         ++num;
                    //                     }
                    //                 }
                    //             }
                    //         }
                    //     }
                    //     break;
                    // }
                    default:
                    {
                        assert(false && "Unsupported result type!");
                    }
                }

                if (result != prev_result)
                {
                    prev_result = std::move(result);
                    view_dirty = true;
                }

                if (use_stopwatch)
                {
                    output << "Command '" << cmd << "': " << stopwatch.elapsed() << " sec" << endl;
                }
            }
            else
            { // No function to run = quit command
                return false;
            }
        }
        else
        {
            output << "Invalid parameters for command '" << cmd << "'!" << endl;
        }
    }
    else
    {
        output << "Unknown command!" << endl;
    }

    return true; // Signal continuing
}

void MainProgram::command_parser(istream& input, ostream& output, PromptStyle promptstyle)
{

    auto initial_status = test_status_;
    auto initial_history = test_history_;
    test_status_ = TestStatus::NOT_RUN;
    test_history_.clear();

    string line;
    do
    {
//        output << string(nesting_level, '>') << " ";
        output << PROMPT;
        getline(input, line, '\n');

        if (promptstyle != PromptStyle::NO_ECHO)
        {
            output << line << endl;
        }

        if (!input) { break; }

        bool cont = command_parse_line(line, output);
        view_dirty = false; // No need to keep track of individual result changes
        if (!cont) { break; }
    }
    while (input);
    //    if (promptstyle != PromptStyle::NO_NESTING) { --nesting_level; }

    if (!test_history_.empty())
    {
        output << "Testread-tests performed:" << endl;
        for (auto [cmd, ok] : test_history_)
        {
            output << "  " << (ok ? "Passed: " : "Failed: ") << cmd << endl;
        }
    }
    test_history_ = initial_history;

    if (test_status_ != TestStatus::NOT_RUN)
    {
        output << "Testread-tests have been run, " << ((test_status_ == TestStatus::DIFFS_FOUND) ? "differences found!" : "no differences found.") << endl;
    }
    if (test_status_ == TestStatus::NOT_RUN || (test_status_ == TestStatus::NO_DIFFS && initial_status == TestStatus::DIFFS_FOUND))
    {
        test_status_ = initial_status;
    }

    view_dirty = true; // To be safe, assume that results have been changed
}

void MainProgram::setui(MainWindow* ui)
{
    ui_ = ui;
}

#ifdef GRAPHICAL_GUI
void MainProgram::flush_output(std::ostream& output)
{
    if (ui_)
    {
        if (auto soutput = dynamic_cast<ostringstream*>(&output))
        {
            ui_->output_text(*soutput);
        }
    }
}
#else
void MainProgram::flush_output(std::ostream& /*output*/)
{
}
#endif

bool MainProgram::check_stop() const
{
#ifdef GRAPHICAL_GUI
    if (ui_)
    {
        return ui_->check_stop_pressed();
    }
#endif
    return false;
}

std::array<unsigned long int, 20> const MainProgram::primes1{4943,   4951,   4957,   4967,   4969,   4973,   4987,   4993,   4999,   5003,
                                                             5009,   5011,   5021,   5023,   5039,   5051,   5059,   5077,   5081,   5087};
std::array<unsigned long int, 20> const MainProgram::primes2{81031,  81041,  81043,  81047,  81049,  81071,  81077,  81083,  81097,  81101,
                                                             81119,  81131,  81157,  81163,  81173,  81181,  81197,  81199,  81203,  81223};

MainProgram::MainProgram()
{
    rand_engine_.seed(time(nullptr));

    //    startmem = get<0>(mempeak());

    init_primes();
    init_regexs();
    init_cmds();
}

int MainProgram::mainprogram(int argc, char* argv[])
{
    vector<string> args(argv, argv+argc);

    if (args.size() < 1 || args.size() > 3)
    {
        cerr << "Usage: " + ((args.size() > 0) ? args[0] : "<program name>") + " [<command file>|--console|--command command]" << endl;
        return EXIT_FAILURE;
    }

    MainProgram mainprg;

    if (args.size() == 2 && args[1] != "--console")
    {
        string filename = args[1];
        ifstream input(filename);
        if (input)
        {
            mainprg.command_parser(input, cout, MainProgram::PromptStyle::NORMAL);
        }
        else
        {
            cout << "Cannot open file '" << filename << "'!" << endl;
        }
    }
    else if (args.size() == 3 && args[1] == "--command")
    {
        string instr = args[2];
        istringstream input(instr);
        if (input)
        {
            mainprg.command_parser(input, cout, MainProgram::PromptStyle::NORMAL);
        }
        else
        {
            cout << "Cannot use '" << instr << "' as input!" << endl;
        }
    }
    else
    {
        mainprg.command_parser(cin, cout, MainProgram::PromptStyle::NO_ECHO);
    }

    cerr << "Program ended normally." << endl;
    if (mainprg.test_status_ == TestStatus::DIFFS_FOUND)
    {
        return EXIT_FAILURE;
    }
    else
    {
        return EXIT_SUCCESS;
    }
}

void MainProgram::init_primes()
{
    // Initialize id generator
    prime1_ = primes1[random<int>(0, primes1.size())];
    prime2_ = primes2[random<int>(0, primes2.size())];
    random_places_added_ = 0;
    random_areas_added_ = 0;
    random_ways_added_ = 0;
}

void MainProgram::create_random_way_network(std::ostream& output, int extrapoints, int extraways)
{
    // Info on all xroads
    struct XInfo
    {
        Coord xy;
        bool is_place;

        int component = -1;
        vector<unsigned long int> neighbours = {};

        bool operator<(XInfo right) const
        {
            if (xy.x < right.xy.x) { return true; }
            if (xy.x > right.xy.x) { return false; }
            if (xy.y < right.xy.y) { return true; }
            if (xy.y > right.xy.y) { return false; }
            return is_place && !right.is_place; // Sorts places first for equal coordinates, important for later unique
        }
        bool operator==(XInfo right) const
        {
            return xy.x == right.xy.x && xy.y == right.xy.y; // Ignore is_place
        }
    };
    vector<XInfo> tset;

    // Add all places as xroads, calculate min & max coords
    Coord def_min{1, 1};
    Coord def_max{100, 100};
    Coord min = def_min;
    Coord max = def_max;
    auto places = ds_.all_places();
    if (!places.empty())
    {
        // Find out bounding box
        min = {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()};
        max = {std::numeric_limits<int>::min(), std::numeric_limits<int>::min()};
        for (auto const& place : places)
        {
            auto [name, type] = ds_.get_place_name_type(place);
            if (type == PlaceType::FIREPIT || type == PlaceType::SHELTER || type == PlaceType::PARKING)
            {
                auto xy = ds_.get_place_coord(place);
                if (xy.x < min.x) { min.x = xy.x; }
                if (xy.y < min.y) { min.y = xy.y; }
                if (xy.x > max.x) { max.x = xy.x; }
                if (xy.y > max.y) { max.y = xy.y; }
                tset.push_back({xy, true});
            }
        }
    }

    if (min == max)
    {
        min = def_min;
        max = def_max;
    }

    // Add "extrapoints" new coordinates
    for (int i=0; i<extrapoints; ++i)
    {
        int x = random<int>(min.x, max.x);
        int y = random<int>(min.y, max.y);
        tset.push_back({{x,y},false});
    }

    // Remove duplicate xroads
    sort(tset.begin(), tset.end());
    tset.erase(unique(tset.begin(), tset.end()), tset.end());
    unsigned long int tsetn = tset.size();

    // A collection of all possible ways (indexed with combined xroad indices)
    vector<tuple<unsigned long int, unsigned long int, Distance>> ways;
    for (unsigned long int i1=0; i1<tset.size(); ++i1)
    {
        auto xy1 = tset[i1].xy;

        for (unsigned long int i2=i1+1; i2<tset.size(); ++i2)
        {
            auto xy2 = tset[i2].xy;
            auto dist = coorddist(xy1, xy2);

            ways.push_back({i1, i2, dist});
        }
    }

    // Shuffle ways first, then sort based on distance
    for (unsigned long int i1=0; i1<ways.size(); ++i1)
    {
        auto i2 = random<unsigned long int>(i1, ways.size());
        swap(ways[i1], ways[i2]);
    }
    sort(ways.begin(), ways.end(), [](auto l, auto r){ return get<2>(l) < get<2>(r); });

    int extras = extraways;
    bool connected = false;
    for(unsigned long int wi1=0; wi1<ways.size(); ++wi1)
    {
        auto [i1, i2, dist1] = ways[wi1];
        // Check if way intersects with an earlier way
        bool discard = false;
        for (unsigned long int wi2=0; wi2<wi1; ++wi2)
        {
            auto [i3, i4, dist2] = ways[wi2];
            if (doIntersect(tset[i1].xy, tset[i2].xy, tset[i3].xy, tset[i4].xy))
            {
                discard = true;
                break;
            }
        }
        if (discard) { continue; }

        if (!connected)
        {
            auto s1 = i1;
            while (tset[s1].component >= 0) { s1 = tset[s1].component; }
            auto s2 = i2;
            while (tset[s2].component >= 0) { s2 = tset[s2].component; }
            if (s1 == s2) { continue; } // Already connected
            tset[s2].component += tset[s1].component;
            tset[s1].component = s2;

            if (tset[s2].component == -static_cast<long int>(tsetn)) { connected = true; }
        }
        else // Adding extra ways
        {
            if (extras <= 0) { break; } // Connected and we've added "extras" extra ways already
            --extras;
        }

        // Add way to used ways
        tset[i1].neighbours.push_back(i2);
        tset[i2].neighbours.push_back(i1);
    }

    // Add ways
    ds_.clear_ways();
    unsigned long int waycount = 0;

    for (unsigned long int i1=0; i1<tset.size(); ++i1)
    {
        auto const &xinfo = tset[i1];

        // // Skip all non-places which have <3 neighbours (used at most as intermediate points in a way or discarded)
        // if (!xinfo.is_place && xinfo.neighbours.size() < 3) { continue; }
        // Skip all non-places which have 2 neighbours (as intermediate points in a way)
        if (!xinfo.is_place && xinfo.neighbours.size() == 2) { continue; }

        for (auto i2 : xinfo.neighbours)
        {
            vector<Coord> waycoords;
            waycoords.push_back(xinfo.xy);

            unsigned long int endidx = i2;
            unsigned long int previdx = i1;
            bool discard = false;
            while (true)
            {
                auto const& einfo = tset[endidx];
                if (einfo.is_place || einfo.neighbours.size() > 2) { break; } // Stop way at a place or crossroad
                if (einfo.neighbours.size() == 1)
                {
                    // discard = true; // Non-place dead-end: do not use
                    break;
                }
                // We have a non-place point with 2 neighbours: just a point in a way
                // Continue in the direction we did not come from
                assert(einfo.neighbours.size() == 2);
                waycoords.push_back(einfo.xy);
                if (einfo.neighbours[0] == previdx)
                {
                    previdx = endidx;
                    endidx = einfo.neighbours[1];
                }
                else
                {
                    assert(einfo.neighbours[1] == previdx);
                    previdx = endidx;
                    endidx = einfo.neighbours[0];
                }
            }

            if (!discard && xinfo < tset[endidx]) // Add only ways to bigger xinfo (to avoid duplicates)
            {
                waycoords.push_back(tset[endidx].xy);
                ds_.add_way("W"+convert_to_string(waycount), waycoords);
                ++waycount;
            }
        }
    }

    output << "Created a random way network with " << waycount << " ways." << std::endl;
}

Distance MainProgram::coorddist(Coord p1, Coord p2)
{
    long long int deltax = p2.x - p1.x;
    long long int deltay = p2.y - p1.y;

    long long int distsq = deltax*deltax + deltay*deltay;

    return static_cast<Distance>(std::sqrt(distsq));
}

// The functions below are taken and modified from https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// point q lies on line segment 'pr'
bool MainProgram::onSegment(Coord p, Coord q, Coord r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
       return true;

    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int MainProgram::orientation(Coord p, Coord q, Coord r)
{
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // colinear

    return (val > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool MainProgram::doIntersect(Coord p1, Coord q1, Coord p2, Coord q2)
{
    if ((p1 == p2 && q1 == q2) || (p1 == q2 && q1 == p2)) { return true; } // Same line
    if (p1 == p2 || p1 == q2 || q1 == p2 || q1 == q2) { return false; } // One same point, cannot intersect
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

std::string MainProgram::print_place(PlaceID id, ostream& output, bool nl)
{
    if (id != NO_PLACE)
    {
        auto [name,type] = ds_.get_place_name_type(id);
        auto xy = ds_.get_place_coord(id);
        if (!name.empty())
        {
            output << name << " (" << convert_placetype_to_string(type) << ")";
        }
        else
        {
            output << "*";
        }

         output << ": pos=";
         print_coord(xy, output, false);
         output << ", id=" << id;
         if (nl) { output << endl; }

        ostringstream retstream;
        retstream << id;
        return retstream.str();
    }
    else
    {
        output << "--NO_PLACE--";
        if (nl) { output << endl; }
        return "";
    }
}

std::string MainProgram::print_place_name(PlaceID id, std::ostream &output, bool nl)
{
    if (id != NO_PLACE)
    {
        auto [name,type] = ds_.get_place_name_type(id);
        if (!name.empty())
        {
            output << name;
        }
        else
        {
            output << "*";
        }

        ostringstream retstream;
        retstream << name;
        if (nl) { output << endl; }
        return retstream.str();
    }
    else
    {
        output << "--NO_PLACE--";
        if (nl) { output << endl; }
        return "";
    }
}

std::string MainProgram::print_area(AreaID id, std::ostream& output, bool nl)
{
    if (id != NO_AREA)
    {
        auto name = ds_.get_area_name(id);
        if (!name.empty())
        {
            output << name << ": id=" << id;
        }
        else
        {
            output << "*" << ": id=" << id;
        }
        ostringstream retstream;
        retstream << id;
        if (nl) { output << endl; }
        return retstream.str();
    }
    else
    {
        output << "--NO_AREA--";
        if (nl) { output << endl; }
        return "";
    }
}

std::string MainProgram::print_way(WayID id, std::ostream &output, bool nl)
{
    if (id != NO_WAY)
    {
        output << "Way id " << id;
        ostringstream retstream;
        retstream << id;
        if (nl) { output << endl; }
        return retstream.str();
    }
    else
    {
        output << "--NO_WAY--";
        if (nl) { output << endl; }
        return "";
    }
}

std::string MainProgram::print_coord(Coord coord, std::ostream& output, bool nl)
{
    if (coord != NO_COORD)
    {
        output << "(" << coord.x << "," << coord.y << ")";
        ostringstream retstream;
        retstream << "(" << coord.x << "," << coord.y << ")";
        if (nl) { output << endl; }
        return retstream.str();
    }
    else
    {
        output << "(--NO_COORD--)";
        if (nl) { output << endl; }
        return "";
    }
}

PlaceType MainProgram::convert_string_to_placetype(std::string from)
{
    std::istringstream istr(from);
    std::string typestr;
    istr >> std::noskipws >> typestr;
    if (istr.fail() || !istr.eof())
    {
        throw std::invalid_argument("Cannot convert string to place type");
    }

    PlaceType result = PlaceType::NO_TYPE;
    if (typestr == "firepit") { result = PlaceType::FIREPIT; }
    else if (typestr == "shelter") { result = PlaceType::SHELTER; }
    else if (typestr == "parking") { result = PlaceType::PARKING; }
    else if (typestr == "peak") { result = PlaceType::PEAK; }
    else if (typestr == "bay") { result = PlaceType::BAY; }
    else if (typestr == "area") { result = PlaceType::AREA; }
    else if (typestr == "other") { result = PlaceType::OTHER; }
    // else
    // {
    //     throw std::invalid_argument("Cannot convert string to place type");
    // }

    return result;
}

std::string MainProgram::convert_placetype_to_string(PlaceType type)
{
    switch (type)
    {
    case PlaceType::FIREPIT:
        return "firepit";
    case PlaceType::SHELTER:
        return "shelter";
    case PlaceType::PARKING:
        return "parking";
    case PlaceType::PEAK:
        return "peak";
    case PlaceType::BAY:
        return "bay";
    case PlaceType::AREA:
        return "area";
    case PlaceType::OTHER:
        return "other";
    default:
        return "!!no type!!";
    }
}

Name MainProgram::n_to_name(unsigned long n)
{
    unsigned long int hash = prime1_*n + prime2_;
    string name;

    while (hash > 0)
    {
        auto hexnum = hash % 26;
        hash /= 26;
        name.push_back('a'+hexnum);
    }

    return name;
}

AreaID MainProgram::n_to_areaid(unsigned long n)
{
    return n_to_placeid(n);
}

WayID MainProgram::n_to_wayid(unsigned long n)
{
 std::ostringstream ostr;
 ostr << "R" << n;
 return ostr.str();
}

PlaceID MainProgram::n_to_placeid(unsigned long int n)
{
    unsigned long int hash = prime2_*n + prime1_;

    return hash % static_cast<unsigned long int>(std::numeric_limits<PlaceID>::max());
}

Coord MainProgram::n_to_coord(unsigned long n)
{
    unsigned long int hash = prime1_ * n + prime2_;
    hash = hash ^ (hash + 0x9e3779b9 + (hash << 6) + (hash >> 2)); // :-P

    return {static_cast<int>(hash % 1000), static_cast<int>((hash/1000) % 1000)};
}

void MainProgram::init_regexs()
{
    // Create regex <whitespace>(cmd1|cmd2|...)<whitespace>(.*)
    string cmds_regex_str = "[[:space:]]*(";
    bool first = true;
    for (auto& cmd : cmds_)
    {
        cmds_regex_str += (first ? "" : "|") + cmd.cmd;
        first = false;

        cmd.param_regex = regex(cmd.param_regex_str+"[[:space:]]*", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    }
    cmds_regex_str += ")(?:[[:space:]]*$|"+wsx+"(.*))";
    cmds_regex_ = regex(cmds_regex_str, std::regex_constants::ECMAScript | std::regex_constants::optimize);
    coords_regex_ = regex(coordx+"[[:space:]]?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    times_regex_ = regex(wsx+"([0-9][0-9]):([0-9][0-9]):([0-9][0-9])", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    commands_regex_ = regex("("+cmdx+");?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
    sizes_regex_ = regex(numx+";?", std::regex_constants::ECMAScript | std::regex_constants::optimize);
}
