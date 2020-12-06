#include "zbuffer.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <time.h>
#include <unordered_set>
int type;


ActiveEdge* merge_two_lists(ActiveEdge* l1, ActiveEdge* l2) {
	ActiveEdge* dummy = new ActiveEdge(), * p = dummy;
	dummy->x = INT_MIN;
	while (l1 && l2) {
		if (l1->x > l2->x) std::swap(l1, l2);
		else if (l1->x == l2->x && l1->dx > l2->dx) std::swap(l1, l2);
		p->next = l1;
		p = l1;
		l1 = l1->next;
	}
	p->next = (l1 == nullptr) ? l2 : l1;
	return dummy->next;
}

ActiveEdge* find_mid(ActiveEdge* head) {
	ActiveEdge* fast = head, * slow = head;
	while (fast && fast->next && fast->next->next) {
		fast = fast->next->next;
		slow = slow->next;
	}
	return slow;
}

ActiveEdge* sortList(ActiveEdge* head) {
	if (head == nullptr || head->next == nullptr) return head;
	ActiveEdge* mid = find_mid(head), * head2 = mid->next;
	mid->next = nullptr;
	return merge_two_lists(sortList(head), sortList(head2));
}




void getPlaneEquation(const Point& p0, const Point& p1, const Point& p2, double &a, double &b, double &c) {
	double v1[3] = { p1._xx - p0._xx, p1._yy - p0._yy, p1._zz - p0._zz }, v2[3] = { p2._xx - p0._xx, p2._yy - p0._yy, p2._zz - p0._zz };
	double cross[3] = { v1[1] * v2[2] - v1[2] * v2[1],v1[2] * v2[0] - v1[0] * v2[2],v1[0] * v2[1] - v1[1] * v2[0] };
	a = cross[0], b = cross[1], c = cross[2];
}


void zbuffer::init(int h, int w) {
	while (1) {
		std::cout << "Please Choose a number\n0: volleyball\n1: Darth Sidious\n\n";
		std::cin >> type;
		if (type >= 0 && type <= 2) break;
	}
	winHeight = h, winWidth = w;
	PolygonTable.resize(winHeight);
	EdgeTable.resize(winHeight);
	buffer.resize(winHeight, std::vector<int>(winWidth, -1));
	std::fstream fin;
	if (type == 0) fin.open("resource\\PaiQiu_Mid.obj", std::ios::in);
	if (type == 1) fin.open("resource\\Darth_Sidious.obj", std::ios::in);
	if (type == 2) fin.open("resource\\00.obj", std::ios::in);
	std::string line, s;
	std::vector<Normal> N;
	double x, y, z;
	while (getline(fin, line)) {
		std::stringstream ss(line);
		ss >> s;
		if (s == "v") {      // vertex
			ss >> x >> y >> z;
			P.push_back(Point(x, y, z));
		}
		else if (s == "vn") {      // vertex
			ss >> x >> y >> z;
			N.push_back(Normal(x, y, z));
		}
		else if (s == "f") { // face
			F.push_back(Face());
			while (ss >> s) {
				int p0 = s.find('/'), p1 = s.find('/', p0 + 1);
				int pnt_index = stoi(s.substr(0, p0));  //读取顶点索引部分
				//int color_index = stoi(s.substr(p0 + 1, p1 - p0 - 1)); //读取颜色索引部分
				int normal_index = stoi(s.substr(p1 + 1)); //读取颜色索引部分
				F.back()._pnts.push_back(pnt_index-1);
				F.back()._normal.push_back(normal_index - 1);
			}
			F.back().getrandcolor(N);
		}
	}
}


void zbuffer::CreatPolygonTableAndEdgeTable() {
	for (int id = 0; id < F.size(); ++id) {
		std::vector<int>& p = F[id]._pnts;
		int ymax = INT_MIN, ymin = INT_MAX;
		double a, b, c;
		getPlaneEquation(P[p[0]], P[p[1]], P[p[2]], a, b, c);
		for (int i = 0; i < 3; ++i) {
			ymax = std::max(P[p[i]]._y, ymax);
			ymin = std::min(P[p[i]]._y, ymin);
			for (int j = 0; j < i; ++j) {
				auto p0 = P[p[i]], p1 = P[p[j]];
				if (p0._y == p1._y) continue;
				if (p0._y < p1._y) std::swap(p0, p1);
				EdgeTable[p0._y].push_back(Edge(p0._x, p1._y, (p0._xx - p1._xx) / (p1._yy - p0._yy), id, a, b, c, p0._zz));
			}
		}
		PolygonTable[ymax].push_back(Polygon{ ymin, ymax, id, a,b,c });
	}
}


