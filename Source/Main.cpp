#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "Headers/Global.hpp"
#include "Headers/Steven.hpp"
#include "Headers/Player.hpp"
#include "Headers/ConvertSketch.hpp"

int main()
{
	//We can hide the HUD.
	bool draw_map = 1;

	std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH> map{};

	//We'll use this variable to make the game framerate-independent.
	std::chrono::microseconds lag(0);

	std::chrono::steady_clock::time_point previous_time;

	sf::Event event;

	//I called this project "FPS" even though there's no "S".
	sf::RenderWindow window(sf::VideoMode(SCREEN_RESIZE * SCREEN_WIDTH, SCREEN_RESIZE * SCREEN_HEIGHT), "FPS", sf::Style::Close);
	window.setMouseCursorVisible(0);
	window.setView(sf::View(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)));

	sf::Sprite map_grid_cell_sprite;
	sf::Sprite map_wall_sprite;

	sf::Texture map_grid_cell_texture;
	map_grid_cell_texture.loadFromFile("Resources/Images/MapGridCell.png");

	sf::Texture map_wall_texture;
	map_wall_texture.loadFromFile("Resources/Images/MapWall" + std::to_string(MAP_CELL_SIZE) + ".png");

	Player player(0, 0);

	Steven steven(0, 0);

	map = convert_sketch(player, steven);

	map_grid_cell_sprite.setTexture(map_grid_cell_texture);
	map_grid_cell_sprite.setTextureRect(sf::IntRect(0, 0, MAP_GRID_CELL_SIZE, MAP_GRID_CELL_SIZE));
	map_wall_sprite.setTexture(map_wall_texture);

	previous_time = std::chrono::steady_clock::now();

	while (1 == window.isOpen())
	{
		std::chrono::microseconds delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time);

		lag += delta_time;

		previous_time += delta_time;

		while (FRAME_DURATION <= lag)
		{
			lag -= FRAME_DURATION;

			while (1 == window.pollEvent(event))
			{
				switch (event.type)
				{
					case sf::Event::Closed:
					{
						window.close();

						break;
					}
					case sf::Event::KeyPressed:
					{
						switch (event.key.code)
						{
							case sf::Keyboard::H:
							{
								draw_map = 1 - draw_map;
							}
						}
					}
				}
			}

			player.update(map, window);

			steven.update(map, window);

			if (FRAME_DURATION > lag)
			{
				//Drawing the sky.
				//You might say, "Hey, you're just changing the color of the screen."
				//And to that I'll say, "Shut up."
				window.clear(sf::Color(73, 255, 255));

				player.draw_screen(window, steven);

				if (1 == draw_map)
				{
					for (unsigned short a = 0; a < ceil(MAP_CELL_SIZE * MAP_WIDTH / static_cast<float>(MAP_GRID_CELL_SIZE)); a++)
					{
						for (unsigned short b = 0; b < ceil(MAP_CELL_SIZE * MAP_HEIGHT / static_cast<float>(MAP_GRID_CELL_SIZE)); b++)
						{
							map_grid_cell_sprite.setPosition(static_cast<float>(MAP_GRID_CELL_SIZE * a), static_cast<float>(MAP_GRID_CELL_SIZE * b));

							window.draw(map_grid_cell_sprite);
						}
					}

					for (unsigned short a = 0; a < MAP_WIDTH; a++)
					{
						for (unsigned short b = 0; b < MAP_HEIGHT; b++)
						{
							if (Cell::Wall == map[a][b])
							{
								map_wall_sprite.setPosition(static_cast<float>(MAP_CELL_SIZE * a), static_cast<float>(MAP_CELL_SIZE * b));

								window.draw(map_wall_sprite);
							}
						}
					}

					player.draw_map(window);

					steven.draw_map(window);
				}

				window.display();
			}
		}
	}
}