#include <chrono>

#include "Headers/GetDegrees.hpp"
#include "Headers/Global.hpp"

float get_degrees(float i_degrees)
{
	//Thank you, Stackoverflow!
	return static_cast<float>(fmod(360 + fmod(i_degrees, 360), 360));
}