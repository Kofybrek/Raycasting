#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "Headers/Global.hpp"
#include "Headers/Steven.hpp"
#include "Headers/Player.hpp"
#include "Headers/ConvertSketch.hpp"

std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH> convert_sketch(Player& i_player, Steven& i_steven)
{
	std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH> output_map{};

	sf::Image map_sketch;
	map_sketch.loadFromFile("Resources/Images/MapSketch.png");

	for (unsigned char a = 0; a < MAP_WIDTH; a++)
	{
		for (unsigned char b = 0; b < MAP_HEIGHT; b++)
		{
			sf::Color pixel = map_sketch.getPixel(a, b);

			if (pixel == sf::Color(0, 0, 0))
			{
				output_map[a][b] = Cell::Wall;
			}
			else if (pixel == sf::Color(255, 0, 0))
			{
				i_player.set_position(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
			}
			else if (pixel == sf::Color(0, 0, 255))
			{
				i_steven.set_position(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
			}
			else
			{
				output_map[a][b] = Cell::Empty;
			}
		}
	}

	return output_map;
}