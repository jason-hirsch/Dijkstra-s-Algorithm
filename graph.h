#ifndef GRAPH_H
#define GRAPH_H

#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class Graph {
	// TODO(student): implement Graph
private:
	struct hash_pair {
		template <class T>
		inline void hash_combine(std::size_t& seed, const T& v) const
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		template <class T1, class T2>
		size_t operator()(const std::pair<T1, T2>& p) const {
			size_t seed = 0;
			hash_combine(seed, p.first);
			hash_combine(seed, p.second);
			return seed;
		}
	};

public:
	std::unordered_map<size_t, std::unordered_set<size_t>> adjList;
	std::unordered_map<size_t, std::unordered_set<size_t>> reverseAdjList;
	std::unordered_map<std::pair<size_t, size_t>, double, hash_pair> weights;

	Graph() {

	}

	Graph(const Graph& other) : adjList(other.adjList), reverseAdjList(other.reverseAdjList), weights(other.weights) {
		
	}

	Graph& operator=(const Graph& rhs) {
		adjList = rhs.adjList;
		reverseAdjList = rhs.reverseAdjList;
		weights = rhs.weights;
		return *this;
	}

	~Graph() {

	}

	size_t vertex_count() const {
		return adjList.size();
	}

	size_t edge_count() const {
		return weights.size();
	}

	bool contains_vertex(size_t id) const {
		return adjList.count(id);
	}

	bool contains_edge(size_t src, size_t dest) const {
		return weights.count(std::make_pair(src, dest));
	}

	double cost(size_t src, size_t dest) const {
		if (contains_edge(src, dest)) {
			return weights.at(std::make_pair(src, dest));
		}
		return INFINITY;
	}

	bool add_vertex(size_t id) {
		adjList.emplace(id, std::unordered_set<size_t>());
		return reverseAdjList.emplace(id, std::unordered_set<size_t>()).second;
	}

	bool add_edge(size_t src, size_t dest, double weight = 1) {
		if (contains_vertex(src) && contains_vertex(dest) && !contains_edge(src, dest)) {
			adjList.at(src).emplace(dest);
			reverseAdjList.at(dest).emplace(src);
			weights.emplace(std::make_pair(src, dest), weight);
			return true;
		}
		return false;
	}

	bool remove_vertex(size_t id) {
		if (contains_vertex(id)) {
			for (auto it : adjList.at(id)) {
				weights.erase(std::make_pair(id, it)); //Erase outgoing edges
			}
			for (auto it : reverseAdjList.at(id)) {
				weights.erase(std::make_pair(it, id)); //Erase incoming edges
			}
			adjList.erase(id);
			reverseAdjList.erase(id);
			return true;
		}
		return false;
	}

	bool remove_edge(size_t src, size_t dest) {
		if (contains_edge(src, dest)) {
			adjList.at(src).erase(dest);
			reverseAdjList.at(dest).erase(src);
			weights.erase(std::make_pair(src, dest));
			return true;
		}
		return false;
	}

	struct VertexAndDistance {
		size_t id;
		double dist;

		VertexAndDistance(size_t id, double dist) : id(id), dist(dist) {

		}

		bool operator<(VertexAndDistance& rhs) {
			return dist < rhs.dist;
		}
	};

	std::unordered_map<size_t, size_t> cameFrom;
	int source = -1;
	bool dijkstrasDidNotRun = false;

	void dijkstra(size_t source_id) {
		cameFrom.clear();
		source = source_id;
		dijkstrasDidNotRun = false;
		
		if (!contains_vertex(source_id)) {
			dijkstrasDidNotRun = true;
			return;
		}

		for (auto it : weights) {
			if (it.second < 0) {
				dijkstrasDidNotRun = true;
				return;
			}
		}
		
		std::unordered_map<size_t, double> unvisited;
		for (auto it : adjList) {
			unvisited.emplace(it.first, INFINITY);
		}
		unvisited.at(source_id) = 0;

		while (!unvisited.empty()) {
			bool allInfinity = true;
			for (auto it : unvisited) {
				if (it.second != INFINITY) {
					allInfinity = false;
					break;
				}
			}

			if (allInfinity) {
				return;
			}

			auto minElemIt = unvisited.begin();
			for (auto it = unvisited.begin(); it != unvisited.end(); it++) {
				if (it->second < minElemIt->second) {
					minElemIt = it;
				}
			}

			for (auto it : adjList.at(minElemIt->first)) {
				if (unvisited.count(it)) {
					if (minElemIt->second + weights.at(std::make_pair(minElemIt->first, it)) < unvisited.at(it)) {
						unvisited.at(it) = minElemIt->second + weights.at(std::make_pair(minElemIt->first, it));
						cameFrom[it] = minElemIt->first;
					}
				}
			}

			unvisited.erase(minElemIt);
		}
	}

	void getPath(std::tuple<std::string, double>& path, size_t curr) const {
		if (!cameFrom.count(curr)) {
			std::get<0>(path) = std::to_string(curr);
			std::get<1>(path) = 0;
			return;
		}
		getPath(path, cameFrom.at(curr));
		std::get<0>(path) += " --> " + std::to_string(curr);
		std::get<1>(path) += weights.at(std::make_pair(cameFrom.at(curr), curr));
	}

	double distance(size_t id) const {
		if (!contains_vertex(id) || dijkstrasDidNotRun || (!cameFrom.count(id) && id != source)) {
			return INFINITY;
		}

		std::tuple<std::string, double> path;
		getPath(path, id);

		return std::get<1>(path);
	}

	void print_shortest_path(size_t dest_id, std::ostream& os = std::cout) const {
		if (!contains_vertex(dest_id) || dijkstrasDidNotRun || (!cameFrom.count(dest_id) && dest_id != source)) {
			os << "<no path>" << std::endl;
			return;
		}

		std::tuple<std::string, double> path;
		getPath(path, dest_id);

		if (std::get<1>(path) == INFINITY) {
			os << "<no path>" << std::endl;
			return;
		}

		os << std::get<0>(path) << " distance: " << std::get<1>(path) << std::endl;
	}
};


#endif  // GRAPH_H
