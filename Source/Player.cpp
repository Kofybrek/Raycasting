#include <array>
#include <chrono>
#include <SFML/Graphics.hpp>

#include "Headers/DegToRad.hpp"
#include "Headers/GetDegrees.hpp"
#include "Headers/Global.hpp"
#include "Headers/MapCollision.hpp"
#include "Headers/RadToDeg.hpp"
#include "Headers/Steven.hpp"
#include "Headers/Player.hpp"

Player::Player(float i_x, float i_y) :
	direction_horizontal(0),
	direction_vertical(0),
	x(i_x),
	y(i_y),
	map_player_sprite(map_player_texture),
	wall_sprite(wall_texture)
{
	map_player_texture.loadFromFile("Resources/Images/MapPlayer" + std::to_string(MAP_CELL_SIZE) + ".png");
	steven_texture.loadFromFile("Resources/Images/Steven" + std::to_string(CELL_SIZE) + ".png");
	wall_texture.loadFromFile("Resources/Images/Wall" + std::to_string(CELL_SIZE) + ".png");

	steven_sprite.setTexture(steven_texture);
}

void Player::draw_map(sf::RenderWindow& i_window)
{
	//If we have 8 frames, each frame will represent 45 degrees.
	float frame_angle = 360.f * MAP_CELL_SIZE / map_player_texture.getSize().x;
	//We need to shift the angles by half so that their bisector matches the frames.
	float shifted_direction = get_degrees(direction_horizontal + 0.5f * frame_angle);
	float start_x = x + 0.5f * CELL_SIZE;
	float start_y = y + 0.5f * CELL_SIZE;

	//Visualizing the FOV.
	sf::VertexArray fov_visualization(sf::TriangleFan, 1 + SCREEN_WIDTH);
	fov_visualization[0].position = sf::Vector2f(MAP_CELL_SIZE * start_x / CELL_SIZE, MAP_CELL_SIZE * start_y / CELL_SIZE);

	map_player_sprite.setPosition(round(MAP_CELL_SIZE * x / CELL_SIZE), round(MAP_CELL_SIZE * y / CELL_SIZE));
	map_player_sprite.setTextureRect(sf::IntRect(static_cast<unsigned short>(MAP_CELL_SIZE * floor(shifted_direction / frame_angle)), 0, MAP_CELL_SIZE, MAP_CELL_SIZE));

	for (unsigned short a = 0; a < SCREEN_WIDTH; a++)
	{
		float ray_direction = get_degrees(direction_horizontal + FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - 1 - a) / (SCREEN_WIDTH - 1));

		//Finding the endpoint of the ray using trigonometry.
		fov_visualization[1 + a].position = sf::Vector2f(MAP_CELL_SIZE * (start_x + view_rays[a] * cos(deg_to_rad(ray_direction))) / CELL_SIZE, MAP_CELL_SIZE * (start_y - view_rays[a] * sin(deg_to_rad(ray_direction))) / CELL_SIZE);
	}

	i_window.draw(fov_visualization);
	i_window.draw(map_player_sprite);
}

