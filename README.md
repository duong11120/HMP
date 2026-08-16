# Hiking Map DSA Project

A C++ data structures and algorithms project developed for the
COMP.CS.300 Data Structures and Algorithms course at Tampere University.

The project implements a hiking map data structure for managing areas,
places, ways, crossroads, and routes.

## Overview

The application provides functionality for:

- Managing geographical areas and subareas
- Managing places and points of interest
- Managing ways and their coordinates
- Finding ways connected to a coordinate
- Finding routes between crossroads
- Finding routes with different routing criteria
- Handling cycles and interconnected ways
- Reading and executing command files for testing

## Main Features

### Areas

- Add and remove areas
- Store area coordinates
- Create relationships between areas and subareas
- Find subareas belonging to an area
- Find common areas between subareas

### Places

- Add places with an ID, name, type, and position
- Find places close to a given coordinate
- Filter nearby places by type

### Ways

- Add and remove ways
- Store way coordinates
- Find ways connected to a coordinate
- Retrieve coordinates belonging to a way

### Routing

The project implements route-finding functionality over the way network,
including:

- `route_any`
- `route_least_crossroads`
- `route_shortest_distance`
- `route_with_cycle`

The routing functionality handles connected ways, crossroads, loops,
and cycles in the map.

## Technologies

- **C++**
- **Qt / Qt Creator**
- Data structures
- Graph algorithms
- Searching and traversal algorithms
- Git / GitLab / GitHub

## Project Structure

The project contains the main data structure implementation together with
the application and testing infrastructure provided for the course.

Typical components include:

- Data structure implementation
- Command-line interface
- Test suites
- Build configuration
- Course-provided supporting files

## Testing

The project was tested using the course-provided test suites.

Testing covered:

- Basic area functionality
- Subarea relationships
- Places
- Ways
- Route finding
- Shortest-distance routing
- Least-crossroad routing
- Routes containing cycles
- Removing ways

The implementation was tested against the provided A, B and C level
functionality.

## Building

The project can be opened and built using Qt Creator with the provided
CMake configuration.

After configuring the project, build and run the application and execute
the provided command/test files.

## Course

**COMP.CS.300 – Data Structures and Algorithms**

Tampere University  
Spring 2026

## Author

**Dương Đào Thuỳ**

Master's student in Data Science  
Tampere University
