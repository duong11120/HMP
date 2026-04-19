// Datastructure.hh

#ifndef DATASTRUCTURES_HH
#define DATASTRUCTURES_HH

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <limits>
#include <functional>


// Types for IDs
using PlaceID = long long int;
using AreaID = long long int;
using Name = std::string;
using WayID = std::string;

// Return values for cases where required thing was not found
PlaceID const NO_PLACE = -1;
AreaID const NO_AREA = -1;
WayID const NO_WAY = "!!No way!!";

// Return value for cases where integer values were not found
int const NO_VALUE = std::numeric_limits<int>::min();

// Return value for cases where name values were not found
Name const NO_NAME = "!!NO_NAME!!";

// Enumeration for different place types
// !!Note since this is a C++11 "scoped enumeration", you'll have to refer to
// individual values as PlaceType::SHELTER etc.
enum class PlaceType { OTHER=0, FIREPIT, SHELTER, PARKING, PEAK, BAY, AREA, NO_TYPE };

// Type for a coordinate (x, y)
struct Coord
{
    int x = NO_VALUE;
    int y = NO_VALUE;
};

// Example: Defining == and hash function for Coord so that it can be used
// as key for std::unordered_map/set, if needed
inline bool operator==(Coord c1, Coord c2) { return c1.x == c2.x && c1.y == c2.y; }
inline bool operator!=(Coord c1, Coord c2) { return !(c1==c2); } // Not strictly necessary

struct CoordHash
{
    std::size_t operator()(Coord xy) const
    {
        auto hasher = std::hash<int>();
        auto xhash = hasher(xy.x);
        auto yhash = hasher(xy.y);
        // Combine hash values (magic!)
        return xhash ^ (yhash + 0x9e3779b9 + (xhash << 6) + (xhash >> 2));
    }
};

// Example: Defining < for Coord so that it can be used
// as key for std::map/set
inline bool operator<(Coord c1, Coord c2)
{
    if (c1.y < c2.y) { return true; }
    else if (c2.y < c1.y) { return false; }
    else { return c1.x < c2.x; }
}

// Return value for cases where coordinates were not found
Coord const NO_COORD = {NO_VALUE, NO_VALUE};

// Type for a distance (in metres)
using Distance = int;

// Return value for cases where Duration is unknown
Distance const NO_DISTANCE = NO_VALUE;



// This exception class is there just so that the user interface can notify
// about operations which are not (yet) implemented
class NotImplemented : public std::exception
{
public:
    explicit NotImplemented(std::string const& msg = "Not implemented")
        : msg_(msg)
    {
    }