void Player::draw_screen(sf::RenderWindow& i_window, const Steven& i_steven)
{
	bool draw_steven = 0;

	//This is the distance when the height of the projection and the height of the wall in front of us are equal.
	float projection_distance =  0.5f * CELL_SIZE / tan(deg_to_rad(0.5f * FOV_VERTICAL));
	//I guess you can call it a "floor".
	float floor_level = round(0.5f * SCREEN_HEIGHT * (1 + tan(deg_to_rad(direction_vertical)) / tan(deg_to_rad(0.5f * FOV_VERTICAL))));
	float ray_start_x = x + 0.5f * CELL_SIZE;
	float ray_start_y = y + 0.5f * CELL_SIZE;
	//This is the absolute angle between the player to Steven.
	//From Wikipedia:
	//The function atan2 is defined as the angle in the Euclidean plane, given in radians, between the positive x-axis and the ray to the point (x, y) =/= (0, 0).
	float steven_direction = get_degrees(rad_to_deg(atan2(ray_start_y - i_steven.get_center_y(), i_steven.get_center_x() - ray_start_x))) - direction_horizontal;
	//My man Pythagoras is saving the day once again!
	float steven_distance = static_cast<float>(sqrt(pow(ray_start_x - i_steven.get_center_x(), 2) + pow(ray_start_y - i_steven.get_center_y(), 2)));
	
	//The column's position can be negative, so SHRT_MIN.
	short previous_column = SHRT_MIN;

	sf::RectangleShape floor_shape(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT - floor_level));
	floor_shape.setFillColor(sf::Color(36, 219, 0));
	floor_shape.setPosition(0, floor_level);

	//This will make sure that the angle stays between -180 and 180.
	if (-180 >= steven_direction)
	{
		steven_direction += 360;
	}
	else if (180 < steven_direction)
	{
		steven_direction -= 360;
	}

	//If Steven is FAAAAAAAAR away from us or behind us, we don't need to draw him.
	draw_steven = RENDER_DISTANCE >= steven_distance && steven_direction <= 0.75f * FOV_HORIZONTAL && steven_direction >= -0.75f * FOV_HORIZONTAL;

	i_window.draw(floor_shape);

	for (unsigned short a = 0; a < SCREEN_WIDTH; a++)
	{
		//My geometry teacher is probably laughing right now: "I TOLD YOU YOU'LL USE THIS IN REAL LIFE!"
		
		//We're drawing columns that are behind Steven.
		if (0 == (1 == draw_steven && steven_distance > view_rays[a]))
		{
			//When "a" is 0, this'll be +FOV / 2
			//When "a" is SCREEN_WIDTH / 2, this'll be 0
			//When "a" is SCREEN_WIDTH, this'll be -FOV / 2
			float ray_direction = FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - a) / (SCREEN_WIDTH - 1);
			//Finding the intersection between the ray and the projection.
			float ray_projection_position = 0.5f * tan(deg_to_rad(ray_direction)) / tan(deg_to_rad(0.5f * FOV_HORIZONTAL));

			//Current column's position on the screen.
			short current_column = static_cast<short>(round(SCREEN_WIDTH * (0.5f - ray_projection_position)));
			short next_column = SCREEN_WIDTH;

			if (a < SCREEN_WIDTH - 1)
			{
				//I just realized that we're calculating the position of the same column twice because of this.
				//Oh well, I'm too lazy to change it.
				float next_ray_direction = FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - 1 - a) / (SCREEN_WIDTH - 1);

				ray_projection_position = 0.5f * tan(deg_to_rad(next_ray_direction)) / tan(deg_to_rad(0.5f * FOV_HORIZONTAL));

				next_column = static_cast<short>(round(SCREEN_WIDTH * (0.5f - ray_projection_position)));
			}

			//This will prevent us from drawing columns on top of each other.
			if (previous_column < current_column)
			{
				float ray_end_x = ray_start_x + view_rays[a] * cos(deg_to_rad(get_degrees(direction_horizontal + ray_direction)));
				float ray_end_y = ray_start_y - view_rays[a] * sin(deg_to_rad(get_degrees(direction_horizontal + ray_direction)));
				//This is the position of the wall texture column we need to draw.
				float wall_texture_column_x = 0;

				//We don't wanna draw the fog if the wall is at RENDER_DISTANCE / 2 or closer to us.
				//So this will be 255 at the RENDER_DISTANCE or farther.
				//255-0 at the RENDER_DISTANCE and RENDER_DISTANCE / 2.
				//0 at the RENDER_DISTANCE / 2 or closer.
				unsigned char brightness = static_cast<unsigned char>(round(255 * std::max<float>(0, 2 * view_rays[a] / RENDER_DISTANCE - 1)));

				//Using cosine to prevent the fisheye effect.
				unsigned short column_height = static_cast<unsigned short>(SCREEN_HEIGHT * projection_distance / (view_rays[a] * cos(deg_to_rad(ray_direction))));

				//This is our "fog". We're drawing a transparent column with the color of the sky.
				sf::RectangleShape shape(sf::Vector2f(std::max(1, next_column - current_column), column_height));
				shape.setFillColor(sf::Color(73, 255, 255, brightness));
				shape.setPosition(current_column, round(floor_level - 0.5f * column_height));

				previous_column = current_column;

				//Checking if the ray hit was vertical or horizontal.
				if (abs(ray_end_x - CELL_SIZE * round(ray_end_x / CELL_SIZE)) < abs(ray_end_y - CELL_SIZE * round(ray_end_y / CELL_SIZE)))
				{
					wall_texture_column_x = ray_end_y - CELL_SIZE * floor(ray_end_y / CELL_SIZE);
				}
				else
				{
					wall_texture_column_x = CELL_SIZE * ceil(ray_end_x / CELL_SIZE) - ray_end_x;
				}

				wall_sprite.setPosition(current_column, round(floor_level - 0.5f * column_height));
				wall_sprite.setTextureRect(sf::IntRect(static_cast<unsigned short>(round(wall_texture_column_x)), 0, 1, CELL_SIZE));
				wall_sprite.setScale(std::max(1, next_column - current_column), column_height / static_cast<float>(CELL_SIZE));

				i_window.draw(wall_sprite);
				i_window.draw(shape);
			}
		}
	}

	if (1 == draw_steven)
	{
		float frame_angle = 360.f * CELL_SIZE / steven_texture.getSize().x;
		//We're getting Steven's direction relative to ours.
		float shifted_direction = get_degrees(i_steven.get_direction() + 0.5f * (180 + frame_angle) - direction_horizontal - steven_direction);
		float steven_projection_position = 0.5f * tan(deg_to_rad(steven_direction)) / tan(deg_to_rad(0.5f * FOV_HORIZONTAL));

		short steven_screen_x = static_cast<short>(round(SCREEN_WIDTH * (0.5f - steven_projection_position)));

		unsigned short steven_size = static_cast<unsigned short>(SCREEN_HEIGHT * projection_distance / (steven_distance * cos(deg_to_rad(steven_direction))));
		
		previous_column = SHRT_MIN;

		steven_sprite.setColor(sf::Color(255, 255, 255, static_cast<unsigned char>(round(255 * std::min<float>(1, 2 * (1 - steven_distance / RENDER_DISTANCE))))));
		steven_sprite.setPosition(round(steven_screen_x - 0.5f * steven_size), round(floor_level - 0.5f * steven_size));
		steven_sprite.setScale(steven_size / static_cast<float>(CELL_SIZE), steven_size / static_cast<float>(CELL_SIZE));
		steven_sprite.setTextureRect(sf::IntRect(static_cast<unsigned short>(CELL_SIZE * floor(shifted_direction / frame_angle)), 0, CELL_SIZE, CELL_SIZE));

		i_window.draw(steven_sprite);

		for (unsigned short a = 0; a < SCREEN_WIDTH; a++)
		{
			//We're drawing columns that are closer than Steven.
			if (steven_distance > view_rays[a])
			{
				float ray_direction = FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - a) / (SCREEN_WIDTH - 1);
				float ray_projection_position = 0.5f * tan(deg_to_rad(ray_direction)) / tan(deg_to_rad(0.5f * FOV_HORIZONTAL));

				short current_column = static_cast<short>(round(SCREEN_WIDTH * (0.5f - ray_projection_position)));
				short next_column = SCREEN_WIDTH;

				if (a < SCREEN_WIDTH - 1)
				{
					float next_ray_direction = FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - 1 - a) / (SCREEN_WIDTH - 1);

					ray_projection_position = 0.5f * tan(deg_to_rad(next_ray_direction)) / tan(deg_to_rad(0.5f * FOV_HORIZONTAL));

					next_column = static_cast<short>(round(SCREEN_WIDTH * (0.5f - ray_projection_position)));
				}

				if (previous_column < current_column)
				{
					float ray_end_x = ray_start_x + view_rays[a] * cos(deg_to_rad(get_degrees(direction_horizontal + ray_direction)));
					float ray_end_y = ray_start_y - view_rays[a] * sin(deg_to_rad(get_degrees(direction_horizontal + ray_direction)));
					float wall_texture_column_x = 0;

					unsigned char brightness = static_cast<unsigned char>(round(255 * std::max<float>(0, 2 * view_rays[a] / RENDER_DISTANCE - 1)));

					unsigned short column_height = static_cast<unsigned short>(SCREEN_HEIGHT * projection_distance / (view_rays[a] * cos(deg_to_rad(ray_direction))));
					
					sf::RectangleShape shape(sf::Vector2f(std::max(1, next_column - current_column), column_height));
					shape.setFillColor(sf::Color(73, 255, 255, brightness));
					shape.setPosition(current_column, round(floor_level - 0.5f * column_height));

					previous_column = current_column;

					if (abs(ray_end_x - CELL_SIZE * round(ray_end_x / CELL_SIZE)) < abs(ray_end_y - CELL_SIZE * round(ray_end_y / CELL_SIZE)))
					{
						wall_texture_column_x = ray_end_y - CELL_SIZE * floor(ray_end_y / CELL_SIZE);
					}
					else
					{
						wall_texture_column_x = CELL_SIZE * ceil(ray_end_x / CELL_SIZE) - ray_end_x;
					}

					wall_sprite.setPosition(current_column, round(floor_level - 0.5f * column_height));
					wall_sprite.setTextureRect(sf::IntRect(static_cast<unsigned short>(round(wall_texture_column_x)), 0, 1, CELL_SIZE));
					wall_sprite.setScale(std::max(1, next_column - current_column), column_height / static_cast<float>(CELL_SIZE));

					i_window.draw(wall_sprite);
					i_window.draw(shape);
				}
			}
		}
	}
}