void zbuffer::sacnline() {
	ActivePolygon* AP_head = new ActivePolygon();
	ActiveEdge* AE_head = new ActiveEdge();
	for (int line_y = 719; line_y >= 0; --line_y) {
		std::vector<double> m(winWidth, INT_MIN);

		//加入新多边形到AP链表
		for (const auto& polygon : PolygonTable[line_y]) {
			ActivePolygon *pAP = new ActivePolygon(polygon);
			if (AP_head->next != nullptr) {
				AP_head->next->prev = pAP;
				pAP->next = AP_head->next;
			}
			AP_head->next = pAP;
			pAP->prev = AP_head;
		}
		

		//从AP链表中删除结束的多边形
		for (auto pAP = AP_head->next; pAP != nullptr; pAP = pAP->next) {
			if (pAP->ymin == line_y) {
				if (pAP->next != nullptr) pAP->next->prev = pAP->prev;
				pAP->prev->next = pAP->next;
				//在AE链表中删除结束的边
				auto pAE = F[pAP->id].pAE;
				if (pAE == nullptr) continue;
				if (pAE->next != nullptr) pAE->next->prev = pAE->prev;
				pAE->prev->next = pAE->next;
				F[pAP->id].pAE = nullptr;
			}
		}



		//加入新边到AE链表
		for (auto& edge : EdgeTable[line_y]) {
			if (F[edge.id].pAE == nullptr) {//从未加入到AE链表中
				ActiveEdge* pAE = new ActiveEdge();
				F[edge.id].pAE = pAE;
				pAE->ylmin = pAE->yrmin = edge.ymin;
				pAE->update_left(edge);
				if (AE_head->next != nullptr) {
					AE_head->next->prev = pAE;
					pAE->next = AE_head->next;
				}
				AE_head->next = pAE;
				pAE->prev = AE_head;
			}
			else {  //已经加入过 则更新它
				ActiveEdge* pAE = F[edge.id].pAE;
				if (pAE->ylmin == line_y) {  //更新左边
					pAE->update_left(edge);
				}
				else {                       //更新右边
					pAE->update_right(edge);
				}
			}
		}

	    //跑AE链表 完成z值更新
		for (auto pAE = AE_head->next; pAE != nullptr; pAE = pAE->next) {
			double z = pAE->zl;
			if (pAE->xl > pAE->xr) {
				pAE->change();
			}
			for (int x = pAE->xl; x <= pAE->xr; ++x) {
				if (z > m[x]) {
					m[x] = z;
					buffer[line_y][x] = pAE->id;
				}
				z += pAE->dzx;
			}
			pAE->xl += pAE->dxl;
			pAE->xr += pAE->dxr;
			pAE->zl += pAE->dzx * pAE->dxl + pAE->dzy;
		}
	}
}

std::unordered_set<int> IN;

