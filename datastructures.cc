// Datastructures.cc

#include "datastructures.hh"

#include <random>

#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>

std::minstd_rand rand_engine; // Reasonably quick pseudo-random generator

template <typename Type>
Type random_in_range(Type start, Type end)
{
    auto range = end-start;
    ++range;

    auto num = std::uniform_int_distribution<unsigned long int>(0, range-1)(rand_engine);

    return static_cast<Type>(start+num);
}

Datastructures::Datastructures()
{
    // No inititalization needed, STL containers are initialized automatically
}

Datastructures::~Datastructures()
{
    // No manual cleanup needed, STL containers handle memory automatically
}

int Datastructures::place_count()
{
    return places_.size();
}

void Datastructures::clear_all()
{
    places_.clear();
    areas_.clear();
    ways_.clear();

    place_ids_.clear();
    area_ids_.clear();
    way_ids_.clear();

    places_by_name_.clear();
    places_by_type_.clear();

    crossroads_.clear();

    alphabetical_cache_.clear();
    coord_order_cache_.clear();

    alphabetical_cache_valid_ = false;
    coord_order_cache_valid_ = false;
}

std::vector<PlaceID> Datastructures::all_places()
{
    return place_ids_;
}

bool Datastructures::add_place(PlaceID id, const Name& name, PlaceType type, Coord xy)
{
    if (places_.find(id) != places_.end())
    {
        return false;
    }
    PlaceInfo new_place;
    new_place.id =id;
    new_place.name = name;
    new_place.type = type;
    new_place.coord = xy;

    places_[id] = new_place;

    place_ids_.push_back(id);

    places_by_name_.insert({name,id});
    places_by_type_.insert({type,id});

    invalidate_place_caches();

    return true;

}

std::pair<Name, PlaceType> Datastructures::get_place_name_type(PlaceID id)
{
    auto it = places_.find(id);

    if(it == places_.end())
    {
        return {NO_NAME, PlaceType::NO_TYPE};
    }
    return {it ->second.name, it-> second.type};
}

Coord Datastructures::get_place_coord(PlaceID id)
{
    auto it = places_.find(id);

    if (it == places_.end())
    {
        return NO_COORD;

    }
    return it ->second.coord;
}

std::vector<PlaceID> Datastructures::places_alphabetically()
{
    if (alphabetical_cache_valid_)
    {
        return alphabetical_cache_;
    }

    alphabetical_cache_ = place_ids_;

    std::sort(alphabetical_cache_.begin(), alphabetical_cache_.end(),
              [this](PlaceID a, PlaceID b)
              {
                  return places_[a].name < places_[b].name;
              });

    alphabetical_cache_valid_ = true;

    return alphabetical_cache_;
}

std::vector<PlaceID> Datastructures::places_coord_order()
{
    if (coord_order_cache_valid_)
    {
        return coord_order_cache_;
    }

    coord_order_cache_ = place_ids_;

    std::sort(coord_order_cache_.begin(), coord_order_cache_.end(),
              [this](PlaceID a, PlaceID b)
              {
                  Coord ca = places_[a].coord;
                  Coord cb = places_[b].coord;

                  long long da = (long long)ca.x * ca.x + (long long)ca.y * ca.y;
                  long long db = (long long)cb.x * cb.x + (long long)cb.y * cb.y;

                  if (da != db)
                  {
                      return da < db;
                  }

                  return ca.y < cb.y;
              });

    coord_order_cache_valid_ = true;

    return coord_order_cache_;
}

std::vector<PlaceID> Datastructures::find_places_name(Name const& name)
{
    std::vector<PlaceID> result;

    auto range = places_by_name_.equal_range(name);

    for (auto it = range.first; it != range.second; ++it)
    {
        result.push_back(it->second);
    }

    return result;
}

std::vector<PlaceID> Datastructures::find_places_type(PlaceType type)
{
    std::vector<PlaceID> result;

    auto range = places_by_type_.equal_range(type);

    for (auto it = range.first; it != range.second; ++it)
    {
        result.push_back(it->second);
    }

    return result;
}

