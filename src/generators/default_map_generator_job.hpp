/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

class config;
class game_config_view;

#include "default_map_generator.hpp"
#include "map/location.hpp"
#include "terrain/translation.hpp"

#include <random>
#include <cstdint>
#include <map>

class default_map_generator_job
{
public:
	default_map_generator_job();
	default_map_generator_job(uint32_t seed);

	/** Generate the map. */
	std::string default_generate_map(generator_data data, std::map<map_location,std::string>* labels, const config& cfg);

	typedef std::vector<std::vector<int>> height_map;
	typedef t_translation::ter_map terrain_map;


	height_map generate_height_map(std::size_t width, std::size_t height,
			std::size_t iterations, std::size_t hill_size,
			std::size_t island_size, std::size_t island_off_center);

	height_map generate_height_map(std::size_t width, std::size_t height,
			std::size_t iterations, std::size_t hill_size,
			std::size_t island_size, std::size_t center_x, std::size_t center_y);

private:

	bool generate_river_internal(const height_map& heights,
			terrain_map& terrain, int x, int y, std::vector<map_location>& river,
			std::set<map_location>& seen_locations, int river_uphill);

	std::vector<map_location> generate_river(const height_map& heights, terrain_map& terrain, int x, int y, int river_uphill);

	bool generate_lake(t_translation::ter_map& terrain, int x, int y, int lake_fall_off, std::set<map_location>& locs_touched);
	map_location random_point_at_side(std::size_t width, std::size_t height);

	std::mt19937 rng_;
	const game_config_view& game_config_;

};
