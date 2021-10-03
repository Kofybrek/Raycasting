#include <chrono>

#include "Headers/DegToRad.hpp"
#include "Headers/Global.hpp"

float deg_to_rad(float i_degrees)
{
	//After googling I found out that smart people use radians because of calculus.
	//I don't know what that word means so I'll keep using degrees.
	return PI * i_degrees / 180;
}