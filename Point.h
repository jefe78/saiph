#ifndef POINT_H
#define POINT_H

#include <iostream>

#include "Globals.h"

class Point {
public:
	Point(int row = -1, int col = -1);

	int col() const;
	int col(int col);
	int row() const;
	int row(int row);
	bool insideMap() const;
	Point& moveNorth();
	Point& moveNortheast();
	Point& moveEast();
	Point& moveSoutheast();
	Point& moveSouth();
	Point& moveSouthwest();
	Point& moveWest();
	Point& moveNorthwest();

	bool operator<(const Point& p) const;
	bool operator>(const Point& p) const;
	bool operator<=(const Point& p) const;
	bool operator>=(const Point& p) const;
	bool operator==(const Point& p) const;
	bool operator!=(const Point& p) const;

private:
	int _row;
	int _col;
};

std::ostream & operator<<(std::ostream& os, const Point& p);
#endif