bool Datastructures::change_place_name(PlaceID id, const Name& newname)
{
    auto it = places_.find(id);

    if (it == places_.end())
    {
        return false;
    }

    // Remove old name from index
    Name oldname = it->second.name;
    auto range = places_by_name_.equal_range(oldname);

    for (auto iter = range.first; iter != range.second; ++iter)
    {
        if (iter->second == id)
        {
            places_by_name_.erase(iter);
            break;
        }
    }

    // Update name
    it->second.name = newname;

    // Insert new name to index
    places_by_name_.insert({newname, id});

    invalidate_place_caches();

    return true;
}

bool Datastructures::add_area(AreaID id, const Name & name, std::vector<Coord> coords)
{
    if (areas_.find(id) != areas_.end())
    {
        return false;
    }

    AreaInfo new_area;
    new_area.id = id;
    new_area.name = name;
    new_area.coords = coords;
    new_area.parent = NO_AREA;

    areas_[id] = new_area;

    area_ids_.push_back(id);

    return true;
}

Name Datastructures::get_area_name(AreaID id)
{
    auto it = areas_.find(id);

    if (it == areas_.end())
    {
        return NO_NAME;
    }

    return it->second.name;
}

std::vector<Coord> Datastructures::get_area_coords(AreaID id)
{
    auto it = areas_.find(id);

    if (it == areas_.end())
    {
        return {NO_COORD};
    }

    return it->second.coords;
}

std::vector<AreaID> Datastructures::all_areas()
{
    return area_ids_;
}

bool Datastructures::add_subarea_to_area(AreaID id, AreaID parentid)
{
    auto it_child = areas_.find(id);
    auto it_parent = areas_.find(parentid);

    if (it_child == areas_.end() || it_parent == areas_.end())
    {
        return false;
    }

    // Subarea already has a parent
    if (it_child->second.parent != NO_AREA)
    {
        return false;
    }

    // Set parent
    it_child->second.parent = parentid;

    // Add to parent's children
    it_parent->second.children.push_back(id);

    return true;
}

std::vector<AreaID> Datastructures::subarea_in_areas(AreaID id)
{
    std::vector<AreaID> result;

    auto it = areas_.find(id);
    if (it == areas_.end())
    {
        return {NO_AREA};
    }

    AreaID current = it->second.parent;

    while (current != NO_AREA)
    {
        result.push_back(current);
        current = areas_[current].parent;
    }

    return result;
}


std::vector<AreaID> Datastructures::all_subareas_in_area(AreaID id)
{
    if (areas_.find(id) == areas_.end())
    {
        return {NO_AREA};
    }

    std::vector<AreaID> result;
    collect_subareas_recursive(id, result);

    return result;
}

std::vector<PlaceID> Datastructures::places_closest_to(Coord xy, PlaceType type)
{
    std::vector<std::pair<long long, PlaceID>> temp;

    for (auto& [id, p] : places_)
    {
        if (type != PlaceType::NO_TYPE && p.type != type)
        {
            continue;
        }

        long long dx = p.coord.x - xy.x;
        long long dy = p.coord.y - xy.y;
        long long dist = dx*dx + dy*dy;

        temp.push_back({dist, id});
    }

    std::sort(temp.begin(), temp.end(),
              [this](auto a, auto b)
              {
                  if (a.first != b.first)
                      return a.first < b.first;

                  return places_[a.second].coord.y < places_[b.second].coord.y;
              });

    std::vector<PlaceID> result;
    for (int i = 0; i < std::min(3, (int)temp.size()); ++i)
    {
        result.push_back(temp[i].second);
    }

    return result;
}

bool Datastructures::remove_place(PlaceID id)
{
    auto it = places_.find(id);
    if (it == places_.end())
        return false;

    Name name = it->second.name;
    PlaceType type = it->second.type;

    places_.erase(it);

    place_ids_.erase(std::remove(place_ids_.begin(), place_ids_.end(), id), place_ids_.end());

    auto r1 = places_by_name_.equal_range(name);
    for (auto i = r1.first; i != r1.second; ++i)
    {
        if (i->second == id)
        {
            places_by_name_.erase(i);
            break;
        }
    }

    auto r2 = places_by_type_.equal_range(type);
    for (auto i = r2.first; i != r2.second; ++i)
    {
        if (i->second == id)
        {
            places_by_type_.erase(i);
            break;
        }
    }

    invalidate_place_caches();
    return true;
}

