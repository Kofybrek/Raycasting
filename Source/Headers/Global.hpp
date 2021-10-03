#pragma once

constexpr float FOV_HORIZONTAL = 90;
constexpr float FOV_VERTICAL = 58.75f;
constexpr float MOVEMENT_SPEED = 2;
//Since C++ stores floating numbers differently, this is the closest PI value we can use.
constexpr float PI = 3.141592653589793116f;
constexpr float RENDER_DISTANCE = 1024;
constexpr float STEVEN_ROTATION_SPEED = 2;

//The size of the cell in the game.
constexpr unsigned char CELL_SIZE = 64;
//The size of the cell in the minimap.
constexpr unsigned char MAP_CELL_SIZE = 8;
constexpr unsigned char MAP_GRID_CELL_SIZE = 16;
constexpr unsigned char MAP_HEIGHT = 24;
constexpr unsigned char MAP_WIDTH = 40;
constexpr unsigned char SCREEN_RESIZE = 1;

constexpr unsigned short SCREEN_HEIGHT = 720;
constexpr unsigned short SCREEN_WIDTH = 1280;

//If we divide 1 second by 60 frames (60 FPS), the duration of each frame will be 16.667 ms.
constexpr std::chrono::microseconds FRAME_DURATION(16667);

//When I started this project, I thought I would have tons of different cells, so I decided to use enum. I only used 2.
enum Cell
{
	Empty,
	Wall
};