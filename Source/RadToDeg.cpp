#include <chrono>

#include "Headers/RadToDeg.hpp"
#include "Headers/Global.hpp"

float rad_to_deg(float i_radians)
{
	return 180 * i_radians / PI;
}