AreaID Datastructures::common_area_of_subareas(AreaID id1 , AreaID id2)
{
    auto it1 = areas_.find(id1);
    auto it2 = areas_.find(id2);

    if (it1 == areas_.end() || it2 == areas_.end())
    {
        return NO_AREA;
    }

    std::unordered_set<AreaID> ancestors;

    AreaID cur = it1->second.parent;
    while (cur != NO_AREA)
    {
        ancestors.insert(cur);
        cur = areas_[cur].parent;
    }

    cur = it2->second.parent;
    while (cur != NO_AREA)
    {
        if (ancestors.find(cur) != ancestors.end())
        {
            return cur;
        }
        cur = areas_[cur].parent;
    }

    return NO_AREA;
}

void Datastructures::clear_ways()
{
    ways_.clear();
    way_ids_.clear();
    crossroads_.clear();
}

std::vector<WayID> Datastructures::all_ways()
{
    return way_ids_;
}

bool Datastructures::add_way(WayID id, std::vector<Coord>coords)
{   
    if (ways_.find(id) != ways_.end())
    {
        return false;
    }

    WayInfo w;
    w.id = id;
    w.coords = coords;
    w.start = coords.front();
    w.end = coords.back();

    w.length = calculate_way_length(coords);

    ways_[id] = w;
    way_ids_.push_back(id);

    crossroads_[w.start].push_back(id);

    if (w.end != w.start)
    {
        crossroads_[w.end].push_back(id);
    }

    return true;
}

std::vector<Coord> Datastructures::get_way_coords(WayID id)
{
    if (ways_.find(id) == ways_.end())
    {
        return {NO_COORD};
    }

    return ways_[id].coords;
}

std::vector<std::pair<WayID, Coord>> Datastructures::ways_from(Coord xy)
{
    std::vector<std::pair<WayID, Coord>> result;

    if (crossroads_.find(xy) == crossroads_.end())
    {
        return result;
    }

    for (auto wid : crossroads_[xy])
    {
        auto& w = ways_[wid];

        Coord next = (w.start == xy ? w.end : w.start);

        result.push_back({wid, next});
    }

    return result;
}

std::vector<std::tuple<Coord, WayID, Distance> > Datastructures::route_any(Coord from, Coord to)
{
    auto it_from = crossroads_.find(from);
    auto it_to = crossroads_.find(to);

    if (it_from == crossroads_.end() || it_to == crossroads_.end()
        || it_from->second.empty() || it_to->second.empty())
    {
        return { {NO_COORD, NO_WAY, NO_DISTANCE} };
    }

    if (from == to)
    {
        return { {from, NO_WAY, 0} };
    }

    std::queue<Coord> q;
    std::unordered_map<Coord, std::pair<Coord, WayID>, CoordHash> parent;
    std::unordered_set<Coord, CoordHash> visited;

    q.push(from);
    visited.insert(from);

    while (!q.empty())
    {
        Coord cur = q.front();
        q.pop();

        if (cur == to)
        {
            break;
        }

        for (auto const& [wid, next] : ways_from(cur))
        {
            if (!visited.count(next))
            {
                visited.insert(next);
                parent[next] = {cur, wid};
                q.push(next);
            }
        }
    }

    if (!visited.count(to))
    {
        return {};
    }

    std::vector<Coord> nodes_reversed;
    std::vector<WayID> ways_reversed;

    Coord cur = to;
    nodes_reversed.push_back(cur);

    while (cur != from)
    {
        auto [prev, wid] = parent[cur];
        ways_reversed.push_back(wid);
        cur = prev;
        nodes_reversed.push_back(cur);
    }

    std::reverse(nodes_reversed.begin(), nodes_reversed.end());
    std::reverse(ways_reversed.begin(), ways_reversed.end());

    std::vector<std::tuple<Coord, WayID, Distance>> result;
    Distance dist = 0;

    for (std::size_t i = 0; i < ways_reversed.size(); ++i)
    {
        result.push_back({nodes_reversed[i], ways_reversed[i], dist});
        dist += ways_[ways_reversed[i]].length;
    }

    result.push_back({nodes_reversed.back(), NO_WAY, dist});

    return result;
}