void zbuffer::zufferscanline() {
	ActivePolygon* AP_head = new ActivePolygon();
	ActiveEdge* AE_head = new ActiveEdge();
	for (int line_y = 719; line_y >= 0; --line_y) {
		//加入新多边形到AP链表
		for (const auto& polygon : PolygonTable[line_y]) {
			ActivePolygon* pAP = new ActivePolygon(polygon);
			pAP->in_out_flag = true;
			//IN.insert(pAP->id);
			if (AP_head->next != nullptr) {
				AP_head->next->prev = pAP;
				pAP->next = AP_head->next;
			}
			AP_head->next = pAP;
			pAP->prev = AP_head;
		}


		//从AP链表中删除结束的多边形
		for (auto pAP = AP_head->next; pAP != nullptr; pAP = pAP->next) {
			if (pAP->ymin == line_y) {
				pAP->in_out_flag = false;
				//IN.erase(pAP->id);
				if (pAP->next != nullptr) pAP->next->prev = pAP->prev;
				pAP->prev->next = pAP->next;
				//在AE链表中删除结束的边
				auto pAE = F[pAP->id].pAE_L;
				if (pAE != nullptr) {
					if (pAE->next != nullptr) pAE->next->prev = pAE->prev;
					pAE->prev->next = pAE->next;
				}
				pAE = F[pAP->id].pAE_R;
				if (pAE != nullptr) {
					if (pAE->next != nullptr) pAE->next->prev = pAE->prev;
					pAE->prev->next = pAE->next;
					F[pAP->id].pAE_L = F[pAP->id].pAE_R = nullptr;
				}
			}
		}



		//加入新边到AE链表
		for (auto& edge : EdgeTable[line_y]) {
			if (F[edge.id].pAE_L == nullptr) {//从未加入到AE链表中
				ActiveEdge* pAE = new ActiveEdge();
				F[edge.id].pAE_L = pAE;
				pAE->update(edge);
				if (AE_head->next != nullptr) {
					AE_head->next->prev = pAE;
					pAE->next = AE_head->next;
				}
				AE_head->next = pAE;
				pAE->prev = AE_head;
			}
			else if (F[edge.id].pAE_R == nullptr) {//从未加入到AE链表中
				ActiveEdge* pAE = new ActiveEdge();
				F[edge.id].pAE_R = pAE;
				pAE->update(edge);
				if (AE_head->next != nullptr) {
					AE_head->next->prev = pAE;
					pAE->next = AE_head->next;
				}
				AE_head->next = pAE;
				pAE->prev = AE_head;
			}
			else {  //已经加入过 则更新它
				ActiveEdge* pAE = F[edge.id].pAE_L;
				if (pAE->ymin == line_y) {  //更新左边
					pAE->update(edge);
				}
				else {                       //更新右边
					pAE = F[edge.id].pAE_R;
					pAE->update(edge);
				}
			}

			if (F[edge.id].pAE_L && F[edge.id].pAE_R) {
				F[edge.id].pAE_L->in_out_edge = true;
				F[edge.id].pAE_R->in_out_edge = false;
			}
		}

		AE_head->next = sortList(AE_head->next);
	
		for (auto pAE = AE_head; pAE != nullptr; pAE = pAE->next) {   //改变F[id]的指针
			if (pAE->next != nullptr) pAE->next->prev = pAE;
			if (pAE == AE_head) continue;
			if (pAE->in_out_edge) F[pAE->id].pAE_L = pAE;
			else F[pAE->id].pAE_R = pAE;
		}
		
		for (auto pAP = AP_head->next; pAP != nullptr; pAP = pAP->next) {
			//更新完毕后，通过比较x值确认哪个是in, 哪个是out
			if (F[pAP->id].pAE_L && F[pAP->id].pAE_R) {
				if (F[pAP->id].pAE_L->x > F[pAP->id].pAE_R->x ||
					F[pAP->id].pAE_L->x == F[pAP->id].pAE_R->x && F[pAP->id].pAE_L->dx > F[pAP->id].pAE_R->dx) {
					std::swap(F[pAP->id].pAE_L, F[pAP->id].pAE_R);
				}
				F[pAP->id].pAE_L->in_out_edge = true;
				F[pAP->id].pAE_R->in_out_edge = false;
			}
		}
		
		auto e1 = AE_head->next;
		int face_id = e1 ? e1->id : -1;
		double cur_z = e1 ? e1->z : INT_MIN;
		
		
		while (e1 && e1->next != nullptr) {
			auto e2 = e1->next;
			for (auto x = e1->x; x < e2->x; ++x) {
				buffer[line_y][x] = face_id;
			}

			if (e2->in_out_edge) {   //in
				IN.insert(e2->id);
				if (e2->z > cur_z) {  //进入的z是最大的
					cur_z = e2->z;
					face_id = e2->id;
				}
			}
			else {                   //out
				IN.erase(e2->id);
				if (e2->id == face_id && e2->next != nullptr) {  //出去的z是最大的
					cur_z = INT_MIN;
					face_id = -1;
					for (auto& id : IN) { //看看在该点谁的z最大
						double zz = F[id].pAE_L->z + (e2->x - F[id].pAE_L->x) * F[id].pAE_L->dzx;
						if (zz > cur_z) {
							cur_z = zz;
							face_id = id;
						}
					}
				}
				
			}
			e1 = e2;
		} 
		
		for (auto pAE = AE_head->next; pAE != nullptr; pAE = pAE->next) {
			pAE->x += pAE->dx;
			pAE->z += pAE->dzx * pAE->dx + pAE->dzy;
		}
	}
}