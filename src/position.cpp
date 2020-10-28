/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "position.h"


// ---- Position ---- //

Position::Position() {
	x = 0;
	y = 0;
	z = 0;
}

Position::Position(int x_, int y_, unsigned char z_) : x(x_), y(y_), z(z_) { }

Position::~Position() { }

bool Position::operator==(const Position pos) const {
	if (this->x == pos.x && this->y == pos.y && this->z == pos.z)
		return true;

	return false;
}

bool Position::operator!=(const Position pos) const {
	if (*this == pos)
		return false;

	return true;
}

bool Position::operator<(const Position pos) const {
	if (this->z > pos.z)
		return false;
	else if (this->z == pos.z && this->y > pos.y)
		return false;
	else if (this->z == pos.z && this->y == pos.y && this->x >= pos.x)
		return false;

	return true;
}

bool Position::operator<=(const Position pos) const {
	if (*this == pos || *this < pos)
		return true;

	return false;
}

bool Position::operator>(const Position pos) const {
	if (this->z < pos.z)
		return false;
	else if (this->z == pos.z && this->y < pos.y)
		return false;
	else if (this->z == pos.z && this->y == pos.y && this->x <= pos.x)
		return false;

	return true;
}

bool Position::operator>=(const Position pos) const {
	if (*this == pos || *this > pos)
		return true;

	return false;
}
