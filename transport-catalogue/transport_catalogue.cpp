#include "transport_catalogue.h"
#include "geo.h"
#include <unordered_set>
namespace transport {
void TransportCatalogue::AddStop(std::string name, double lat, double lon)
{
	stops_.push_back({ std::move(name),lat,lon });
	Stop& new_stop = stops_.back();
	stop_names_[new_stop.name] = &new_stop;
}

void TransportCatalogue::AddBus(std::string name, std::vector<std::string> stop_names, bool is_ring)
{
	Bus bus;
	bus.name = std::move(name);
	bus.is_ring = is_ring;
	bus.stops.reserve(stop_names.size());
	for (const auto& stop_name : stop_names) {
		const Stop* stop = FindStop(stop_name);
		bus.stops.push_back(stop);
	}

	buses_.push_back(std::move(bus));
	const Bus& new_bus = buses_.back();
	bus_names_[new_bus.name] = &new_bus;

	for (const Stop* stop : new_bus.stops)  {
		stop_to_buses_[stop].insert(new_bus.name);
	}
}

const TransportCatalogue::Stop* TransportCatalogue::FindStop(std::string name) const
{
	auto it = stop_names_.find(name);
	if (it != stop_names_.end()) {
		return it->second;
	}
	return nullptr;
}

const TransportCatalogue::Bus* TransportCatalogue::FindBus(std::string name) const
{
	auto it = bus_names_.find(name);
	if (it != bus_names_.end()) {
		return it->second;
	}
	return nullptr;
}

TransportCatalogue::RouteInfo TransportCatalogue::GetRouteInfo(const Bus* bus) const
{
	if (!bus) {
		return { 0,0,0.0 };
	}

	std::vector<const Stop*> route_stops;
	route_stops.reserve(bus->stops.size() * 2 - 1);

	route_stops.insert(route_stops.end(), bus->stops.begin(), bus->stops.end());

	if (!bus->is_ring) {
		for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
			route_stops.push_back(*it);
		}
	}

	size_t stops_count = route_stops.size();
	
	std::unordered_set<std::string_view> unique_names;
	for (const Stop* stop : route_stops) {
		unique_names.insert(stop->name);
	}
	size_t unique_stops = unique_names.size();

	double route_length = 0.0;
	for (size_t i = 1; i < route_stops.size(); ++i) {
		route_length += ComputeDistance(
			Coordinates{ route_stops[i - 1]->latitude, route_stops[i - 1]->longitude },
			Coordinates{ route_stops[i]->latitude, route_stops[i]->longitude });
	}
	return RouteInfo{ stops_count,unique_stops,route_length };
}

const std::set<std::string>* TransportCatalogue::GetBusesForStop(const Stop* stop) const {
	if (!stop) {
		return nullptr;
	}
	auto it = stop_to_buses_.find(stop);
	if (it != stop_to_buses_.end()) {
		return &it->second;
	}
	return nullptr;
}
} // namespace transport