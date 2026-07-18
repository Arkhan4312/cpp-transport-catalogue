#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <set>
namespace transport {
class TransportCatalogue {
private:
	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	struct Bus {
		std::string name;
		bool is_ring = false;
		std::vector<const Stop*> stops;
	};

public:
	struct RouteInfo {
		size_t stops_count;
		size_t unique_stops;
		double route_length;
	};

	void AddStop(std::string name, double lat, double lon);

	void AddBus(std::string name, std::vector<std::string> stop_names, bool is_ring);

	const Stop* FindStop(std::string name) const;

	const Bus* FindBus(std::string name) const;
	
	RouteInfo GetRouteInfo(const Bus* bus) const;

	const std::set<std::string>* GetBusesForStop(const Stop* stop) const;
private:	
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stop_names_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> bus_names_;
	std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses_;
};
} // namespace transport