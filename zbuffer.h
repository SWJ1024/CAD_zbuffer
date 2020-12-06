#pragma once
#include <vector>
#include <iostream>
extern int type;


class Point {
public:
	Point(double x, double y, double z) :_xx(x), _yy(y), _zz(z) {
		if (type == 0) _x = 10 * x + 500, _y = 10 * y + 50;
		if (type == 1) _x = 180 * x + 500, _y = 180 * y + 30;
		if (type == 2) _x = 4 * x + 500, _y = 4 * y + 50;
	}
	double _xx, _yy, _zz;
	int _x, _y;
};



class Edge {
public:
	Edge(int x, int ymin, double k, int id, double a, double b, double c, double z) : x(x), ymin(ymin), dx(k), id(id), a(a), b(b), c(c), z(z) {};
	double a, b, c;
	double z;    //上顶点的深度
	double dx;   //-1/k
	int x, ymin, id;
};



class ActiveEdge {
public:
	ActiveEdge() :prev(nullptr), next(nullptr) {}
	void update_left(Edge& edge) {
		xl = edge.x;
		ylmin = edge.ymin;
		id = edge.id;
		dxl = edge.dx;
		zl = edge.z;
		if (type == 0) {
			dzx = -edge.a / edge.c / 10;
			dzy = edge.b / edge.c / 10;
		}
		if (type == 1) {
			dzx = -edge.a / edge.c / 180;
			dzy = edge.b / edge.c / 180;
		}
		if (type == 2) {
			dzx = -edge.a / edge.c / 4;
			dzy = edge.b / edge.c / 4;
		}
	}
	void update_right(Edge& edge) {
		xr = edge.x;
		yrmin = edge.ymin;
		id = edge.id;
		dxr = edge.dx;
		zr = edge.z;
	}
	void update(Edge& edge) {
		x = edge.x;
		z = edge.z;
		ymin = edge.ymin;
		dx = edge.dx;
		id = edge.id;
		if (type == 0) {
			dzx = -edge.a / edge.c / 10;
			dzy = edge.b / edge.c / 10;
		}
		if (type == 1) {
			dzx = -edge.a / edge.c / 180;
			dzy = edge.b / edge.c / 180;
		}
		if (type == 2) {
			dzx = -edge.a / edge.c / 4;
			dzy = edge.b / edge.c / 4;
		}
	}
	void change() {
		std::swap(xl, xr);
		std::swap(ylmin, yrmin);
		std::swap(zl, zr);
		std::swap(dxl, dxr);
	}
	ActiveEdge* prev, * next;
	double xl, xr, ylmin, yrmin;

	double dxl, dxr, zl, zr;
	double dzx, dzy;
	int id;

	bool in_out_edge;
	double x, ymin;
	double dx, z;
};



class Normal {
public:
	Normal(double x, double y, double z) :_xx(x), _yy(y), _zz(z) {}
	double _xx, _yy, _zz;
};



class Face {
public:
	std::vector<int> _pnts;
	std::vector<int> _normal;
	double _color[3];
	ActiveEdge* pAE = nullptr, *pAE_L = nullptr, *pAE_R = nullptr;
	//bool in_out_flag = false;
	void getrandcolor(std::vector<Normal> &N) {
		double x = 0, y = 0, z = 0;
		for (int i = 0; i < 3; ++i) {
			x += N[_normal[i]]._xx;
			y += N[_normal[i]]._yy;
			z += N[_normal[i]]._zz;
		}
		if (z > 0) {

			_color[0] = x/3;
			_color[1] = y/3;
			_color[2] = z/3;
		}
		else {

			_color[0] = 0.3;
			_color[1] = 0.1;
			_color[2] = 0.1;
		}
	}
};



class Polygon {
public:
	Polygon(int ymin, int ymax, int id, double a, double b, double c) :ymin(ymin), ymax(ymax), id(id), a(a), b(b), c(c) {};
	double a, b, c;
	int ymin, ymax, id;
};



class ActivePolygon {
public:
	ActivePolygon() :prev(nullptr), next(nullptr) {}
	ActivePolygon(Polygon p) :id(p.id), ymin(p.ymin), prev(nullptr), next(nullptr) {}
	ActivePolygon* prev, *next;
	bool in_out_flag;
	int id;
	int ymin;
};



class zbuffer {
public:
	void init(int winHeight, int winWidth);
	void CreatPolygonTableAndEdgeTable();
	void sacnline();
	void zufferscanline();

	int winHeight, winWidth;
	std::vector<std::vector<int>> buffer;
	std::vector<std::vector<Polygon>> PolygonTable;
	std::vector<std::vector<Edge>> EdgeTable;
	std::vector<Face> F;
	std::vector<Point> P;
};