bool Datastructures::remove_way(WayID id)
{
    auto it = ways_.find(id);
    if (it == ways_.end())
    {
        return false;
    }

    Coord start = it->second.start;
    Coord end = it->second.end;

    auto it_start = crossroads_.find(start);
    if (it_start != crossroads_.end())
    {
        auto& v = it_start->second;
        v.erase(std::remove(v.begin(), v.end(), id), v.end());
        if (v.empty())
        {
            crossroads_.erase(it_start);
        }
    }

    if (end != start)
    {
        auto it_end = crossroads_.find(end);
        if (it_end != crossroads_.end())
        {
            auto& v = it_end->second;
            v.erase(std::remove(v.begin(), v.end(), id), v.end());
            if (v.empty())
            {
                crossroads_.erase(it_end);
            }
        }
    }

    ways_.erase(it);
    way_ids_.erase(std::remove(way_ids_.begin(), way_ids_.end(), id), way_ids_.end());

    return true;
}

std::vector<std::tuple<Coord, WayID, Distance> > Datastructures::route_least_crossroads(Coord from, Coord to)
{
    auto it_from = crossroads_.find(from);
    auto it_to = crossroads_.find(to);

    if (it_from == crossroads_.end() || it_to == crossroads_.end()
        || it_from->second.empty() || it_to->second.empty())
    {
        return { {NO_COORD, NO_WAY, NO_DISTANCE} };
    }

    if (from == to)
    {
        return { {from, NO_WAY, 0} };
    }

    std::queue<Coord> q;
    std::unordered_map<Coord, std::pair<Coord, WayID>, CoordHash> parent;
    std::unordered_set<Coord, CoordHash> visited;

    q.push(from);
    visited.insert(from);

    while (!q.empty())
    {
        Coord cur = q.front();
        q.pop();

        if (cur == to)
        {
            break;
        }

        for (auto const& [wid, next] : ways_from(cur))
        {
            if (!visited.count(next))
            {
                visited.insert(next);
                parent[next] = {cur, wid};
                q.push(next);
            }
        }
    }

    if (!visited.count(to))
    {
        return {};
    }

    std::vector<Coord> nodes_reversed;
    std::vector<WayID> ways_reversed;

    Coord cur = to;
    nodes_reversed.push_back(cur);

    while (cur != from)
    {
        auto [prev, wid] = parent[cur];
        ways_reversed.push_back(wid);
        cur = prev;
        nodes_reversed.push_back(cur);
    }

    std::reverse(nodes_reversed.begin(), nodes_reversed.end());
    std::reverse(ways_reversed.begin(), ways_reversed.end());

    std::vector<std::tuple<Coord, WayID, Distance>> result;
    Distance dist = 0;

    for (std::size_t i = 0; i < ways_reversed.size(); ++i)
    {
        result.push_back({nodes_reversed[i], ways_reversed[i], dist});
        dist += ways_[ways_reversed[i]].length;
    }

    result.push_back({nodes_reversed.back(), NO_WAY, dist});
    return result;
}

