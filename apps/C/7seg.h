// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#define segA  1
#define segB  2
#define segC  3
#define segD  4
#define segE  5
#define segF  6
#define segG  7

int seg2pat[10][8] = {
	{segA,segB,segC,segD,segE,segF,0},	// 0
	{segB,segC,0},				// 1
	{segA,segB,segD,segE,segG,0},		// 2
	{segA,segB,segC,segD,segG,0},		// 3
	{segB,segC,segF,segG,0},		// 4
	{segA,segC,segD,segF,segG,0},		// 5
	{segA,segC,segD,segE,segF,segG,0},	// 6
	{segA,segB,segC,0},			// 7
	{segA,segB,segC,segD,segE,segF,segG,0},	// 8
	{segA,segB,segC,segD,segF,segG,0}	// 9
};

int pat2reg[9] = {
	0,14,15,17,18,22,23,24,25
};

inline  gen_regpat (int num) {
	int i, v;
	v = 1<<7 | 1<<8;
	for (i=0;i<8;i++) {
		int x;
		x = seg2pat[num][i];
		if(x==0) break;
		v = v + (1<<pat2reg[x]);
	}
	return v;
}


