#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "Headers/DegToRad.hpp"
#include "Headers/GetDegrees.hpp"
#include "Headers/Global.hpp"
#include "Headers/MapCollision.hpp"
#include "Headers/Steven.hpp"

Steven::Steven(float i_x, float i_y) :
	direction(0),
	x(i_x),
	y(i_y),
	map_steven_sprite(map_steven_texture)
{
	map_steven_texture.loadFromFile("Resources/Images/MapSteven" + std::to_string(MAP_CELL_SIZE) + ".png");
}

float Steven::get_center_x() const
{
	return x + 0.5f * CELL_SIZE;
}

float Steven::get_center_y() const
{
	return y + 0.5f * CELL_SIZE;
}

float Steven::get_direction() const
{
	return direction;
}

void Steven::draw_map(sf::RenderWindow& i_window)
{
	float frame_angle = 360.f * MAP_CELL_SIZE / map_steven_texture.getSize().x;
	float shifted_direction = get_degrees(direction + 0.5f * frame_angle);

	map_steven_sprite.setPosition(round(MAP_CELL_SIZE * x / CELL_SIZE), round(MAP_CELL_SIZE * y / CELL_SIZE));
	map_steven_sprite.setTextureRect(sf::IntRect(static_cast<unsigned short>(MAP_CELL_SIZE * floor(shifted_direction / frame_angle)), 0, MAP_CELL_SIZE, MAP_CELL_SIZE));

	i_window.draw(map_steven_sprite);
}

void Steven::set_position(float i_x, float i_y)
{
	x = i_x;
	y = i_y;
}

void Steven::update(const std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH>& i_map, const sf::RenderWindow& i_window)
{
	float step_x = 0;
	float step_y = 0;

	if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		direction += STEVEN_ROTATION_SPEED;
	}
	else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		direction -= STEVEN_ROTATION_SPEED;
	}

	direction = get_degrees(direction);

	if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		step_x -= MOVEMENT_SPEED * cos(deg_to_rad(direction));
		step_y += MOVEMENT_SPEED * sin(deg_to_rad(direction));
	}
	else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		step_x += MOVEMENT_SPEED * cos(deg_to_rad(direction));
		step_y -= MOVEMENT_SPEED * sin(deg_to_rad(direction));
	}

	if (0 == map_collision(step_x + x, step_y + y, i_map))
	{
		x += step_x;
		y += step_y;
	}
	else if (0 == map_collision(step_x + x, y, i_map))
	{
		x += step_x;
		y = CELL_SIZE * round(y / CELL_SIZE);
	}
	else if (0 == map_collision(x, step_y + y, i_map))
	{
		x = CELL_SIZE * round(x / CELL_SIZE);
		y += step_y;
	}
	else
	{
		x = CELL_SIZE * round(x / CELL_SIZE);
		y = CELL_SIZE * round(y / CELL_SIZE);
	}
}