std::vector<std::tuple<Coord, WayID> > Datastructures::route_with_cycle(Coord from)
{
    auto it_from = crossroads_.find(from);
    if (it_from == crossroads_.end() || it_from->second.empty())
    {
        return { {NO_COORD, NO_WAY} };
    }

    std::unordered_set<Coord, CoordHash> visited;
    std::vector<Coord> path_coords;
    std::vector<WayID> path_ways;

    std::vector<std::tuple<Coord, WayID>> result;

    std::function<bool(Coord, Coord, WayID)> dfs =
        [&](Coord cur, Coord prev, WayID prev_way) -> bool
    {
        visited.insert(cur);
        path_coords.push_back(cur);

        for (auto const& [wid, next] : ways_from(cur))
        {
            // Do not immediately go back along the same edge
            if (next == prev && wid == prev_way)
            {
                continue;
            }

            // Found a cycle: next is already on the current route
            auto it = std::find(path_coords.begin(), path_coords.end(), next);
            if (it != path_coords.end())
            {
                path_ways.push_back(wid);

                for (std::size_t i = 0; i < path_ways.size(); ++i)
                {
                    result.push_back({path_coords[i], path_ways[i]});
                }
                result.push_back({next, NO_WAY});
                return true;
            }

            if (!visited.count(next))
            {
                path_ways.push_back(wid);

                if (dfs(next, cur, wid))
                {
                    return true;
                }

                path_ways.pop_back();
            }
        }

        path_coords.pop_back();
        return false;
    };

    if (!dfs(from, NO_COORD, NO_WAY))
    {
        return {};
    }

    return result;
}

std::vector<std::tuple<Coord, WayID, Distance> > Datastructures::route_shortest_distance(Coord from, Coord to)
{
    auto it_from = crossroads_.find(from);
    auto it_to = crossroads_.find(to);

    if (it_from == crossroads_.end() || it_to == crossroads_.end()
        || it_from->second.empty() || it_to->second.empty())
    {
        return { {NO_COORD, NO_WAY, NO_DISTANCE} };
    }

    if (from == to)
    {
        return { {from, NO_WAY, 0} };
    }

    using State = std::pair<Distance, Coord>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    std::unordered_map<Coord, Distance, CoordHash> dist;
    std::unordered_map<Coord, std::pair<Coord, WayID>, CoordHash> parent;

    dist[from] = 0;
    pq.push({0, from});

    while (!pq.empty())
    {
        auto [curdist, cur] = pq.top();
        pq.pop();

        if (curdist > dist[cur])
        {
            continue;
        }

        if (cur == to)
        {
            break;
        }

        for (auto const& [wid, next] : ways_from(cur))
        {
            Distance nd = curdist + ways_[wid].length;

            if (!dist.count(next) || nd < dist[next])
            {
                dist[next] = nd;
                parent[next] = {cur, wid};
                pq.push({nd, next});
            }
        }
    }

    if (!dist.count(to))
    {
        return {};
    }

    std::vector<Coord> nodes_reversed;
    std::vector<WayID> ways_reversed;

    Coord cur = to;
    nodes_reversed.push_back(cur);

    while (cur != from)
    {
        auto [prev, wid] = parent[cur];
        ways_reversed.push_back(wid);
        cur = prev;
        nodes_reversed.push_back(cur);
    }

    std::reverse(nodes_reversed.begin(), nodes_reversed.end());
    std::reverse(ways_reversed.begin(), ways_reversed.end());

    std::vector<std::tuple<Coord, WayID, Distance>> result;
    Distance total = 0;

    for (std::size_t i = 0; i < ways_reversed.size(); ++i)
    {
        result.push_back({nodes_reversed[i], ways_reversed[i], total});
        total += ways_[ways_reversed[i]].length;
    }

    result.push_back({nodes_reversed.back(), NO_WAY, total});
    return result;
}

Distance Datastructures::trim_ways()
{

        throw NotImplemented();
}

void Datastructures::invalidate_place_caches()
{
    alphabetical_cache_valid_ = false;
    coord_order_cache_valid_ = false;
}

void Datastructures::collect_subareas_recursive(AreaID id, std::vector<AreaID>& result)
{
    for (AreaID child : areas_[id].children)
    {
        result.push_back(child);
        collect_subareas_recursive(child, result);
    }
}

Distance Datastructures::calculate_way_length(std::vector<Coord> const& coords)
{
    Distance total = 0;

    for (size_t i = 1; i < coords.size(); ++i)
    {
        int dx = coords[i].x - coords[i-1].x;
        int dy = coords[i].y - coords[i-1].y;

        double dist = std::sqrt(dx*dx + dy*dy);

        total += static_cast<int>(dist); // round down
    }

    return total;
}