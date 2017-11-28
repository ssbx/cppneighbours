//============================================================================
// Name        : cppneighbours.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdint.h>
#include <stdio.h>
using namespace std;

const uint16_t ctab[] = {
#define Z(a) a,a+1,a+256,a+257
#define Y(a) Z(a),Z(a+2),Z(a+512),Z(a+514)
#define X(a) Y(a),Y(a+4),Y(a+1024),Y(a+1028)
	X(0), X(8), X(2048), X(2056)
#undef X
#undef Y
#undef Z
};

int ilog2(long arg) {
#ifdef __GNUC__
	if (arg == 0)
		return 0;
	return 8 * sizeof(long) - 1 - __builtin_clzl(arg);
#endif
	int res = 0;
	while (arg > 0xFFFF) {
		res += 16;
		arg >>= 16;
	}
	if (arg > 0x00FF) {
		res |= 8;
		arg >>= 8;
	}
	if (arg > 0x000F) {
		res |= 4;
		arg >>= 4;
	}
	if (arg > 0x0003) {
		res |= 2;
		arg >>= 2;
	}
	if (arg > 0x0001) {
		res |= 1;
	}
	return res;
}

int nside2order(long nside) {
	return ((nside) & (nside - 1)) ? -1 : ilog2(nside);
}

const uint16_t utab[] = {
#define Z(a) 0x##a##0, 0x##a##1, 0x##a##4, 0x##a##5
#define Y(a) Z(a##0), Z(a##1), Z(a##4), Z(a##5)
#define X(a) Y(a##0), Y(a##1), Y(a##4), Y(a##5)
	X(0), X(1), X(4), X(5)
#undef X
#undef Y
#undef Z
};

int compress_bits(int v) {
	int raw = (v & 0x5555) | ((v & 0x55550000) >> 15);
	return ctab[raw & 0xff] | (ctab[raw >> 8] << 4);

}

void nest2xyf(long nside, long pix, int &ix, int &iy, int &face_num) {

	long npface_ = nside * 12 * 12;
	long order_ = nside2order(nside);
	face_num = pix >> (2 * order_);
	pix &= (npface_ - 1);
	ix = compress_bits(pix);
	iy = compress_bits(pix >> 1);
}

int spread_bits(int v) {
	return utab[v & 0xff] | (utab[(v >> 8) & 0xff] << 16);
}

const int nb_xoffset[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
const int nb_yoffset[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
const int nb_facearray[][12] = { { 8, 9, 10, 11, -1, -1, -1, -1, 10, 11, 8, 9 }, // S
	{ 5, 6, 7, 4, 8, 9, 10, 11, 9, 10, 11, 8 },   // SE
	{ -1, -1, -1, -1, 5, 6, 7, 4, -1, -1, -1, -1 },   // E
	{ 4, 5, 6, 7, 11, 8, 9, 10, 11, 8, 9, 10 },   // SW
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 },   // center
	{ 1, 2, 3, 0, 0, 1, 2, 3, 5, 6, 7, 4 },   // NE
	{ -1, -1, -1, -1, 7, 4, 5, 6, -1, -1, -1, -1 },   // W
	{ 3, 0, 1, 2, 3, 0, 1, 2, 4, 5, 6, 7 },   // NW
	{ 2, 3, 0, 1, -1, -1, -1, -1, 0, 1, 2, 3 } }; // N
const int nb_swaparray[][3] = { { 0, 0, 3 },   // S
	{ 0, 0, 6 },   // SE
	{ 0, 0, 0 },   // E
	{ 0, 0, 5 },   // SW
	{ 0, 0, 0 },   // center
	{ 5, 0, 0 },   // NE
	{ 0, 0, 0 },   // W
	{ 6, 0, 0 },   // NW
	{ 3, 0, 0 } }; // N
long xyf2nest(int ix, int iy, int face_num, long order_) {
	return (long(face_num) << (2 * order_)) + spread_bits(ix)
		+ (spread_bits(iy) << 1);
}

void neighbours_nest(long pix, long nside, long *result) {
	int ix, iy, face_num;

	int order_ = nside2order(nside);
	nest2xyf(nside, pix, ix, iy, face_num);
	long nsm1 = nside - 1;

	printf("ix: %i\n", ix);
	printf("iy: %i\n", iy);
	printf("face_num: %i\n", face_num);
	printf("order: %i\n", order_);

	if ((ix > 0) && (ix < nsm1) && (iy > 0) && (iy < nsm1)) {
		printf("hello implemented\n");

		long fpix = long(face_num) << (2 * order_), px0 = spread_bits(ix), py0 =
			spread_bits(iy) << 1, pxp = spread_bits(ix + 1), pyp =
			spread_bits(iy + 1) << 1, pxm = spread_bits(ix - 1), pym =
			spread_bits(iy - 1) << 1;

		result[0] = fpix + pxm + py0;
		result[1] = fpix + pxm + pyp;
		result[2] = fpix + px0 + pyp;
		result[3] = fpix + pxp + pyp;
		result[4] = fpix + pxp + py0;
		result[5] = fpix + pxp + pym;
		result[6] = fpix + px0 + pym;
		result[7] = fpix + pxm + pym;

	} else {
		for (int i = 0; i < 8; ++i) {
			printf("\n\nhello set %i\n", i);
			int x = ix + nb_xoffset[i], y = iy + nb_yoffset[i];
			int nbnum = 4;
			if (x < 0) {
				printf("path a\n");
				x += nside;
				nbnum -= 1;
			} else if (x >= nside) {
				printf("path b\n");
				x -= nside;
				nbnum += 1;
			}
			if (y < 0) {
				printf("path c\n");
				y += nside;
				nbnum -= 3;
			} else if (y >= nside) {
				printf("path d\n");
				y -= nside;
				nbnum += 3;
			}

			int f = nb_facearray[nbnum][face_num];
			if (f >= 0) {
				int bits = nb_swaparray[nbnum][face_num >> 2];
				printf("path gx %i\n", face_num);
				printf("path gx %i\n", face_num>>2);
				if (bits & 1) {
					printf("path e\n");
					x = nside - x - 1;
				}
				if (bits & 2) {
					printf("path f\n");
					y = nside - y - 1;
				}
				if (bits & 4) {
					printf("path g\n");
					swap(x, y);
				}
				result[i] = xyf2nest(x, y, f, order_);
				printf("path h result is %li\n", xyf2nest(x,y,f,order_));
			} else{
				printf("path i\n");
				result[i] = -1;
			}
		}
	}
}

int main() {

	cout << "Hello World" << endl; // prints Hello World
	int i, j;
	long nside = 8192;
	long result[8];

	for (i = 100; i < 101; i++) {

		neighbours_nest(i, nside, result);
		printf("For %i\n", i);
		for (j = 0; j < 8; j++) {
			printf("Have found neighbor %li\n", result[j]);

		}

	}
	return 0;
}
