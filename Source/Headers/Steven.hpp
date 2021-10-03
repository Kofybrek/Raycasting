#pragma once

class Steven
{
	float direction;
	float x;
	float y;

	sf::Sprite map_steven_sprite;

	sf::Texture map_steven_texture;
public:
	Steven(float i_x, float i_y);

	float get_center_x() const;
	float get_center_y() const;
	float get_direction() const;

	void draw_map(sf::RenderWindow& i_window);
	void set_position(float i_x, float i_y);
	void update(const std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH>& i_map, const sf::RenderWindow& i_window);
};