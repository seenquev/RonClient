/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __POSITION_H_
#define __POSITION_H_

class Position {
public:
	int x;
	int y;
	unsigned char z;
	
public:
	Position();
	Position(int x_, int y_, unsigned char z_);
	~Position();
	
	bool operator==(const Position pos) const;
	bool operator!=(const Position pos) const;
	bool operator<(const Position pos) const;
	bool operator<=(const Position pos) const;
	bool operator>(const Position pos) const;
	bool operator>=(const Position pos) const;
};

#endif //__POSITION_H_