void Player::set_position(float i_x, float i_y)
{
	x = i_x;
	y = i_y;
}

void Player::update(const std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH>& i_map, const sf::RenderWindow& i_window)
{
	float rotation_horizontal = 0;
	float rotation_vertical = 0;
	float step_x = 0;
	float step_y = 0;

	unsigned short window_center_x = static_cast<unsigned short>(round(0.5f * i_window.getSize().x));
	unsigned short window_center_y = static_cast<unsigned short>(round(0.5f * i_window.getSize().y));

	//Mouse control!
	//By the way, do I need to write comments? Can't you just watch my video? I've already explained everything there.
	rotation_horizontal = FOV_HORIZONTAL * (window_center_x - sf::Mouse::getPosition(i_window).x) / i_window.getSize().x;
	rotation_vertical = FOV_VERTICAL * (window_center_y - sf::Mouse::getPosition(i_window).y) / i_window.getSize().y;

	direction_horizontal = get_degrees(direction_horizontal + rotation_horizontal);
	//Putting 90 here breaks the game so I put 89.
	direction_vertical = std::clamp<float>(direction_vertical + rotation_vertical, -89, 89);

	//Just so you know, this works even if the window is out of focus.
	sf::Mouse::setPosition(sf::Vector2i(window_center_x, window_center_y), i_window);

	if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		step_x = MOVEMENT_SPEED * cos(deg_to_rad(get_degrees(90 + direction_horizontal)));
		step_y = -MOVEMENT_SPEED * sin(deg_to_rad(get_degrees(90 + direction_horizontal)));
	}
	else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		step_x = MOVEMENT_SPEED * cos(deg_to_rad(get_degrees(direction_horizontal - 90)));
		step_y = -MOVEMENT_SPEED * sin(deg_to_rad(get_degrees(direction_horizontal - 90)));
	}
	
	if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		step_x -= MOVEMENT_SPEED * cos(deg_to_rad(direction_horizontal));
		step_y += MOVEMENT_SPEED * sin(deg_to_rad(direction_horizontal));
	}
	else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		step_x += MOVEMENT_SPEED * cos(deg_to_rad(direction_horizontal));
		step_y -= MOVEMENT_SPEED * sin(deg_to_rad(direction_horizontal));
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
		//We're placing the player on the nearest cell so that it touches the wall.
		x = CELL_SIZE * round(x / CELL_SIZE);
		y = CELL_SIZE * round(y / CELL_SIZE);
	}

	for (unsigned short a = 0; a < SCREEN_WIDTH; a++)
	{
		char cell_step_x = 0;
		char cell_step_y = 0;

		float ray_direction = get_degrees(direction_horizontal + FOV_HORIZONTAL * (floor(0.5f * SCREEN_WIDTH) - a) / (SCREEN_WIDTH - 1));
		float ray_direction_x = cos(deg_to_rad(ray_direction));
		float ray_direction_y = -sin(deg_to_rad(ray_direction));
		//This is the value we need.
		float ray_length = 0;
		float ray_start_x = x + 0.5f * CELL_SIZE;
		float ray_start_y = y + 0.5f * CELL_SIZE;
		//This ray checks for horizontal collisions.
		float x_ray_length = 0;
		//This ray checks for vertical collisions.
		float y_ray_length = 0;
		//This is the length of the ray after moving 1 unit along the x-axis.
		float x_ray_unit_length = static_cast<float>(CELL_SIZE * sqrt(1 + pow(ray_direction_y / ray_direction_x, 2)));
		//This is the length of the ray after moving 1 unit along the y-axis.
		float y_ray_unit_length = static_cast<float>(CELL_SIZE * sqrt(1 + pow(ray_direction_x / ray_direction_y, 2)));

		unsigned char current_cell_x = static_cast<unsigned char>(floor(ray_start_x / CELL_SIZE));
		unsigned char current_cell_y = static_cast<unsigned char>(floor(ray_start_y / CELL_SIZE));

		if (0 > ray_direction_x)
		{
			cell_step_x = -1;

			//In order for the algorithm to work, the ray must start at the cell borders.
			//So if the starting position of the ray is not a cell border (which is very likely), we'll stretch it to the closest one.
			x_ray_length = x_ray_unit_length * (ray_start_x / CELL_SIZE - current_cell_x);
		}
		else if (0 < ray_direction_x)
		{
			cell_step_x = 1;

			x_ray_length = x_ray_unit_length * (1 + current_cell_x - ray_start_x / CELL_SIZE);
		}
		else
		{
			cell_step_x = 0;
		}

		if (0 > ray_direction_y)
		{
			cell_step_y = -1;

			y_ray_length = y_ray_unit_length * (ray_start_y / CELL_SIZE - current_cell_y);
		}
		else if (0 < ray_direction_y)
		{
			cell_step_y = 1;

			y_ray_length = y_ray_unit_length * (1 + current_cell_y - ray_start_y / CELL_SIZE);
		}
		else
		{
			cell_step_y = 0;
		}

		//We continue casting the ray until it reaches the render distance.
		while (RENDER_DISTANCE >= ray_length)
		{
			//In case the ray hits a corner (which is very unlikely, but I like to add this kind of stuff just in case).
			bool corner_collision = 0;

			//We stretch the shortest ray.
			if (x_ray_length < y_ray_length)
			{
				ray_length = x_ray_length;
				x_ray_length += x_ray_unit_length;

				current_cell_x += cell_step_x;
			}
			else if (x_ray_length > y_ray_length)
			{
				ray_length = y_ray_length;
				y_ray_length += y_ray_unit_length;

				current_cell_y += cell_step_y;
			}
			else
			{
				//If the rays are equal, that means we hit the corner, so we stretch both rays.
				corner_collision = 1;

				ray_length = x_ray_length;
				x_ray_length += x_ray_unit_length;
				y_ray_length += y_ray_unit_length;

				current_cell_x += cell_step_x;
				current_cell_y += cell_step_y;
			}

			//Making sure the current cell we're checking is inside our map.
			if (0 <= current_cell_x && 0 <= current_cell_y && MAP_HEIGHT > current_cell_y && MAP_WIDTH > current_cell_x)
			{
				if (Cell::Wall == i_map[current_cell_x][current_cell_y])
				{
					//We stop casting the ray if it hits the wall.
					break;
				}
				else if (1 == corner_collision)
				{
					//The ray can't go through 2 walls standing diagonally.
					if (Cell::Wall == i_map[current_cell_x - cell_step_x][current_cell_y] && Cell::Wall == i_map[current_cell_x][current_cell_y - cell_step_y])
					{
						break;
					}
				}
			}
		}

		//The ray's length must be less than or equal to the render distance.
		ray_length = std::min(RENDER_DISTANCE, ray_length);

		view_rays[a] = ray_length;
	}
}