    const char* what() const noexcept override
    {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

// This is the class you are supposed to implement

class Datastructures
{
public:
    Datastructures();
    ~Datastructures();

    // Estimate of performance: O(1)
    // Short rationale for estimate: directly returns size of unordered_map
    int place_count();

    // Estimate of performance: O(n)
    // Short rationale for estimate: clears multiple containers,total proportional to number of elements
    void clear_all();

    // Estimate of performance: O(n)
    // Short rationale for estimate: returns vector of all place IDs
    std::vector<PlaceID> all_places();

    // Estimate of performance: O(log n) average O(1)
    // Short rationale for estimate: unordered_map insert + multimap insert
    bool add_place(PlaceID id, Name const& name, PlaceType type, Coord xy);

    // Estimate of performance: O(1)
    // Short rationale for estimate: unordered_map find
    std::pair<Name, PlaceType> get_place_name_type(PlaceID id);

    // Estimate of performance: O(1)
    // Short rationale for estimate: unordered_map find
    Coord get_place_coord(PlaceID id);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance: O(n log n)
    // Short rationale for estimate: sorting vector of place IDs by name
    std::vector<PlaceID> places_alphabetically();

    // Estimate of performance: O(n log n)
    // Short rationale for estimate: sorting by distance from origin
    std::vector<PlaceID> places_coord_order();

    // Estimate of performance:O(k)
    // Short rationale for estimate: equal_range over multimap
    std::vector<PlaceID> find_places_name(Name const& name);

    // Estimate of performance: O(k)
    // Short rationale for estimate: equal_range over multimap
    std::vector<PlaceID> find_places_type(PlaceType type);

    // Estimate of performance: O(k)
    // Short rationale for estimate: remove old entry + insert new in multimap
    bool change_place_name(PlaceID id, Name const& newname);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance: O(1)
    // Short rationale for estimate: insert into unordered map
    bool add_area(AreaID id, Name const& name, std::vector<Coord> coords);

    // Estimate of performance: O(1)
    // Short rationale for estimate: lookup in unordered_map
    Name get_area_name(AreaID id);

    // Estimate of performance: O(1)
    // Short rationale for estimate: direct access
    std::vector<Coord> get_area_coords(AreaID id);

    // Estimate of performance: O(n)
    // Short rationale for estimate: return vector
    std::vector<AreaID> all_areas();

    // Estimate of performance: O(1)
    // Short rationale for estimate: update parent and push to children vector
    bool add_subarea_to_area(AreaID id, AreaID parentid);

    // Estimate of performance: O(h)
    // Short rationale for estimate: traverse up parent chain
    std::vector<AreaID> subarea_in_areas(AreaID id);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance: O(n)
    // Short rationale for estimate: recursive traversal of subtree
    std::vector<AreaID> all_subareas_in_area(AreaID id);

    // Estimate of performance: O(n log n)
    // Short rationale for estimate: compute distances + sort
    std::vector<PlaceID> places_closest_to(Coord xy, PlaceType type);

    // Estimate of performance: O(n)
    // Short rationale for estimate: remove from vector + multimap cleanup
    bool remove_place(PlaceID id);

    // Estimate of performance: O(h)
    // Short rationale for estimate: traverse ancestors
    AreaID common_area_of_subareas(AreaID id1, AreaID id2);

    // Estimate of performance: O(n)
    // Short rationale for estimate: clear all structures
    void clear_ways();

    // Estimate of performance: O(n)
    // Short rationale for estimate: return vector
    std::vector<WayID> all_ways();

    // Estimate of performance: O(n)
    // Short rationale for estimate: calculate length + insert
    bool add_way(WayID id, std::vector<Coord> coords);

    // Estimate of performance: O(1)
    // Short rationale for estimate: direct access
    std::vector<Coord> get_way_coords(WayID id);

    // Estimate of performance: O(k)
    // Short rationale for estimate: iterate crossroads adjacency list
    std::vector<std::pair<WayID, Coord>> ways_from(Coord xy);

    // We recommend you implement the operations below only after implementing the ones above

    // Estimate of performance: O(V+E)
    // Short rationale for estimate: BFS traversal
    std::vector<std::tuple<Coord, WayID, Distance>> route_any(Coord fromxy, Coord toxy);

    // Estimate of performance: O(n)
    // Short rationale for estimate: remove from vectors and maps
    bool remove_way(WayID id);

    // Estimate of performance: O(V+E)
    // Short rationale for estimate: uses BFS on the way graph, which visits each crossroad and connection at most once
    std::vector<std::tuple<Coord, WayID, Distance>> route_least_crossroads(Coord fromxy, Coord toxy);

    // Estimate of performance: O(V+E)
    // Short rationale for estimate: uses DFS to detect a cycle, visiting each crossroad and connection at most once
    std::vector<std::tuple<Coord, WayID>> route_with_cycle(Coord fromxy);

    // Estimate of performance: O((V+E) log V)
    // Short rationale for estimate: dijkstra with priority queue
    std::vector<std::tuple<Coord, WayID, Distance>> route_shortest_distance(Coord fromxy, Coord toxy);

    // The operation below is a bonus operation (a little more challenging and probably requires googling for an algorithm)

    // Estimate of performance: O(n)
    // Short rationale for estimate: find positions + rebuild vector
    Distance trim_ways();

private:
    // Add stuff needed for your class implementation here

    struct PlaceInfo
    {
        PlaceID id;
        Name name;
        PlaceType type;
        Coord coord;
    };

    struct AreaInfo
    {
        AreaID id;
        Name name;
        std::vector<Coord> coords;
        AreaID parent = NO_AREA;
        std::vector<AreaID> children;
    };

    struct WayInfo
    {
        WayID id;
        std::vector<Coord> coords;
        Coord start = NO_COORD;
        Coord end = NO_COORD;
        Distance length = 0;
    };

    std::unordered_map<PlaceID, PlaceInfo> places_;
    std::unordered_map<AreaID, AreaInfo> areas_;
    std::unordered_map<WayID, WayInfo> ways_;

    std::vector<PlaceID> place_ids_;
    std::vector<AreaID> area_ids_;
    std::vector<WayID> way_ids_;

    std::unordered_multimap<Name, PlaceID> places_by_name_;
    std::unordered_multimap<PlaceType, PlaceID> places_by_type_;

    std::unordered_map<Coord, std::vector<WayID>, CoordHash> crossroads_;

    bool alphabetical_cache_valid_ = false;
    bool coord_order_cache_valid_ = false;

    std::vector<PlaceID> alphabetical_cache_;
    std::vector<PlaceID> coord_order_cache_;

    static long long squared_distance_from_origin(Coord coord);
    static long long squared_distance_between(Coord a, Coord b);
    static bool coord_order_compare(Coord a, Coord b);
    static Distance calculate_way_length(std::vector<Coord> const& coords);

    void collect_subareas_recursive(AreaID id, std::vector<AreaID>& result);
    void invalidate_place_caches();
};


#endif // DATASTRUCTURES_HH
