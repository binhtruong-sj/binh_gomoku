/*
 * caro.h
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <algorithm>
#include <fcntl.h>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#ifndef CARO_H_
#define CARO_H_

//#define HASH 1
//#define GLOBAL_ALPHA_BETA 1

//#define CHECK
//#define DUAL_CHECK
//#define old
#define GOODVERSION
#define board_size 16
#define row_size board_size
#define col_size board_size
#define ROWCOL 1
#define RANDOMIZE 1
#define SEast 7
#define South 6
#define SWest 5
#define West 4
#define NWest 3
#define North 2
#define NEast 1
#define East 0
#define nullptr 0
#define NO_TERMINATE 'N'
#define ALPHABETA 'A'
#define MAXDEP 'M'
#define WIN 'W'
#define TIMEOUT 'T'

#define INF(depth)           \
	{                        \
		0x03FFFFFF, 0, depth \
	}

#define NINF(depth)             \
	{                           \
		-(0x03FFFFFF), 0, depth \
	}

#define E_FAR 0		 // cell val: empty, far from any occupied cell (4 too far)
#define BOUNDARY 0xb // for boundary cells
#define PLAYERx 0x1	 // occupied with X
#define PLAYERo 0x2	 // occupied with O
#define isX_(a) a == PLAYERx
#define isO_(a) a == PLAYERo

#define E_NEAR 0x10	 // Empty cell, but close to other occupied cells
#define E_FNEAR 0x11 // Empty cell, but close to other occupied cells
#define E_TNEAR 0x20 // for temporary Near, clear after move is made
#define E_CAL 0x100
#define E_LAST 0x8000

#define st_RESET 0
#define st_SETUP 1
#define st_HUMAN_MOVE 2
#define st_GEN_WORKS 3
#define st_AI_MOVE 4
#define st_QUIT 5
#define T_ARR_MAX 10
#define printInterval 100000
#define MAXDEPTH 41
#define LOW_THRESH_HOLD 10

#define SEARCH_DISTANCE 7
#define ReverseDirection(a) (a + 4) % 8
//#define oppositeVal(a) a^0x3
#define oppositeVal(a) a ^ 0x3
#define myTurn(a) a % 2
#define FLIP(a) (((a = (a ^ 0x1))) ? "ON" : "OFF")
#define percentAdjust(val, percent) (val * percent) / 100

#define BASE_SCORE 20

#define MAXWIDTH 50
#define CONTINUOUS_BIAS 1

#define SYMBOLMODE 0
#define SYMBOLMODE2 2
#define SYMBOLMODE3 4
#define SCOREMODE 6
#define SCOREMODEm 8
#define POSSMOVEMODE 10
#define SYMBOLMODE4 12
#define RAWMODE 14
#define MINSCORE 2

#define cellVal(rowcol) board[rowcol.row][rowcol.col].val
#define cellScore(rowcol) board[rowcol.row][rowcol.col].score
#define cell_itself(rowcol) board[rowcol.row][rowcol.col]
#define toBinary(v, binary)                    \
	{                                          \
		int vvall = v;                         \
		for (int bit = 7; bit >= 0; bit--)     \
		{                                      \
			binary[bit] = (vvall & 0x1) + '0'; \
			vvall >>= 1;                       \
		}                                      \
		binary[8] = 0;                         \
	}
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define mapping(row) (board_size - row)
#define reverseMapping(row) (board_size - row)
#define convertROWCCOL(row, ccol, col) \
	{                                  \
		row = reverseMapping(row);     \
		col = convertCharToCol(ccol);  \
	}

#define flipCoin() (std::rand() % 2)
#define isMyPlay(play, aiVal) (play & aiVal)
#define convertValToXO(i) ((char)(i % 2 ? 'X' : 'O'))

/*
Scoring policy:
On each turn, each location is evaluated for both X and O (is it accurate? in 
evalutating perspective) (perhaps, it needs to discount the out-of-turn player 
score, since it is next move, as derivative move)
MAX: take the biggest score of either player as the result. Find the best move 
     of either player.
ADD: add both players score and use it as the result score. Find the best offensive
     and defensive combined move.
SUB: AI - opponent's move. This is the correct scoring, for mini-max algorithim, 
     the minimum is the best move for opponent.  On the other hand, maximum is the
	 best move for AI. So, subtractive is akin to additive (best of both world)
*/

#define scoreMATH(sA, sM, a, b) (sM ? MAX(a, b) : (sA ? (a + b) : (a - b)))
#define DELTA(a, b) (a - b)

#ifdef RANDOMIZE
#define asMIN(cmp, a, b) ((flipCoinResult) ? (((cmp = b.greaterThanOrEqualValue(a))) ? a : b) : (((cmp = b.greaterValueThan(a))) ? a : b))
#define asMAX(cmp, a, b) ((flipCoinResult) ? (((cmp = a.greaterThanOrEqualValue(b))) ? a : b) : (((cmp = a.greaterValueThan(b))) ? a : b))
#else
#define asMIN(cmp, a, b) ((cmp = b.greaterValueThan(a))) ? a : b
#define asMAX(cmp, a, b) ((cmp = a.greaterValueThan(b))) ? a : b
#endif
#define charToNum(c) ((c >= 'a') ? (10 + c - 'a') : (c - '0'))
#define displayPlay(setVal) (setVal == PLAYERx ? whoPlay1st : whoPlay2nd)
#define maximizerToPlay(maximizingPlayer) maximizingPlayer ? aiVal : humanVal
#define convertValToCol(col) ((char)(col - 1 + 'a'))
#define printScores(pn, n) ((printScore && (pn > (n - 1))))

#define convertCellToStr(val) (char)((val == PLAYERx) ? whoPlay1st : (val == PLAYERo) ? whoPlay2nd : (val == E_FAR) ? '.' : (val == 0xb) ? 'b' : (val & E_CAL) ? '_' : (val & E_TNEAR) ? '+' : '`')
#define pause()                             \
	{                                       \
		cout << "PAUSED for input" << endl; \
		char ach;                           \
		cin >> ach;                         \
	}

#define VERBOSE0 1
#define SIGNUM 0x000BABE0
#define NUM9 NUM8 * 2
#define NUM8 NUM7 * 2
#define NUM7 NUM6 * 3
#define NUM6 NUM5 * 3
#define NUM5 NUM4 * 3
#define NUM4 NUM3 * 3
#define NUM3 NUM2 * 3
#define NUM2 6
#define NUM1 2

#define PRINTSCORE 1

// (a+b)/2 =50 percent -> a+b = 2*p
#define fscale(a, b, p) a + ((b - a) * p) / 100
#define runtimeInMicroSecond 9000000 * 1 // 9 seconds
/*
#define checkBound(a, bound)        \
	{                               \
		if ((a < 0) || (a > bound)) \
			cout << "\n; Bound Error a = " << a << endl; }
			*/
#define checkBound(a, b) \
	{                    \
	}

extern int all_quit;
extern char whoPlay1st, whoPlay2nd;

/*
 * Line for the purpose of scoring
 */
using std::hex;
using std::showbase;
typedef int points;
extern mutex gprint_mtx;
extern char ostr[];
extern bool scoreAdd, scoreSub, scoreMax;
extern bool printScore, newScoreScheme;
extern pair<int, int> printScoreHighNum;
extern vector<pair<int, int>> stable;

extern int printScoreLowNum;
extern bool traceAll, doSaveScore;
extern int humanVal, aiVal;
extern int MAGICNUMBER;
extern int CCHK_WIN;
extern bool aiIsX, humanIsX;
extern int tableTracker[256];
extern int rc;
extern vector<vector<pair<char, char>>> XOscoreArray;

enum WINNER
{
	W_Human,
	W_Ai,
	W_None
};

enum T_STAGE
{
	TS_off,
	TS_looking_for_best_move,
	TS_search_for_id,
	TS_after_found_id

};
enum TS_ACT
{
	TSA_none,
	TSA_save_bestID,
	TSA_found_id,
	TSA_save_score
};
struct hEntry
{
public:
	int line;
	int connected;
	int bitcnt;
	int score;
	unsigned refcnt = 0;
};
class scoreTable
{
	char OscoreArray[256], XscoreArray[256];
	char subArray[32] = {
		0,	//  00000
		4,	//  00001
		4,	//  00010
		8,	//  00011
		4,	//  00100
		8,	//  00101
		8,	//  00110
		12, //  00111
		4,	//  01000
		6,	//  01001
		8,	//  01010
		12, //  01011
		8,	//  01100
		12, //  01101
		12, //  01110
		16, //  01111
		4,	//  10000
		5,	//  10001
		6,	//  10010
		9,	//  10011
		8,	//  10100
		12, //  10101
		12, //  10110
		16, //  10111
		8,	//  11000
		9,	//  11001
		12, //  11919
		16, //  11011
		12, //  11100
		16, //  11101
		16, //  11110
		20};

public:
	scoreTable()
	{
		for (int i = 0; i < 256; i++)
		{
			OscoreArray[i] = 0xFF;
		}
		OscoreArray[0] = 0;
		for (int i = 1; i < 256; i++)
		{
			int j = i;
			while ((j & 1) == 0)
			{
				j = j >> 1;
			}
			if (j < 0x1F)
				OscoreArray[i] = subArray[j];
			else
			{
				int ii = i & 0x1f;
				OscoreArray[i] = subArray[ii];
				int iii = i >> 5;
				switch (iii)
				{
				case 2:
					if (ii = 0x15)
						OscoreArray[i] = 14; // 0x_x_x_X
					break;
				default:
					break;
				}
			}
		}
		for (int i = 0; i < 256; i++)
		{
			XscoreArray[i] = 0xFF;
		}
		XscoreArray[0] = 0;
		for (int i = 1; i < 256; i++)
		{
			int j = i;
			while ((j & 1) == 0)
			{
				j = j >> 1;
			}
			if (j < 0x1F)
				XscoreArray[i] = subArray[j];
			else
			{
				int ii = i & 0x1f;
				XscoreArray[i] = subArray[ii];
				int iii = i >> 5;
				switch (iii)
				{
				case 7:
				case 3:
				case 1:
					if (ii & 1)
					{
						if (ii <= 7)
						{
							break;
						}
						else
						{
							XscoreArray[i] = 0;
						}
					}
					else
					{
						XscoreArray[i] = XscoreArray[i] + 4;
					}
					break;
				case 2:
					if (ii = 0x15)
						XscoreArray[i] = 8; // 0x_x_x_X
					else
						XscoreArray[i] = XscoreArray[i] >> 1;
					break;
				default:
					break;
				}
			}
		}
	}
	int getScore(int pattern, int val)
	{
		int n;
		char p = (char)pattern;
		if (val & PLAYERx)
		{
			n = XscoreArray[p];
		}
		else
		{
			n = OscoreArray[p];
		}
		return n;
	}
	void printTable(ofstream &f)
	{
		int i = 0;
		f << "XscoreArray[] = {"
		  << "\n";
		for (auto e : XscoreArray)
		{
			char bn[9];
			toBinary(i, bn);
			f << (int)XscoreArray[i] << ", //\t" << bn << "\n";
			i++;
		}
		f << "} ;"
		  << "\n";
		f << "OscoreArray[] = {"
		  << "\n";
		for (auto e : XscoreArray)
		{
			char bn[9];
			toBinary(i, bn);
			f << (int)OscoreArray[i] << ", //\t" << bn << "\n";
			i++;
		}
		f << "} ;" << endl;
	}
};
extern scoreTable sTable;

class hashTable
{
public:
	hEntry arrayE[1000];
	int arrayE_cnt = 0;
	int lowest;
	int lowestI;
	int swapcnt = 0;
	hashTable();
	void swap2e(int from, int to);
	void addEntry(int line, int connected, int cnt, int score);
	void print();
};
#define COSTADJUSTPERCENT 50 // 5 percent cost per level of depth, lose 50% value after 10 level

class aScore
{
public:
	points myScore = 0, oppScore = 0;
	union {
		int j = 0;
		struct
		{
			unsigned char connectedOrCost, combo, ocombo4, mcombo4;
		} i;
	};
	/*
	aScore(int m = 0, int o = 0) : myScore{m}, oppScore{o}, i.connectedOrCost{0}
	{
	}
	aScore(int m = 0, int o = 0, int d = 0) : myScore{m}, oppScore{o}, i.connectedOrCost{d}
	{
	}
	
*/
	bool operator==(const aScore &v)
	{
		bool out;

		out = (v.myScore == myScore) && (v.oppScore == oppScore) && (v.i.connectedOrCost == i.connectedOrCost);
		return out;
	}

	friend ostream &operator<<(ostream &out, const aScore &v)
	{
		out << "(" << v.myScore << "." << v.oppScore << "," << dec
			<< (int)v.i.connectedOrCost << ")";
		return out;
	}

	aScore &operator=(const aScore &v)
	{
		myScore = v.myScore;
		oppScore = v.oppScore;
		/*	i.connectedOrCost = v.i.connectedOrCost;
		i.winner = v.i.winner;
		i.combo = v.i.combo;
		*/
		j = v.j;
		return *this;
	}
	bool operator<=(const aScore &b)
	{
		return ((myScore - oppScore) > (b.myScore - b.oppScore));
	}
	aScore &operator+=(const aScore &v)
	{
		myScore = myScore + v.myScore;
		oppScore = oppScore + v.oppScore;
		i.connectedOrCost = MAX(i.connectedOrCost, v.i.connectedOrCost);
		return *this;
	}
	aScore &init(int v, int d)
	{
		myScore = v;
		oppScore = d;
		i.connectedOrCost = 0;
		return *this;
	}

	int bestMove()
	{
		return myScore + oppScore;
	}
	int bestValue()
	{
		return scoreMATH(scoreAdd, scoreMax, myScore, oppScore);
	}

	bool greaterValueThan(aScore &other)
	{
		bool result;
		//debugOut << "+";

		int deltaCost = DELTA(other.i.connectedOrCost, i.connectedOrCost) >> 1;
		if (deltaCost == 0)
		{
			result = (bestValue() > other.bestValue());
		}
		else if (deltaCost < 0)
		{
			int v = bestValue();
			v = v >> deltaCost;
			result = (v > other.bestValue());
		}
		else
		{
			int v = other.bestValue();
			v = v >> -deltaCost;
			result = (bestValue() > v);
		}
		return result;
	}
	bool greaterThanOrEqualValue(aScore &other)
	{
		bool result;
		int deltaCost = DELTA(other.i.connectedOrCost, i.connectedOrCost) >> 1;
		//	debugOut << ".";
		if (deltaCost == 0)
		{
			result = (bestValue() > other.bestValue());
		}
		else if (deltaCost < 0)
		{
			int v = bestValue();
			v = v >> deltaCost;
			result = (v >= other.bestValue());
		}
		else
		{
			int v = other.bestValue();
			v = v >> -deltaCost;
			result = (bestValue() >= v);
		}
		return result;
	}
	bool equal(aScore &other)
	{
		bool result;
		int deltaCost = DELTA(other.i.connectedOrCost, i.connectedOrCost);
		if (deltaCost == 0)
		{
			result = (bestValue() == other.bestValue());
		}
		else if (deltaCost < 0)
		{
			int v = bestValue();
			v = v >> deltaCost;
			result = (v == other.bestValue());
		}
		else
		{
			int v = other.bestValue();
			v = v >> -deltaCost;
			result = (bestValue() == v);
		}
		return result;
	}
};
class Line
{
public:
	friend class hashTable;
	int score;
	int val = 0;   // 32 bit encoded of a line
				   // not the whole line.  Just started on O_
				   // scan to maximum of 7 squares
	int cnt = 0;   // how many X_
	int connected; // How long is the longest connected X_
	int continuous;
	int blocked; // is it blocked by O_ (next to an X_).
	int offset;	 // to add or subtract to scoring
	int type;	 // X_ or O_ are being search

	friend ostream &operator<<(ostream &out, Line &v)
	{
		char binary[9];
		toBinary(v.val, binary);
		out << "Line=" << binary << " Cnt=" << v.cnt;
		out << " Blocked=" << v.blocked << " Connected=" << v.connected
			<< " cont=" << v.continuous;
		out << " Score = " << v.score;
		return out;
	}

	bool operator==(Line &other)
	{
		bool rtnval;
		rtnval = true;
		if (val != other.val)
		{
			rtnval = false;
		}
		if (score != other.score)
		{
			rtnval = false;
		}

		if (connected != other.connected)
		{
			rtnval = false;
		}
		if (blocked != other.blocked)
		{
			rtnval = false;
		}
		if (offset != other.offset)
		{
			rtnval = false;
		}
		if (continuous != other.continuous)
		{
			rtnval = false;
		}
		if (cnt != other.cnt)
		{
			rtnval = false;
		}
		return rtnval;
	}
	int evaluate(bool ending);
	void print(ofstream &debugOut)
	{
		char binary[9];
		toBinary(val, binary);
		sprintf(ostr, "Line=%s Cnt=%d blocked=%d connected =%d", binary, cnt, blocked,
				connected);
		debugOut << ostr;
		debugOut << score;
		debugOut << endl;
	}
};

/*
 * composite score for all crosslines
 */
class FourLines
{
public:
	Line Xlines[4];
	int score;
	void print(ofstream &out);
};

/*
 * a cell in a caro table, has pointers to adjacent cells
 */
class cellV
{
public:
	int val = 0;
	aScore score;
	friend ostream &operator<<(ostream &out, cellV &v)
	{
		out << convertCellToStr(v.val);
		return out;
	}
};
class cell : public cellV
{
public:
	char rowVal, colVal;
	char name[2];
#ifndef ROWCOL
	cell *near_ptr[8];
#endif
	/*
	void print()
	{
		debugOut << val;
	}
*/
	cell(int r = 8, int c = 8)
	{
		rowVal = r;
		colVal = c;
	}
	friend ostream &operator<<(ostream &out, const cell &c)
	{
		out << "[" << dec
			<< mapping(
				   c.rowVal)
			<< convertValToCol(c.colVal) << "]" << convertCellToStr(c.val);
		return out;
	}
	friend ostream &operator<<(ostream &out, const cell *c)
	{
		if (c == nullptr)
		{
			out << "NULL";
		}
		else
		{
			out << "[" << dec
				<< mapping(
					   c->rowVal)
				<< convertValToCol(c->colVal) << "]" << convertCellToStr(c->val);
		}
		return out;
	}
	void print(ofstream &debugOut, int v, int num)
	{
		if (num < 0)
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
		}
		else
		{
			char ach = (char)convertCellToStr(v);
			sprintf(ostr, "%2d%c ", num, ach);
		}
		debugOut << ostr;
		return;
	}
	void print(ostream &debugOut, int v, int num)
	{
		if (num < 0)
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
		}
		else
		{
			char ach = (char)convertCellToStr(v);
			sprintf(ostr, "%2d%c ", num, ach);
		}
		debugOut << ostr;
		return;
	}

	void print(ofstream &debugOut, int mode)
	{
		int pval = val & 0xFFF;
		if (pval & (PLAYERx | PLAYERo))
		{
			if ((mode % 2) == 0)
			{
				char ach = (char)convertCellToStr(pval);
				if (val & E_LAST)
					sprintf(ostr, " |%c|", ach);
				else
					sprintf(ostr, "%3c ", ach);
			}
			else
			{
				sprintf(ostr, "%3c ", ' ');
			}
			debugOut << ostr;
			return;
		}

		switch (mode)
		{
		case SYMBOLMODE:
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
			debugOut << ostr;
			return;
		}
		case (SYMBOLMODE + 1):
		{
			return;
		}
		case SYMBOLMODE2:
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
			debugOut << ostr;

			return;
		}
		case (SYMBOLMODE2 + 1):
		{
			pval = score.myScore + score.oppScore;
			break;
		}
		case SYMBOLMODE3:
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
			debugOut << ostr;
			return;
		}
		case (SYMBOLMODE3 + 1):
		{
			pval = score.myScore - score.oppScore;
			break;
		}
		case SYMBOLMODE4:
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
			debugOut << ostr;
			return;
		}
		case (SYMBOLMODE4 + 1):
		{
			pval = val;
			break;
		}
		case SCOREMODE: // aiPlay
			pval = score.myScore;
			break;
		case SCOREMODE + 1: // opnVal or defVal
			pval = score.oppScore;
			break;
		case SCOREMODEm: // +
			pval = score.myScore + score.oppScore;
			break;
		case SCOREMODEm + 1: // -
			pval = (score.i.connectedOrCost);
			break;
		case POSSMOVEMODE:
		{
			char ach = (char)convertCellToStr(val);
			sprintf(ostr, "%3c ", ach);
			debugOut << ostr;
			return;
		}
		case (POSSMOVEMODE + 1):
		{
			return;
		}
		case RAWMODE: // aiPlay
			pval = val;
			sprintf(ostr, "%3x ", (0xFFF & (pval)));
			debugOut << ostr;
			return;
		case RAWMODE + 1: // opnVal or defVal
			pval = score.myScore + score.oppScore;
			break;
		default:
			break;
		}
		if ((val & E_CAL) == 0)
		{
			pval = 0;
		}
		if (pval)
			if (pval > 0xFFF)
				debugOut << "FFF ";
			else
			{
				sprintf(ostr, "%3x ", (0xFFF & (pval)));
				debugOut << ostr;
			}
		else
		{
			if ((mode % 2) == 0)
				debugOut << "  . ";
			else
				debugOut << "    ";
			//TODO: y defval printed on X 9j
		}
	}

	void printid(ofstream &debugOut)
	{
		sprintf(ostr, "[%d%c]", mapping(rowVal), convertValToCol(colVal));
		debugOut << ostr;
	}
	void getstr(char *s, int d)
	{
		sprintf(s, "%d-[%d%c]--", d, mapping(rowVal), convertValToCol(colVal));
	}

	void cellLocation(ostream &out)
	{
		out << dec << mapping(rowVal) << convertValToCol(colVal) << endl;
		out << flush;
	}
	void printCellLocation(int PIPEout)
	{
		sprintf(ostr, "%d%c\n", mapping(rowVal), convertValToCol(colVal));
		write(PIPEout, ostr, strlen(ostr) + 1);
	}
	void cellPipe(int pipeout)
	{
		ostr[0] = convertValToCol(rowVal);
		ostr[1] = convertValToCol(colVal);
		ostr[2] = 0;
		write(pipeout, ostr, 3);
	}
	/*
	cell &operator=(cell &c)
	{
		score = c.score;
		val = c.val;
		return *this;
	}
*/
	bool operator==(cell &other)
	{
		return ((other.score == score) && (other.val == val));
	}
};

class scoreElement : public aScore
{
public:
	cell *cellPtr = nullptr;
	char rowVal, colVal;
	/*
	using aScore::aScore;
	
	scoreElement(int a=0, int b=0, int c=0) : aScore(a, b, c),cellPtr(nullptr) {}

	scoreElement():aScore(0,0,0),cellPtr(nullptr) {}
	*/
	scoreElement(int row = 0, int col = 0)
	{
		rowVal = row;
		colVal = col;
	}

	friend ostream &operator<<(ostream &output,
							   vector<scoreElement> &values)
	{
		for (auto &value : values)
		{
			output << value << endl;
		}
		return output;
	}

	friend ostream &operator<<(ostream &out, scoreElement &c)
	{
		if (c.cellPtr)
		{
			out << *c.cellPtr;
			out << (c.myScore) << "," << (c.oppScore) << "," << hex << int((c.i.mcombo4 / 2))
				<< int((c.i.ocombo4 / 2)) << int(c.i.combo & 0x7F) //(c.combo ? "C":"")
				<< "c" << dec << (int)c.i.connectedOrCost << "="
				<< c.myScore + c.oppScore << " ";
		}
		else
		{
			out << "[" << mapping((int)c.rowVal) << convertValToCol(c.colVal) << "]";
		}
		return out;
	}
	void printDelta(ostream &out, scoreElement &best)
	{
		if (cellPtr)
		{
			out << *cellPtr;
			out << (myScore + oppScore) << "(" << ((myScore + oppScore) - (best.myScore + best.oppScore)) << ")  ";
		}
		else
		{
			out << "[NULL] ";
		}
	}

	scoreElement &operator+(scoreElement &other)
	{
		myScore = other.myScore + myScore;
	}
	scoreElement &operator=(const scoreElement &other)
	{
		myScore = other.myScore;
		oppScore = other.oppScore;
		/*
		i.connectedOrCost = other.i.connectedOrCost;
		i.winner = other.i.winner;
		i.combo = other.i.combo;
		*/
		j = other.j;
		rowVal = other.rowVal;
		colVal = other.colVal;
		if (other.cellPtr != nullptr)
			cellPtr = other.cellPtr;
		return *this;
	}

	bool operator<=(const scoreElement &b)
	{
		return ((myScore - oppScore) > (b.myScore - b.oppScore));
	}
	void scale(bool my)
	{
		if (my)
			oppScore = oppScore - (oppScore >> 3);
		else
			myScore = myScore - (myScore >> 3);
	}

	void getScore(const scoreElement &other)
	{
		myScore = other.myScore;
		oppScore = other.oppScore;
		/*
		i.connectedOrCost = other.i.connectedOrCost;
		i.winner = other.i.winner;
		i.combo = other.i.combo;
		*/
		j = other.j;
	}
	void getScore(const scoreElement &other, int scale)
	{
		myScore = other.myScore + ((myScore + oppScore) >> scale);
		oppScore = 0;
		/*
		i.connectedOrCost = other.i.connectedOrCost;
		i.winner = other.i.winner;
		i.combo = other.i.combo;
		*/
		j = other.j;
	}
	void getParameter(const scoreElement &other)
	{
		myScore = other.myScore;
		oppScore = other.oppScore;
		/*
		i.connectedOrCost = other.i.connectedOrCost;
		i.winner = other.i.winner;
		i.combo = other.i.combo;
		*/
		j = other.j;
		rowVal = other.cellPtr->rowVal;
		colVal = other.cellPtr->colVal;
		cellPtr = nullptr;
	}
	scoreElement &operator=(const aScore &other)
	{
		myScore = other.myScore;
		oppScore = other.oppScore;
		/*
		i.connectedOrCost = other.i.connectedOrCost;
		i.winner = other.i.winner;
		i.combo = other.i.combo;
		*/
		j = other.j;
		return *this;
	}

	scoreElement &operator=(cell &other)
	{
		cellPtr = &other;
		return *this;
	}
	scoreElement &score(scoreElement &f)
	{
		myScore = f.myScore;
		oppScore = f.oppScore;
		return *this;
	}
	points bestMove()
	{
		return myScore + oppScore;
	}
	points bestValue()
	{
		return MAX(myScore, oppScore);
	}
	bool greaterMove(scoreElement &other)
	{
		if (i.connectedOrCost == other.i.connectedOrCost)
			return (bestMove() > other.bestMove());
		else
			return (i.connectedOrCost < other.i.connectedOrCost);
	}
	bool greaterMoveRaw(scoreElement &other)
	{
		return (bestMove() > other.bestMove());
	}
	int getScore(int setVal, int myval)
	{ // returning setVal's score, need aiPlay for ref
		return ((setVal == myval) ? myScore : oppScore);
	}
	bool compareCell(scoreElement &o)
	{
		cout << "cell a=" << *this << " cell b" << o << endl;
		if (cellPtr && o.cellPtr)
		{
			return (cellPtr == o.cellPtr);
		}
		else if (o.cellPtr)
			return ((rowVal == o.cellPtr->rowVal) && (colVal == o.cellPtr->colVal));
		else if (cellPtr)
			return ((cellPtr->rowVal == o.rowVal) && (cellPtr->colVal == o.colVal));
		else
			return ((rowVal == o.rowVal) && (colVal == o.colVal));
	}
};

class tScore : public scoreElement
{
public:
	aScore ts_ret;
	tScore();
	tScore(scoreElement);
};
class hist
{
public:
	vector<cell> hArray;
	int gameCh;
	friend ostream &operator<<(ostream &out, const hist &c)
	{
		int i = 0;
		for (auto h : c.hArray)
		{
			cell ac = h;
			out << i++ << ac << "->";
		}
		char ach = c.hArray.back().name[0];
		if (isalpha(ach))
			out << "*" << ach;
		return out;
	}

	int limitSearch(int row, int col, int limit)
	{
		for (int i = 0; i < limit; i++)
		{
			if ((row == hArray[i].rowVal) && (col == hArray[i].colVal))
				return (++i);
		}

		return (-1);
	}

	int search(int row, int col)
	{
		for (int i = 0; i < hArray.size(); i++)
		{
			if ((row == hArray[i].rowVal) && (col == hArray[i].colVal))
				return (i);
		}
		return (-1);
	}

	void extract(vector<scoreElement> &se)
	{
		hArray.resize(0);
		gameCh = aiVal;
		for (auto e : se)
		{
			if (e.cellPtr)
			{
				hArray.push_back(*e.cellPtr);
			}
			else
			{
				cell acell;
				acell.rowVal = e.rowVal;
				acell.colVal = e.colVal;
				hArray.push_back(acell);
			}
		}
	}
};

class acrumb : public cellV
{
public:
	int width_id, depth_id;

	cell *ptr = nullptr;
	friend ostream &operator<<(ofstream &out, const acrumb &c)
	{
		out << "{w=" << c.width_id << " d=" << c.depth_id << " ";
		if (c.ptr)
		{
			c.ptr->printid(out);
			out << convertCellToStr(c.val);
		}
		else
			out << "NULL ";
		out << "}";
		return out;
	}
};
class breadCrumb
{
public:
	acrumb top;
	acrumb *array;
	breadCrumb(int depth)
	{
		top.depth_id = depth;
		array = new acrumb[depth];
	}
	~breadCrumb()
	{
		delete array;
	}
	friend ostream &operator<<(ofstream &out, const breadCrumb &c)
	{
		out << c.top << " | ";
		for (int i = c.top.depth_id - 1; i >= 0; i--)
		{
			out << c.array[i] << " - ";
		}
		out << endl;
		return out;
	}

	void extractTohistArray(hist &histArray)
	{
	}

	breadCrumb &operator=(const breadCrumb &other)
	{
		if (other.top.depth_id == top.depth_id)
		{
			top = other.top;
		}
		else if (other.top.depth_id <= top.depth_id - 1)
		{
			int dd =
				(other.top.depth_id < top.depth_id) ? other.top.depth_id : top.depth_id;
			for (int i = 0; i < dd; i++)
			{
				array[i] = other.array[i];
			}
			array[dd] = other.top;
		}
		else
		{
			cout << "ERRROOOOROOOOR" << endl;
		}
		return *this;
	}
	int bestWidthAtDepth(int depth)
	{
		if (depth == top.depth_id)
			return top.width_id;
		else
			return array[depth].width_id;
	}
	cell *bestCellAtDepth(int depth)
	{
		if (depth == top.depth_id)
			return top.ptr;
		else
			return array[depth].ptr;
	}
};
class cellDebug
{
	cell *debugCell[40];
	int dcDepth[40];
	int dcDline[40];
	int debugCnt = 0;

public:
	void reset()
	{
		debugCnt = 0;
	}
	void add(cell *dcell, int depth, int debugLine)
	{
		if (dcell)
		{
			debugCell[debugCnt] = dcell;
			dcDepth[debugCnt] = depth;
			dcDline[debugCnt] = debugLine;
			debugCnt++;
		}
	}
	bool ifMatch(cell *lookupCell, int depth, int dcl)
	{
		for (int i = 0; i < debugCnt; i++)
		{
			if (depth < 0)
			{
				if ((dcl == dcDline[i]) && (*lookupCell == *debugCell[i]))
					return true;
			}
			else if ((dcl == dcDline[i]) && (depth == dcDepth[i]) && (*lookupCell == *debugCell[i]))
				return true;
		}
		return false;
	}
};
/*
 * caro table can be upto 20x20.  However 15x15 is more fair to O'x
 * size-2 is the size of the game; 0 and size-1 are used for boundary cells
 */
class caroDebug
{
	vector<char> debugTrace, trace;
	int drow, dcol;
	/*
	 *
	 */
public:
	int printTrace(ofstream &debugOut)
	{
		debugOut << "\nTRACE: ";
		for (unsigned int i = 0; i < trace.size(); i++)
		{
			sprintf(ostr, "%02d,", trace[i]);
			debugOut << ostr;
		}
		return (trace.size());
	}
	/*
	 *
	 */
	void enterDebugTrace(char *debugString)
	{
		char *dstr;
		dstr = debugString;
		debugTrace.clear();
		while (char achar = *dstr++)
		{
			if (achar != ',')
			{
				debugTrace.push_back((points)(achar - '0'));
			}
		}
	}
	void enterDebugCell(char *debugString)
	{
		char ccol;
		sscanf(debugString, "[%d%c]", &drow, &ccol);
		if (isupper(ccol))
			dcol = ccol - 'A';
		else
			dcol = ccol - 'a';
	}
	bool traceMatch(int r, int c)
	{
		bool cellmatch = false;
		if ((drow >= 0) && (dcol >= 0))
		{
			if ((r == drow) && (c == dcol))
				cellmatch = true;
		}
		else
		{
			cellmatch = true;
		}

		if (cellmatch)
		{
			if (trace == debugTrace)
			{
				return true;
			}
		}

		return false;
	}
	void tracePush(int i)
	{
		trace.push_back((points)i);
	}

	void tracePop()
	{
		return (trace.pop_back());
	}
};
class debugid
{
public:
	vector<int> id;
	bool find(int i)
	{
		bool debugNext = false;
		for (unsigned int ii = 0; ii < id.size(); ii++)
		{
			if ((debugNext = (id[ii] == i)))
			{
				break;
			}
		}
		return debugNext;
	}
};

// tracer is an element of vector to trace the tree path.  Along the traverse path, 2 vector of "depth" is created to track the best path and the new path.
// if new path is better than bestpath, it will be "copy" over, implying delete the old "elements" and copied over new one.

class tracer
{
public:
	char index = 0xff, terminate = 'N', row = 0, col = 0;
	int points;
	void getstr(char *s)
	{
		sprintf(s, "%d[%d%c]", index, mapping(row), convertValToCol(col));
	}
	friend ostream &operator<<(ostream &out, tracer &v)
	{

		//out << dec <<(int) v.index << "[" << (int)v.row << convertValToCol(v.col) << "]";
		char s[10];
		v.getstr(s);
		out << s << v.points;
		if (v.terminate != NO_TERMINATE)
			out << " <" << v.terminate << "> ";
		return out;
	}
	void assignValues(scoreElement &s, int i)
	{
		index = i;
		row = s.cellPtr->rowVal;
		col = s.cellPtr->colVal;
		points = scoreMATH(scoreAdd, scoreMax, s.myScore, s.oppScore); //
																	   /*
Scoring policy:
On each turn, each location is evaluated for both X and O (is it accurate? in 
evalutating perspective) (perhaps, it needs to discount the out-of-turn player 
score, since it is next move, as derivative move)
MAX: take the biggest score of either player as the result. Find the best move 
     of either player.
ADD: add both players score and use it as the result score. Find the best offensive
     and defensive combined move.
SUB: AI - opponent's move. This is the correct scoring, for mini-max algorithim, 
     the minimum is the best move for opponent.  On the other hand, maximum is the
	 best move for AI. So, subtractive is akin to additive (best of both world) -- 
	 This is wrong ... perhaps, ADD with 
*/
	}

	/*
	void extractTohistArray(int val, cell *cptr, hist &histArray)
	{
		int j = 0;
		tracer *tracerPtr;
		histArray.gameCh = val;
		histArray.hArray[j++] = cptr;
		histArray.hArray[j++] = savePoint.cellPtr;
		tracerPtr = next;
		while (1)
		{
			if (tracerPtr == nullptr)
				break;
			if (tracerPtr->savePoint.cellPtr)
			{
				histArray.hArray[j++] = tracerPtr->savePoint.cellPtr;
			}
			else
				break;
			tracerPtr = tracerPtr->next;
		}
		histArray.size = j;
	}
	*/
};

class tracerArray
{
public:
	vector<tracer> tarray;
	tracerArray(int size)
	{
		tarray.resize(size);
	}

	char search(int row, int col)
	{
		for (unsigned int i = 0; i < tarray.size(); i++)
		{
			if ((row == (int)tarray[i].row) && (col == (int)tarray[i].col))
			{
				char r = convertValToXO(i);
				return r;
			}
			if (tarray[i].terminate != NO_TERMINATE)
				break;
		}
		return ('.');
	}
	void copyFromIndex(unsigned int index, tracerArray &cT)
	{
		do
		{
			tarray[index] = cT.tarray[index];

		} while ((cT.tarray[index++].terminate == NO_TERMINATE) && (index < cT.tarray.size()));
	}
	void copyBeforeIndex(int index, tracerArray &cT)
	{
		for (int i = 0; i < (index + 1); i++)
		{
			tarray[i] = cT.tarray[i];
		}
	}

	void print(ofstream &debugOut, unsigned int index)
	{
		index++;
		for (unsigned int i = 0; i < tarray.size(); i++)
		{
			if (i == index)
				debugOut << ">>>>";
			debugOut << (i % 2 ? "Mn" : "Mx");
			debugOut << tarray[i] << " -> ";
			if (i == index)
				debugOut << "<<<<";
			if (tarray[i].terminate != NO_TERMINATE)
				break;
		}
		debugOut << endl;
	}

	/*
	friend ostream &operator<<(ostream &out, tracerArray &v)
	{
		for(i::iterator)
		out << v.savePoint << "<-";
		return out;
	}
	*/
};
class getInputStr
{
	char inputstr[80];
	char *ptr = inputstr;

public:
	getInputStr()
	{
		clear();
	}
	void clear()
	{
		ptr = inputstr;
		inputstr[0] = 0;
	}

	int mygetstr(char *returnStr)
	{
		do
		{

			int i = 0;
			if (*ptr)
			{
				if (!(isalpha(*ptr)))
				{
					while (isdigit(*ptr) || (*ptr == '-') || (*ptr == ','))
					{
						if (*ptr == ',')
						{
							ptr++;
							break;
						}
						returnStr[i++] = *ptr++;
					}
					returnStr[i] = 0;
				}
				else
				{
					while ((isalpha(*ptr)))
					{
						returnStr[i++] = *ptr++;
					}
					returnStr[i] = 0;
				}
				return (strlen(ptr));
			}
			else
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cin >> inputstr;
				ptr = inputstr;
			}
		} while (1);
	}
};
class rowCol
{
public:
	int row, col;

	rowCol()
	{
		row = col = 0;
	}
	rowCol(int r, int c)
	{
		row = r;
		col = c;
	}

	rowCol &operator=(const rowCol &other)
	{
		row = other.row;
		col = other.col;
		return *this;
	}

	void setv(int r, int c)
	{
		row = r;
		col = c;
		return;
	}

	void moveToDir(int rdir, int cdir)
	{
		row += rdir;
		col += cdir;
		checkBound(row, 16);
		checkBound(col, 16);
	}
	void moveToDir(rowCol &rcDir)
	{
		row += rcDir.row;
		col += rcDir.col;
		checkBound(row, 16);
		checkBound(col, 16);
	}
	void reverse()
	{
		row = -row;
		col = -col;
		checkBound(row, 16);
		checkBound(col, 16);
	}
	void setDirection(int dir)
	{
		switch (dir)
		{
		case East:
			setv(0, +1);
			break;
		case NEast:
			setv(-1, +1);
			break;
		case North:
			setv(-1, 0);
			break;
		case NWest:
			setv(-1, -1);
			break;
		case West:
			setv(0, -1);
			break;
		case SWest:
			setv(1, -1);
			break;
		case South:
			setv(1, 0);
			break;
		case SEast:
			setv(1, +1);
			break;
		}
	}
	/*
	 int cellVal() {
	 return board[row][col].val;
	 }
	 */
};
class moveHist
{
public:
	cell *histOfMoves[256]; // fixed at 256, change to use remeainder if different setting
	int histOfMoves_Val[256];
	int histOfMoves_Index = 0;
	int moveCnt = 0;
	void resetHist()
	{
		moveCnt = 0;
		histOfMoves_Index = 0;
	}
	friend ostream &operator<<(ostream &out, const moveHist &c)
	{
		for (int i = 0; i < c.histOfMoves_Index; i++)
		{
			out << *c.histOfMoves[i] << "->";
		}
		out << endl;
		return out;
	}

	void histPrint(ofstream &debugOut)
	{
		debugOut << "lastMoveIndex=" << histOfMoves_Index << "\n";
		for (int i = 0; i < histOfMoves_Index; i++)
		{
			debugOut << "i=" << i << *histOfMoves[i] << "->";
		}
		debugOut << endl;
	}
	cell *lastMoveMade()
	{
		return (histOfMoves[histOfMoves_Index]);
	}
	bool find(cell *b)
	{
		int cnt = moveCnt;
		int hi = histOfMoves_Index;
		while (cnt--)
		{
			if (b == histOfMoves[hi])
			{
				return (true);
			}
			hi = (hi - 1) & 0xFF; // wrap around -- circular
		}
		return false;
	}
	void addMove(cell *b)
	{
		histOfMoves[histOfMoves_Index] = b;
		histOfMoves_Val[histOfMoves_Index] = b->val;
		histOfMoves_Index = (histOfMoves_Index + 1) & 0xFF; // wrap around -- circular
		moveCnt++;
	}
	void addMove(int mvNum, cell *b)
	{
		int lm = mvNum;
		histOfMoves[lm] = b;
		histOfMoves_Val[lm] = b->val;
		histOfMoves_Index = ++moveCnt; // this is to force the entry .. only work if all the restore are done
	}
	void extractTohistArray(int currVal, hist &histArray)
	{
		histArray.gameCh = currVal;
		histArray.hArray.resize(0);
		for (int i = 0; i < moveCnt; i++)
		{
			cell *hm = histOfMoves[i];
			histArray.hArray.push_back(*hm);
		}
	}
};
class PathElement
{
public:
	scoreElement scoreE;
	bool isLeaf;
};
class Path
{
public:
	PathElement array[30];
	void clear()
	{
		for (int i = 0; i < 30; i = i + 2)
		{
			array[i].scoreE = INF(i);
			array[i].isLeaf = 0;
		}
		for (int i = 1; i < 30; i = i + 2)
		{
			array[i].scoreE = NINF(i);
			array[i].isLeaf = 0;
		}
	}
	void print(ofstream &debugOut, int i)
	{
		debugOut << "bestPath: ";
		do
		{
			debugOut << i << array[i].scoreE << "->";
		} while (array[i--].isLeaf == false);
		debugOut << "END" << endl;
	}
};

class scoreElementArray
{
public:
	vector<scoreElement> array;
	int cnt = 0;
};
class treeTracer
{
public:
	bool enableOn = false;
	vector<scoreElement> current, best;
	vector<vector<cell>> TTlist;
	int trTraceActiveDepth = -1, trTraceActiveIndex = -1, trackSubIndex = -1;
	string condition;

	void init(int listRowCnt = 0, bool en = false)
	{
		enableOn = en;
		TTlist.resize(0);
		TTlist.resize(listRowCnt);
		for (auto &e : TTlist)
			e.resize(0);
	}

	friend ostream &operator<<(ostream &out, const treeTracer &v)
	{
		if (!v.enableOn)
			return out;
		for (auto lst : v.TTlist)
		{
			for (auto e : lst)
			{
				out << e << " ";
			}
			out << endl;
		}
		return out;
	}
	treeTracer &operator=(const treeTracer &o)
	{
		if (o.enableOn)
		{
			TTlist.resize(o.TTlist.size());
			for (int i = 0; i < o.TTlist.size(); i++)
			{
				for (auto e : o.TTlist[i])
				{
					TTlist[i].push_back(e);
				}
			}
			enableOn = o.enableOn;
		}
	}
	bool printList(ofstream &o, vector<scoreElement> best)
	{
		if (!(enableOn && (trTraceActiveDepth > -1)))
		{
			return false;
			;
		}
		o << "\n"
		  << "TraceBest Path =";
		int i = 0;
		for (auto e : best)
		{
			o << i++ << e << " ";
		}
		o << condition << endl;
		return true;
	}
	void printCompact(ofstream &o, vector<scoreElement> best, int index)
	{
		if (!(enableOn && (trTraceActiveDepth > -1)))
		{
			return;
		}
		for (auto e : best)
		{
			o << index++ << *e.cellPtr << "->";
		}
		o << endl;
	}

	void setupMyOwn(vector<scoreElement> &list, scoreElement &e)
	{
		if (enableOn && (trTraceActiveDepth >= 0))
		{
			list.resize(0);
			list.push_back(e);
		}
	}
	void terminalNode(vector<scoreElement> &list, scoreElement &e, string &s)
	{
		if (enableOn && (trTraceActiveDepth >= 0))
		{
			list.push_back(e);
			condition = s;
		}
	}
	void moveTo(vector<scoreElement> &f, vector<scoreElement> &t)
	{
		if (enableOn && (trTraceActiveDepth >= 0))
		{
			t.resize(1);
			for (auto m : f)
			{
				t.push_back(m);
			}
		}
	}
	void addRow()
	{
		int size = TTlist.size();
		TTlist.resize(size + 1);
	}
	void addTTraceItem(int row, int col)
	{
		enableOn = true;
		{
			int listRow = TTlist.size() - 1;
			cell e;
			e.rowVal = row;
			e.colVal = col;
			TTlist[listRow].push_back(e);
		}
	}

	int TraceConfig(int depth, scoreElement &c, vector<scoreElement> &list)
	{
		if (!enableOn)
			return -1;
		int foundTrace = -1;
		if (depth == 0)
		{
			if (traceAll)
			{
				trTraceActiveDepth = 0;
				trTraceActiveIndex = 1;
				foundTrace = 1;
			}
			else
				for (int i = 0; i < TTlist.size(); i++)
				{
					if (TTlist[i].size() > depth)
						if ((c.cellPtr->rowVal == TTlist[i][0].rowVal) &&
							(c.cellPtr->colVal == TTlist[i][0].colVal))
						{
							trTraceActiveIndex = i;
							if (TTlist[i].size() > 1)
							{
								trTraceActiveDepth = -1;
								trackSubIndex = 0;
								foundTrace = 0; // on track -- but not yet
							}
							else
							{
								trTraceActiveDepth = 0;
								foundTrace = 1; // found trace
							}
							break;
						}
				}
		}
		else
		{
			if (trTraceActiveIndex >= 0)
			{
				if (trTraceActiveDepth < 0)
				{
					if (trackSubIndex == (depth - 1))
					{
						if ((TTlist[trTraceActiveIndex][depth].rowVal == c.cellPtr->rowVal) &&
							(TTlist[trTraceActiveIndex][depth].colVal == c.cellPtr->colVal))
						{
							trackSubIndex++;
							if ((TTlist[trTraceActiveIndex].size() - 1) == depth)
							{
								trTraceActiveDepth = depth;
								trTraceActiveIndex = -1;
								trackSubIndex = -2;
								foundTrace = 1; // found
							}
							else
								foundTrace = 0; // on track, but not yet
						}
					}
				}
				else if (depth < trTraceActiveDepth)
				{
					trTraceActiveDepth = -2;
				}
			}
		}
		if ((trTraceActiveDepth >= 0) || ((trTraceActiveIndex >= 0) && (trackSubIndex == depth)))
		{
			list.resize(0);
			list.push_back(c);
		}
		return (foundTrace);
	}
	void resetCurrentTracing()
	{
		trTraceActiveIndex = trTraceActiveDepth = -1;
	}
};
class caro : public moveHist
{
public:
	ofstream caroDebugOut;
	vector<int> printScoreNum;
	int *printScoreNumPtr;
	int tid = 0;
	int docheck = 0;
	int toplevelCnt = -1;
	int debugCnt = 0;
	int tbidCnt;
	int dcnt;

	//	int printScoreNum = printScoreLowNum;
	treeTracer caroTTracer;
	vector<scoreElement> *currtraceBestPathPtr;
	T_STAGE currtraceBestPathState[20];

	caroDebug cdebug;
	//	ostream outfile;
	//	FILE * ofile;
	vector<unsigned char> trace;
	Path bestPath; // track the best Path
	//cell board[21][21];
	array<array<cell, 21>, 21> board;
	unsigned int evalCnt = 0;
	int size = 17;
	int highestDepth = 30;

	int localCnt = 0;
	int desiredRuntime = 0;
	bool debugScoring = 0;
	cell *possMove[50];
	vector<char> getWidth{3, 3, 3, 3, 3, 3, 3};
	caro(int table_size = 15)
	{
		size = table_size + 1;
		// Setup pointer to ajacent cells, only from 1 to size-1
		/*
		 #ifdef old
		 ofile  = fopen("oldCheckfile.txt","w");
		 #else
		 ofile  = fopen("Chkfile.txt","w");
		 #endif
		 */
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				board[row][col].rowVal = row;
				board[row][col].colVal = col;
				board[row][col].name[0] = mapping(row);
				board[row][col].name[1] = convertValToCol(col);

				if ((row == 0) || (col == 0) || (row == size) || (col == size))
				{
					board[row][col].val = BOUNDARY;
#ifndef ROWCOL
					for (int i = 0; i < 8; i++)
						board[row][col].near_ptr[i] = &board[row][col];
#endif
				}
				else
				{
					board[row][col].val = E_FAR; // FAR: empty cell 5 away from any occupied cell
#ifndef ROWCOL
					board[row][col].near_ptr[West] = &board[row][col - 1];
					board[row][col].near_ptr[NWest] = &board[row - 1][col - 1];
					board[row][col].near_ptr[North] = &board[row - 1][col];
					board[row][col].near_ptr[NEast] = &board[row - 1][col + 1];

					board[row][col].near_ptr[East] = &board[row][col + 1];
					board[row][col].near_ptr[SEast] = &board[row + 1][col + 1];
					board[row][col].near_ptr[South] = &board[row + 1][col];
					board[row][col].near_ptr[SWest] = &board[row + 1][col - 1];
#endif
				}
			}
		}
	}
	void derivativeScaling(scoreElement &a, int player)
	{
		if (isMyPlay(player, aiVal))
		{
			a.oppScore = a.oppScore * 8 / 10;
		}
		else
		{
			a.myScore = a.myScore * 8 / 10;
		}
	}
	void debugOutopen(string name)
	{
		caroDebugOut.open(name);
	}
	int printTrace()
	{
		caroDebugOut << "\nTRACE: ";
		for (unsigned i = 0; i < trace.size(); i++)
		{
			sprintf(ostr, "%02de", trace[i]);
			caroDebugOut << ostr;
		}

		return trace.size();
	}

	virtual ~caro();
	friend ostream &operator<<(ostream &out, caro &c)
	{
		out << "\n"
			<< "EvalCnt=" << c.evalCnt << " MoveCnt" << c.moveCnt
			<< " Tid=" << c.tid << "\n";
		out << "    ";
		for (char pchar = 1; pchar <= 16; pchar++)
			out << " " << (int)pchar % 10;
		out << "\n";
		out << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			out << " " << pchar;
		out << "\n";
		for (int row = 0; row <= c.size; row++)
		{
			char r1[8];
			sprintf(r1, "%02d", mapping(row));
			out << r1;
			for (int col = 0; col <= c.size; col++)
			{
				cell *cptr = &c.board[row][col];
				if (c.find(cptr))
					out << " " << (char)tolower(convertCellToStr(c.board[row][col].val));
				else
					out << " " << (char)convertCellToStr(c.board[row][col].val);
			}
			int rrr = board_size - row;
			out << " ROW " << dec << rrr << "\t" << row << "\n";
		}
		out << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			out << " " << pchar;
		out << "\n";
		out << "Game Values: " << convertValToXO(humanVal) << " " << convertValToXO(aiVal) << " " << humanIsX << " " << aiIsX << endl;
		return out;
	}
	caro &operator=(caro &other)
	{
		size = other.size;
		highestDepth = other.highestDepth;
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				board[row][col] = other.board[row][col];
			}
		}
		return *this;
	}

	caro &save(caro &other)
	{
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				other.board[row][col] = board[row][col];
			}
		}
		return *this;
	}
	bool compare(caro &other)
	{
		bool result = true;
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				if ((!((other.board[row][col].score == board[row][col].score))) || (!((other.board[row][col].val == board[row][col].val))))
				{

					caroDebugOut << "FAILED  at row=" << row << " col=" << col << "\n";
					caroDebugOut << board[row][col] << board[row][col].score << " <> "
								 << other.board[row][col]
								 << other.board[row][col].score << "\n";
					result = false;
				}
			}
		}
		if (result == false)
		{
			caroDebugOut << "new";
			print(SCOREMODE, 30);
			caroDebugOut << "backup";
			other.print(SCOREMODE, 31);
		}
		return result;
	}

	void print(int mode, int marker)
	{
		int lastMoveIndex1 = histOfMoves_Index - 1;
		gprint_mtx.lock();
		caroDebugOut << "\n"
					 << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
					 << " Tid=" << tid << " Prev_Move_Points="
					 << " Mark=" << marker << "\n";
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				if (histOfMoves[histOfMoves_Index] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (histOfMoves[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
				board[row][col].print(caroDebugOut, mode);
				if (histOfMoves[histOfMoves_Index] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (histOfMoves[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
			}
			caroDebugOut << " ROW " << dec << mapping(row) << "\n";
			for (int col = 0; col <= size; col++)
			{
				board[row][col].print(caroDebugOut, mode + 1);
			}

			caroDebugOut << "\n";
		}
		caroDebugOut << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
		{
			sprintf(ostr, "%3C ", pchar);
			caroDebugOut << ostr;
		}
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void print(hist &histArray, int marker)
	{
		int v, vb;
		gprint_mtx.lock();
		caroDebugOut << "\n"
					 << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
					 << " Tid=" << tid << " Marker=" << marker << "\n";

		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				int lh = histArray.search(row, col);
				vb = board[row][col].val;
				if (vb & (PLAYERx | PLAYERo))
					v = vb;
				else
					v = (lh % 2) ? oppositeVal(histArray.gameCh) : histArray.gameCh;
				board[row][col].print(caroDebugOut, v, lh);
			}
			caroDebugOut << " ROW " << dec << mapping(row) << "\n";
			caroDebugOut << "\n";
		}
		caroDebugOut << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
		{
			sprintf(ostr, "%3C ", pchar);
			caroDebugOut << ostr;
		}
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void print(ostream &dOut, hist &histArray)
	{
		int v, vb;
		//	gprint_mtx.lock();
		dOut << "\n"
			 << "      \!";
		for (int pchar = 1; pchar <= 16; pchar++)
		{
			sprintf(ostr, "%3d ", pchar);
			dOut << ostr;
		}
		dOut << "\n"
			 << "      \!";
		for (char pchar = 'a'; pchar <= 'p'; pchar++)
		{
			sprintf(ostr, "%3C ", pchar);
			dOut << ostr;
		}
		dOut << "\n";

		for (int row = 0; row <= size; row++)
		{
			char rrr[8];
			sprintf(rrr, "\$%02d", mapping(row));
			dOut << rrr;
			for (int col = 0; col <= size; col++)
			{
				int lh = histArray.search(row, col);
				vb = board[row][col].val;
				if (vb & (PLAYERx | PLAYERo))
					v = vb;
				else
					v = (lh % 2) ? oppositeVal(histArray.gameCh) : histArray.gameCh;
				board[row][col].print(dOut, v, lh);
			}
			dOut << " ROW " << dec << mapping(row) << " " << row << "\n";
			dOut << "\n";
		}
		dOut << "       ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
		{
			sprintf(ostr, "%3C ", pchar);
			dOut << ostr;
		}
		dOut << "\n"
			 << "\n"
			 << "1"
			 << "\n"
			 << "g 0 0 0"
			 << "\n"
			 << "\n";
		dOut << "Human is " << (humanIsX ? "X" : "O") << endl;
		//	gprint_mtx.unlock();
	}

	void print(cell *possMove[])
	{
		int lastMoveIndex1 = histOfMoves_Index - 1;
		gprint_mtx.lock();
		caroDebugOut << "\n"
					 << "EvalCnt=" << evalCnt << " MoveCnt" << moveCnt
					 << " Tid=" << tid << "\n";

		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				if (histOfMoves[histOfMoves_Index] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (histOfMoves[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
				int i;
				for (i = 0; i < 40; i++)
				{
					if (possMove[i] == nullptr)
					{
						i = 41;
						break;
					}
					else if (possMove[i] == &board[row][col])
					{
						break;
					}
				}
				if (i >= 40)
					board[row][col].print(caroDebugOut, SYMBOLMODE);
				else
				{
					sprintf(ostr, "%3d ", i + 1);
					caroDebugOut << ostr;
				}

				if (histOfMoves[histOfMoves_Index] == &board[row][col])
					board[row][col].val ^= E_LAST;
				if (histOfMoves[lastMoveIndex1] == &board[row][col])
					board[row][col].val ^= E_LAST;
			}
			caroDebugOut << " ROW " << dec << mapping(row) << "\n";
			caroDebugOut << "\n";
		}
		caroDebugOut << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
		{
			sprintf(ostr, "%3C ", pchar);
			caroDebugOut << ostr;
		}
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void print()
	{
		gprint_mtx.lock();
		caroDebugOut << "\n";
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				caroDebugOut << " " << (char)convertCellToStr(board[row][col].val);
			}
			int rrr = board_size - row;
			caroDebugOut << " ROW " << dec << rrr << "\n";
		}
		caroDebugOut << "  ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			caroDebugOut << " " << pchar;
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void print(tracerArray &ar)
	{
		gprint_mtx.lock();
		caroDebugOut << "\n";
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				char v = ar.search(row, col);
				if (v != '.')
					caroDebugOut << " " << v;
				else
					caroDebugOut << " " << (char)convertCellToStr(board[row][col].val);
			}
			caroDebugOut << " ROW " << dec << mapping(row) << "\n";
		}
		caroDebugOut << "  ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			caroDebugOut << " " << pchar;
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void lprint(ostream &dOut, hist &ar, int limit)
	{
		dOut << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			dOut << " " << pchar;
		dOut << "\n";
		for (int row = 0; row <= size; row++)
		{
			char r1[8];
			sprintf(r1, "%02d", mapping(row));
			dOut << r1;
			for (int col = 0; col <= size; col++)
			{
				if ((row % 16 == 0) || (col % 16 == 0))
				{
					dOut << " b";
				}
				else
				{
					int s = ar.limitSearch(row, col, limit);
					if (s < 0)
						dOut << " .";
					else
					{
						if (s == limit)
							dOut << " " << ((char)convertCellToStr(board[row][col].val));
						else
							dOut << " " << (char)tolower(convertCellToStr(board[row][col].val));
					}
				}
			}
			int rrr = board_size - row;
			dOut << " ROW " << dec << rrr << "\n";
		}
		dOut << "    ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			dOut << " " << pchar;
		dOut << "\n"
			 << limit << endl;
	}
	void print(vector<scoreElement> &ar)
	{
		gprint_mtx.lock();
		caroDebugOut << "\n";
		for (int row = 0; row <= size; row++)
		{
			for (int col = 0; col <= size; col++)
			{
				char v;
				int fi = -1;
				for (unsigned int i = 0; i < ar.size(); i++)
				{
					if ((row == (int)ar[i].cellPtr->rowVal) && (col == (int)ar[i].cellPtr->colVal))
					{
						v = i % 2 ? displayPlay(humanVal) : displayPlay(aiVal);
						v = tolower(v);
						fi = i;
						break;
					}
				}
				if (fi >= 0)
					caroDebugOut << hex << fi << v;
				else
					caroDebugOut << " " << (char)convertCellToStr(board[row][col].val);
			}
			caroDebugOut << " ROW " << dec << mapping(row) << "\n";
		}
		caroDebugOut << "  ";
		for (char pchar = 'A'; pchar <= 'P'; pchar++)
			caroDebugOut << " " << pchar;
		caroDebugOut << endl;
		gprint_mtx.unlock();
	}

	void undo1move()
	{
		caroDebugOut << "Undo p=" << histOfMoves_Index << "moveCnt=" << moveCnt << endl;

		if ((histOfMoves_Index - 1 > 0) && moveCnt > 0)
		{
			moveCnt--;
			histOfMoves_Index = (histOfMoves_Index - 1) & 0xff;
			restoreCell(E_FAR,
						histOfMoves[histOfMoves_Index + 1]->rowVal,
						histOfMoves[histOfMoves_Index + 1]->colVal);

			int t;
			t = humanVal;
			humanVal = aiVal;
			aiVal = t;
			humanIsX = !humanIsX;
			aiIsX = !aiIsX;
		}
	}
	void redo1move()
	{
		caroDebugOut << "Redo p=" << histOfMoves_Index << "moveCnt=" << moveCnt << endl;

		if (histOfMoves_Index >= 0)
		{
			moveCnt += 2;
			restoreCell(histOfMoves_Val[histOfMoves_Index + 2],
						histOfMoves[histOfMoves_Index + 2]->rowVal,
						histOfMoves[histOfMoves_Index + 2]->colVal);
			restoreCell(histOfMoves_Val[histOfMoves_Index + 1],
						histOfMoves[histOfMoves_Index + 1]->rowVal,
						histOfMoves[histOfMoves_Index + 1]->colVal);
			histOfMoves_Index = (histOfMoves_Index + 2) & 0xff;
		}
	}
	void reCalBoard(int currVal)
	{
		for (int row = 1; row < size; row++)
			for (int col = 1; col < size; col++)
			{
				if ((board[row][col].val & (PLAYERo | PLAYERx)) == 0)
					score1Cell(currVal, row, col, false);
			}
	}

#define INSPECTdISTANCE 3
#define SAVErESTOREdISTANCE INSPECTdISTANCE + 8 //  ---X---
	/*
	 * set a cell to X or O
	 */
	cell *prevSetCell;
	int setCell(int setVal, int row, int col, int near)
	{
		int oval = board[row][col].val;
		prevSetCell = &board[row][col];
		int testV = (oval & (PLAYERx | PLAYERo));
		if (testV == 0)
		{ // this check is bc of lazyness of setting up board
			board[row][col].val = setVal;
			if (near == E_NEAR)
			{ // Only real set can be UNDO
				// code to handle undo
				addMove(&board[row][col]);
				board[row][col].score = {moveCnt, 0};
			}
			else if (near == E_FNEAR)
			{ // Only real set can be UNDO
				//		board[row][col].score = {moveCnt,0};
				board[row][col].score = {0, 0};
			}
			else
			{
				localCnt++;
				//	board[row][col].score = {moveCnt+localCnt,0};
				board[row][col].score = {0, 0};
			}
			if (near != E_FAR)
			{
				setNEAR(row, col, near & 0xF0);
			}
		}

		return testV;
	}

	/*
	 * After temporary setting a Cell to X' or O' for scoring, now return it and its neighbor to prev values
	 */

#ifdef ROWCOL
	int restoreCell(int saveVal, int row, int col)
	{
		rowCol crCurrCell, crDir;
		int rval;
		localCnt--;
		for (int dir = East; dir <= SEast; dir++)
		{
			crDir.setDirection(dir);
			crCurrCell.setv(row, col);
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell) & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);

				cellVal(crCurrCell) = cellVal(crCurrCell) & ~(E_CAL);
				if (cellVal(crCurrCell) & (E_TNEAR))
				{
					cellVal(crCurrCell) = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
				}
				else if (rval == 3) // boundary
					break;
			}
		}
		rval = board[row][col].val;
		board[row][col].val = saveVal & ~(E_CAL); // Return the val to prev
		return rval;
	}

	/*
	 * Temporary near marking set when traverse further ahead
	 */
	void setNEAR(int row, int col, int near)
	{
		rowCol crCurrCell, crDir;
		int rval;
		for (int dir = East; dir <= SEast; dir++)
		{
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell) & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				cellVal(crCurrCell) = cellVal(crCurrCell) & ~(E_CAL);
				if (cellVal(crCurrCell) == 0)
				{
					cellVal(crCurrCell) = near;
				}
				if (rval == 3) // boundary
					break;
			}
		}
	}

#define rangeh(n, h) ((n > h) ? h : n)
#define rangel(l, n) ((l > n) ? l : n)

	int primitive_scoring(int base_score, int &cnt, int ecnt, int &ocnt, int &ccnt, int &pdec)
	{
		if (printScores(*printScoreNumPtr, 4))
			caroDebugOut << "B-cnt=" << cnt << " ecnt=" << ecnt << " ocnt=" << ocnt << " ccnt=" << ccnt << " pdec=" << pdec;
		if (ocnt == 2)
		{
			if ((cnt + ecnt) < 5)
			{
				cnt = ccnt = 0;
			}
			else
				ocnt = 1;
		}
		if (pdec)
		{
			if (cnt && (ccnt < 5))
			{
				ccnt--;
				cnt--;
			}
		}
		/*
		if ((ccnt >= 4) && (ecnt >= 2))
			ccnt = ccnt - ecnt;
		*/
		if (printScores(*printScoreNumPtr, 4))
			caroDebugOut << "A-cnt=" << cnt << " ecnt=" << ecnt << " ocnt=" << ocnt << " ccnt=" << ccnt << " pdec=" << pdec;
		int effective_cnt = rangel(0, (cnt - 1 - ocnt));

		effective_cnt = (ccnt > 5) ? 0 : (ccnt > 2) ? ((MAGICNUMBER >> ((5 - ccnt) * 2))) : (1 << (effective_cnt + 3));
		if (printScores(*printScoreNumPtr, 3))
			caroDebugOut << " s=" << effective_cnt << endl;
		pdec = 0;

		return effective_cnt;
	}

	int primitive_chk(int cnt, int emptyCnt, int ccnt)
	{
		//int chk = 0;
		/*
	if (cnt > 2)
	{
		if (oppCnt > 0)
		{
			chk = cnt - 2;
		}
		else
		{
			chk = cnt - 1;
		}
	}
	*/
		return ccnt;
	}

#define sideCount(r, c, oplay, mplay, rowOffset, colOffset, cnt, tecnt, secnt, ocnt, ccnt, pdec) \
	{                                                                                            \
		secnt = 0;                                                                               \
		bool ccntStop = false;                                                                   \
		for (int i = 1; i < 5; i++)                                                              \
		{                                                                                        \
			int rr = rangel(0, rangeh((r + (i * rowOffset)), row_size));                         \
			int cc = rangel(0, rangeh((c + (i * colOffset)), col_size));                         \
			if ((board[rr][cc].val & oplay))                                                     \
			{                                                                                    \
				if (!ccntStop)                                                                   \
				{                                                                                \
					ccntStop = true;                                                             \
					pdec = 1;                                                                    \
				}                                                                                \
				ocnt++;                                                                          \
				break;                                                                           \
			}                                                                                    \
			else if ((board[rr][cc].val & mplay))                                                \
			{                                                                                    \
				secnt = 0;                                                                       \
				cnt++;                                                                           \
				ccnt = ccntStop ? ccnt : ccnt + 1;                                               \
			}                                                                                    \
			else                                                                                 \
			{                                                                                    \
				if (secnt++)                                                                     \
					ccntStop = true;                                                             \
				else                                                                             \
					pdec = 0;                                                                    \
				if (++tecnt > 3)                                                                 \
				{                                                                                \
					if (ocnt)                                                                    \
						ocnt--;                                                                  \
					if (tecnt > 4)                                                               \
						break;                                                                   \
				}                                                                                \
			}                                                                                    \
		}                                                                                        \
	}
#define nextBoard(row, col, dr, dc) (board[row+dr][col+dc]))
#define isEmpty(val, eval) (((int)val & (int)eval) == 0)

#ifdef OLDEXT
	int extractLine(int row, int col, int deltaRow, int deltaCol, int val, int &cCnt, int &discount)
	{
		int save_val = board[row][col].val;
		int oval = oppositeVal(val);
		int bval = val | oval;
		board[row][col].val = val;
		int cr = row, cc = col;
		int ovCnt = 0;
		int spCnt = 0;
		discount = 0;
		if (printScores(*printScoreNumPtr, 4))
			caroDebugOut << endl
						 << "----------------------------------" << endl
						 << board[row][col] << endl;
		cCnt = 0;
		int oblk = 0;
		for (int i = 0; i < 6; i++)
		{
			cr = cr + deltaRow;
			cc = cc + deltaCol;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << "F" << board[cr][cc] << " ";

			if (cr > 15 || cc > 15)
			{
				break;
			}
			if (board[cr][cc].val & oval)
			{
				oblk++;
				ovCnt++;
				break;
			}
			else if (isEmpty(board[cr][cc].val, bval)
			{
				if (spCnt++)
					break;
			}
		}
		if (printScores(*printScoreNumPtr, 5))
			caroDebugOut << endl
						 << board[cr][cc];

		int extractNum = 0;
		int j = 1;
		int mval = 0;
		int special = 0;
		for (int i = 0; i < 6; i++)
		{
			cr = cr - deltaRow;
			cc = cc - deltaCol;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << "B" << board[cr][cc];
			int v = board[cr][cc].val;
			if (v & oval)
			{
				if (oblk)
				{
					if (i < 5)
					{
						cCnt = 0;
						extractNum = 0;
						break;
					}
				}
				if (mval)
				{

					if (cCnt < 4)
					{
						cCnt--;
						extractNum = extractNum ^ (j >> 1);
					}
					else
						discount++;
				}
				break;
			}
			else if (v & val)
			{
				mval = 1;
				if (ovCnt)
				{
					special = j;
					ovCnt = 0;
				}
				else
				{
					extractNum = (extractNum | j);
				}
				cCnt++;
			}
			else
			{
				mval = 0;
				ovCnt = 0;
			}
			j = j << 1;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << hex << extractNum << "," << j;
		}
		if (cCnt > 3)
		{
			extractNum = extractNum | special;
			discount++;
		}
		if (cCnt >= 5)
			discount = 0;
		board[row][col].val = save_val;
		return (extractNum);
	}

#else
	int extractLine(int row, int col, int deltaRow, int deltaCol, int val, int &cCnt, int &discount)
	{
		int save_val = board[row][col].val;
		int oval = oppositeVal(val);
		int bval = val | oval;
		board[row][col].val = val;
		int cr = row, cc = col;
		int ovCnt = 0;
		int spCnt = 0;
		discount = 0;
		if (printScores(*printScoreNumPtr, 4))
			caroDebugOut << endl
						 << "----------------------------------" << endl
						 << board[row][col] << endl;
		cCnt = 0;
		int oblk = 0;
		for (int i = 0; i < 6; i++)
		{
			cr = cr + deltaRow;
			cc = cc + deltaCol;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << "F" << i << board[cr][cc] << " ";

			if (board[cr][cc].val & oval)
			{
				oblk++;
				ovCnt++;
				break;
			}
			else if (isEmpty(board[cr][cc].val, bval))
			{
				if (spCnt++)
					break;
			}
			else
			{
				spCnt = 0;
			}
		}
		if (printScores(*printScoreNumPtr, 5))
			caroDebugOut << endl
						 << board[cr][cc];

		int extractNum = 0;
		int j;
		int special = 0;
		if (oblk)
		{
			j = 1;
		}
		else
		{
			j = 2;
			cr = cr - deltaRow;
			cc = cc - deltaCol;
		}
		for (int i = 0; i < 6; i++)
		{
			cr = cr - deltaRow;
			cc = cc - deltaCol;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << "B" << i << oblk << board[cr][cc] << flush;
			int v = board[cr][cc].val;
			if (v & oval)
			{
				if (oblk)
				{
					if (i < 5)
					{
						extractNum = 0;
						break;
					}
				}
				else
				{ // hit a block, flip the line.  That way, blocked is on the LSB
					int newNum = 0;
					j = j >> 1;
					while (j)
					{
						newNum = newNum << 1;
						newNum = newNum | (extractNum & 1);
						extractNum = extractNum >> 1;
						j = j >> 1;
					}
					extractNum = newNum;
				}
				break;
			}
			else if (v & val)
			{
				extractNum = (extractNum | j);
			}
			j = j << 1;
			if (printScores(*printScoreNumPtr, 5))
				caroDebugOut << ">" << hex << extractNum << "<" << j;
		}
		board[row][col].val = save_val;
		return (extractNum);
	}
#endif

//#define sTable_getScore(p, val) sTable.getScore(p, val)
#define sTable_getScore(p, val) XOscoreArray[val - 1][p].first
#define sTable_getcCnt(p, val) XOscoreArray[val - 1][p].second
void swapScoreChk(int &c, int &s, int &cc, int p, int val, int cA[])                
	{                                                 
		bool skip = false;                            
		int j = 0x80;                                 
		do                                            
		{                                             
			j >>= 1;                                  
			skip = (p == j) | skip;                   
		} while (j);                                  
		if (skip)                                     
		{                                             
			cc = c = s = 0;                          
		}                                             
		else                                          
		{                         
			                   
			tableTracker[p]++;                        
			c = cc = XOscoreArray[val - 1][p].second;
			int c2 = c / 2;
			s = XOscoreArray[val - 1][p].first;       
			/*c = cc >> 1; */                         
			int scMultiplier = stable[c2].first;       
			int scOffset = stable[c2].second;          
			s = (scMultiplier * c2 + s) + scOffset;
			cA[c2] = cA[c2] + cc;

			/*cc = c;   */                            
		}                                             
	}

	int evalMaxConnected;
	aScore eval1Cell(const int cPlay, cell *cellPtr,
					 bool debugThis)
	{
		int row = cellPtr->rowVal;
		int col = cellPtr->colVal;
		char binaryStr[33];
		bool doit = true;
		if (doit)
			if (cellPtr->val & E_CAL)
				return cellPtr->score;

		int opnVal = oppositeVal(aiVal);
		int tempVal[2] = {aiVal, opnVal};
		points scores[2] = {0, 0};
		aScore rtn;
		rtn.i.connectedOrCost = 0;
		int chk[2];
		int discount;
		int combo = 0;
		string binaryS;
		auto BINARY = [&](int n) {
			binaryS = "";
			int i = 0x100;
			while (i >>= 1)
			{
				binaryS.push_back((i & n ? '1' : '0'));
			}
			return binaryS;
		};
		for (int p = 0; p < 2; p++)
		{
			rc++;
			int cA[6] = {0, 0, 0, 0, 0, 0};

			bool restrictPlay = (tempVal[p] == PLAYERx);
			int currPlay = tempVal[p];
			int oppPlay = oppositeVal(currPlay);
			if (printScores(*printScoreNumPtr, 4))
				caroDebugOut << rc << " Ev" << *cellPtr << displayPlay(currPlay);
			int shortEcnt;
			int pdec = 0;
			int h_chk, h_count, h_score;
			int v_chk, v_count, v_score;
			int u_chk, u_count, u_score;
			int d_chk, d_count, d_score;
			// horizontal
			if (newScoreScheme == false)
			{
				int h_emptyCnt = 0;
				shortEcnt = 0;
				h_count = 1;
				int h_ocnt = 0;
				int h_ccnt = 1;
				//	caroDebugOut << "RoW=" << row << " CoL=" << col << endl;
				sideCount(row, col, oppPlay, currPlay, 0, -1, h_count, h_emptyCnt, shortEcnt, h_ocnt, h_ccnt, pdec); // Horizontal backward
				sideCount(row, col, oppPlay, currPlay, 0, 1, h_count, h_emptyCnt, shortEcnt, h_ocnt, h_ccnt, pdec);	 // Horizontal forward
				h_score = primitive_scoring(BASE_SCORE, h_count, h_emptyCnt, h_ocnt, h_ccnt, pdec);
				h_chk = primitive_chk(h_count, h_emptyCnt, h_ccnt);
			}
			else
			{
				int cCnt;
				// Need to change scoring to favor combination, lone scoring should be discount
				int n = extractLine(row, col, 0, 1, currPlay, cCnt, discount);
				swapScoreChk(h_count, h_score, h_chk, n, currPlay,cA);

				if (printScores(*printScoreNumPtr, 4))
				{
					if (newScoreScheme)
						caroDebugOut
							<< "Line=" << BINARY(n) << dec << " score=" << h_score << " chk=" << h_chk << endl;
				}
			}
			if (newScoreScheme == false)
			{

				int v_emptyCnt = 0;
				shortEcnt = 0;
				v_count = 1;
				int v_ocnt = 0;
				int v_ccnt = 1;

				sideCount(row, col, oppPlay, currPlay, -1, 0, v_count, v_emptyCnt, shortEcnt, v_ocnt, v_ccnt, pdec); // Vertical
				sideCount(row, col, oppPlay, currPlay, 1, 0, v_count, v_emptyCnt, shortEcnt, v_ocnt, v_ccnt, pdec);	 // Vertical

				v_score = primitive_scoring(BASE_SCORE, v_count, v_emptyCnt, v_ocnt, v_ccnt, pdec);
				v_chk = primitive_chk(v_count, v_emptyCnt, v_ccnt);
			}
			else
			{
				int cCnt;
				int n = extractLine(row, col, 1, 0, currPlay, cCnt, discount);
				swapScoreChk(v_count, v_score, v_chk, n, currPlay,cA);
				if (printScores(*printScoreNumPtr, 4))
				{
					toBinary(n, binaryStr);

					if (newScoreScheme)
						caroDebugOut << "Line=" << BINARY(n) << dec << " score=" << v_score << " chk=" << v_chk << endl;
				}
			}
			if (newScoreScheme == false)
			{
				int u_emptyCnt = 0;
				shortEcnt = 0;
				u_count = 1;
				int u_ocnt = 0;
				int u_ccnt = 1;

				sideCount(row, col, oppPlay, currPlay, 1, -1, u_count, u_emptyCnt, shortEcnt, u_ocnt, u_ccnt, pdec); // Up Angle
				sideCount(row, col, oppPlay, currPlay, -1, 1, u_count, u_emptyCnt, shortEcnt, u_ocnt, u_ccnt, pdec); // Up Angle

				u_score = primitive_scoring(BASE_SCORE, u_count, u_emptyCnt, u_ocnt, u_ccnt, pdec);
				u_chk = primitive_chk(u_count, u_emptyCnt, u_ccnt);
			}
			else
			{
				int cCnt;
				int n = extractLine(row, col, -1, 1, currPlay, cCnt, discount);
				swapScoreChk(u_count, u_score, u_chk, n, currPlay,cA);

				if (printScores(*printScoreNumPtr, 4))
				{
					toBinary(n, binaryStr);

					if (newScoreScheme)
						caroDebugOut << "Line=" << BINARY(n) << dec << " score=" << u_score << " chk=" << u_chk << endl;
				}
			}
			if (newScoreScheme == false)
			{
				int d_emptyCnt = 0;
				shortEcnt = 0;
				d_count = 1;
				int d_ocnt = 0;
				int d_ccnt = 1;

				sideCount(row, col, oppPlay, currPlay, -1, -1, d_count, d_emptyCnt, shortEcnt, d_ocnt, d_ccnt, pdec); // Down Angle
				sideCount(row, col, oppPlay, currPlay, 1, 1, d_count, d_emptyCnt, shortEcnt, d_ocnt, d_ccnt, pdec);	  // Down Angle

				d_score = primitive_scoring(BASE_SCORE, d_count, d_emptyCnt, d_ocnt, d_ccnt, pdec);
				d_chk = primitive_chk(d_count, d_emptyCnt, d_ccnt);
			}
			else
			{
				int cCnt;
				int n = extractLine(row, col, 1, 1, currPlay, cCnt, discount);
				swapScoreChk(d_count, d_score, d_chk, n, currPlay,cA);

				if (printScores(*printScoreNumPtr, 4))
				{
					toBinary(n, binaryStr);

					if (newScoreScheme)
						caroDebugOut << "Line=" << BINARY(n) << dec << " score=" << d_score << " chk=" << d_chk << endl;
				}
			}
			chk[p] = max(max(h_chk, v_chk), max(u_chk, d_chk));
			evalMaxConnected = max(evalMaxConnected, chk[p]);
			/*
			if (currPlay == aiVal)
			{
				chk[p] = -chk[p];
			}
			*/
			vector<int> scoreArr = {h_score, v_score, u_score, d_score};
			sort(scoreArr.begin(), scoreArr.end());

			int cb = cA[2] / 4 + cA[3] / 6 + cA[4] / 8;
			combo = combo + cb;

			int combo4 = 0;
			combo4 = cA[3] + cA[4];
			for (int i = 3; i > 0; i--)
			{
				if (--cb > 1)
					scoreArr[i] = scoreArr[i] * cb;
				else
					break;
			}
			scores[p] = scoreArr[0] + scoreArr[1] + scoreArr[2] + scoreArr[3];

			if (restrictPlay)
			{
				if (printScores(*printScoreNumPtr, 4))
				{
					caroDebugOut << "cA3=" << cA[3] << " cA4=" << cA[4] << " cA5=" << cA[5] << endl;
				}

				if (cA[3] / 6 > 1 || cA[4] / 8 > 1 || cA[5] / 10 > 1)
				{
					combo4 = 0;
					combo = 0;
					scores[p] = -scores[p];
					chk[p] = 0;
				}
			}
			if (p)
				rtn.i.ocombo4 = combo4;
			else
				rtn.i.mcombo4 = combo4;

			if (printScores(*printScoreNumPtr, 4))
				caroDebugOut << " combo=" << combo << " Combo4=" << combo4 << endl;
		}
		rtn.myScore = scores[0];
		rtn.oppScore = scores[1];
		rtn.i.connectedOrCost = max((chk[0]), (chk[1]));
		if (chk[0] > chk[1])
			combo = combo | 0x80;
		rtn.i.combo = combo;
		cellPtr->score = rtn;
		//cout << "as=" << board[row][col] << endl;
		// only turn on CAL if NEAR or TNEAR
		if (doit)
		{
			if (cellPtr->val & (E_NEAR | E_TNEAR))
			{						   // redudant!
				cellPtr->val |= E_CAL; // will not be re-evaluate
			}
		}
		return rtn;
	}

	int evalPlay;
	char evalgid[40];
	int evalDepth;
	static bool lbetterMove(scoreElement &a, scoreElement &b)
	{
		/*
		bool cRe = a.i.connectedOrCost == b.i.connectedOrCost;
		bool cR = a.i.connectedOrCost > b.i.connectedOrCost;
		*/
		bool cRe = true;
		bool cR = false;
		bool cS = ((a.myScore + a.oppScore) > (b.myScore + b.oppScore));
		return ((cRe && cS) || (!cRe && cR));
	}
#ifdef GOODVERSION

	int createTodoList(int todoSize, vector<scoreElement> &todoArray,
					   vector<scoreElement> &bestScoreArray, const int highestConnected, int avg)
	{
		if (todoSize > bestScoreArray.size())
			avg = 0;
		int mark = avg - (avg >> 2);
		vector<scoreElement> remain;
		int sum = 0;
		for (auto e : bestScoreArray)
		{
			if (e.i.connectedOrCost > 7)
				todoArray.push_back(e);
			else
			{
				if ((e.myScore + e.oppScore) > mark)
					remain.push_back(e);
			}
		}

		sort(todoArray.begin(), todoArray.end(), lbetterMove);
		sort(remain.begin(), remain.end(), lbetterMove);
		todoSize = todoSize - todoArray.size();
		int i = 0;
		while (todoSize-- > 0)
		{
			if (i < remain.size())
			{
				todoArray.push_back(remain[i++]);
			}
		}
		return todoArray.size();
	}
	bool reSort(scoreElement &one, scoreElement &two, scoreElement &n)
	{
		auto btterValue = [](scoreElement &a, scoreElement &b) {
			return (MAX(a.myScore, a.oppScore) > MAX(b.myScore, b.oppScore));
		};
		if (btterValue(n, one))
		{
			two = one;
			one = n;
			return true;
		}
		else if (btterValue(n, two))
		{
			two = n;
		}
		return false;
	}

#define MAX3(a, b, c, picka) ((picka = ((a > b) && (a > c))) ? a : ((b > c) ? b : c))
	void finalizedScore(scoreElement &f, scoreElement &s, bool maximizingPlayer)
	{
		bool pickF;
		/*
	after picking the best move, and play it. Could result in a WIN or blocked Win, 
	Calculates the the resulting game score. 
	*/
		if (maximizingPlayer)
			f.myScore = MAX3(f.myScore, s.oppScore, s.myScore, pickF);
		else
			f.myScore = -MAX3(f.oppScore, s.oppScore, s.myScore, pickF);

		if (pickF == false)
		{
			f.i.ocombo4 = s.i.ocombo4;
			f.i.mcombo4 = s.i.mcombo4;
			f.i.combo = s.i.combo;
		}
		// pick the best score and put it in myscore for the final score.
		f.oppScore = 0;
	}
	scoreElement currentBoardScoring(vector<scoreElement> &todoList, int getWidth, const bool maximizingPlayer)
	{
		vector<scoreElement> boardScores;
		scoreElement scoreOfaMove;
		scoreElement firstMove, secondMove;

		char highestConnected = 0;
		int evalPlay = maximizerToPlay(maximizingPlayer);
		auto returnFinal = [&](char c) {
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << "1st: " << firstMove << " 2nd:" << secondMove;
			finalizedScore(firstMove, secondMove, maximizingPlayer);
			firstMove.rowVal = c;
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << " Finalized: " << firstMove << "-" << c << endl;
			return (firstMove);
		};
		auto returnWin = [&](char c) {
			firstMove.rowVal = c;
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << " Finalized: Win " << firstMove << "-" << c << endl;
			if (maximizingPlayer == false)
				firstMove.myScore = -firstMove.oppScore;

			firstMove.oppScore = 0;

			return (firstMove);
		};

		evalMaxConnected = 0;
		int sum = 0;
		int maxCombo4[] = {0, 0};
		for (int row = 1; row < size; row++)
		{
			for (int col = 1; col < size; col++)
			{

				if (board[row][col].val & (E_NEAR | E_TNEAR))
				{
					scoreOfaMove.cellPtr = &board[row][col];
					if (printScores(*printScoreNumPtr, 4))
						caroDebugOut << "before-eval1cell=" << scoreOfaMove << endl
									 << flush;

					scoreOfaMove = eval1Cell(evalPlay, scoreOfaMove.cellPtr, false);
					if (((evalPlay == PLAYERx) && (((scoreOfaMove.myScore < 0) && aiIsX) ||
												   ((scoreOfaMove.oppScore < 0) && humanIsX))))
						continue;
					maxCombo4[0] = max(maxCombo4[0], int(scoreOfaMove.i.mcombo4));
					maxCombo4[1] = max(maxCombo4[1], int(scoreOfaMove.i.ocombo4));

					if (scoreOfaMove.i.connectedOrCost > 0)
					{
						scoreOfaMove.scale(evalPlay == aiVal);

						if (printScores(*printScoreNumPtr, 4))
							caroDebugOut << "eval1cell=" << scoreOfaMove << endl;
						bool newBetterValue = reSort(firstMove, secondMove, scoreOfaMove);
						bool newHighConnected = false;

						// at EVAL --  only terminate if own play is winning
						if (scoreOfaMove.i.connectedOrCost >= CCHK_WIN)
						{
							if (maximizingPlayer)
							{
								if (scoreOfaMove.i.combo & 0x80)
								{
									if (printScores(*printScoreNumPtr, 1))
										caroDebugOut << "AI-win=" << scoreOfaMove;
									return (returnWin('A'));
								}
							}
							else
							{
								if ((scoreOfaMove.i.combo & 0x80) == 0)
								{
									if (printScores(*printScoreNumPtr, 1))
										caroDebugOut << "Hu-win=" << scoreOfaMove;
									return (returnWin('H'));
								}
							}
							//	boardScores.resize(0);
							if (getWidth)
							{

								todoList.push_back(scoreOfaMove);
								return (firstMove);
							}
							else
							{
								return (returnFinal('D'));
							}
						}
						if (getWidth)
						{
							if (scoreOfaMove.i.connectedOrCost <= 6)
								sum = sum + (scoreOfaMove.oppScore + scoreOfaMove.myScore);
							boardScores.push_back(scoreOfaMove);
						}
					}
				}
			}
		}
		if (printScores(*printScoreNumPtr, 4))
			caroDebugOut << "myCombo4=" << maxCombo4[0] << " OppCombo4=" << maxCombo4[1] << endl;

		if (getWidth)
		{
			bool m[2][2] = {{maxCombo4[0] > 7, maxCombo4[1] > 7},
							{ maxCombo4[0] > 9,
							  maxCombo4[1] > 9 }};

			if (m[1][0] || m[1][1])
			{
				if (m[1][0] == false)
				{
					if (maximizingPlayer == false)
					{
						return (returnWin('h'));
					}
					// blocked
				}
				else if (m[1][1] == false)
				{
					if (maximizingPlayer)
					{
						return (returnWin('a'));
					}
					// blocked
				}
				// block -- or one of them win
				// technically, one of them win. But, to play safe, let evaluate further
				// Remove all -- but leaving combo4 moves
				for (auto e : boardScores)
				{
					int cm, co;
					cm = e.i.mcombo4;
					co = e.i.ocombo4;
					if ((cm > 8) || (co > 8))
						todoList.push_back(e);
				}
				sort(todoList.begin(), todoList.end(), lbetterMove);
				return (firstMove);
			}
			else if (m[0][0] || m[0][1])
			{
				for (auto e : boardScores)
				{
					int cm, co;
					cm = e.i.mcombo4;
					co = e.i.ocombo4;
					if ((cm >= 7) || (co >= 7))
						todoList.push_back(e);
				}
				sort(todoList.begin(), todoList.end(), lbetterMove);
				return (firstMove);
			}
			if (boardScores.size() > 0)
			{
				int tavg = sum / boardScores.size();
				createTodoList(getWidth, todoList, boardScores, evalMaxConnected, tavg);
			}
			return (firstMove);
		}
		else
		{
			return (returnFinal('D'));
		}
	}
#endif

#ifdef NO

	int createTodoList(int todoSize, vector<scoreElement> &todoArray,
					   vector<scoreElement> &bestScoreArray, const int highestConnected, int avg)
	{
		if (todoSize > bestScoreArray.size())
			avg = 0;
		int mark = avg - (avg >> 2);
		vector<scoreElement> remain;
		int sum = 0;
		for (auto e : bestScoreArray)
		{
			if (e.i.connectedOrCost == 4)
				todoArray.push_back(e);
			else
			{
				if ((e.myScore + e.oppScore) > mark)
					remain.push_back(e);
			}
		}
		sort(remain.begin(), remain.end(), lbetterMove);
		todoSize = todoSize - todoArray.size();
		int i = 0;
		while (todoSize-- > 0)
		{
			if (i < remain.size())
			{
				todoArray.push_back(remain[i++]);
			}
		}

		return todoArray.size();
	}
	bool reSort(scoreElement &one, scoreElement &two, scoreElement &n)
	{
		auto btterValue = [](scoreElement &a, scoreElement &b) {
			return (MAX(a.myScore, a.oppScore) > MAX(b.myScore, b.oppScore));
		};
		if (btterValue(n, one))
		{
			two = one;
			one = n;
			return true;
		}
		else if (btterValue(n, two))
		{
			two = n;
		}
		return false;
	}

#define MAX3(a, b, c, picka) ((picka = ((a > b) && (a > c))) ? a : ((b > c) ? b : c))
	void finalizedScore(scoreElement &f, scoreElement &s, bool maximizingPlayer)
	{
		bool pickF;
		/*
	after picking the best move, and play it. Could result in a WIN or blocked Win, 
	Calculates the the resulting game score. 
	*/
		if (maximizingPlayer)
			f.myScore = MAX3(f.myScore, s.oppScore, s.myScore, pickF);
		else
			f.myScore = -MAX3(f.oppScore, s.oppScore, s.myScore, pickF);

		if (pickF == false)
		{
			f.i.winner = s.i.winner;
			f.i.combo = s.i.combo;
		}
		// pick the best score and put it in myscore for the final score.
		f.oppScore = 0;
	}

	scoreElement currentBoardScoring(vector<scoreElement> &todoList, int getWidth, const bool maximizingPlayer)
	{
		vector<scoreElement> boardScores;
		scoreElement scoreOfaMove;
		scoreElement firstMove, secondMove;

		char highestConnected = 0;
		int evalPlay = maximizerToPlay(maximizingPlayer);
		// after playing the best(only) move, calculate the score of the result
		auto returnFinal = [&](char c) {
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << "1st: " << firstMove << " 2nd:" << secondMove;
			finalizedScore(firstMove, secondMove, maximizingPlayer);
			firstMove.rowVal = c;
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << " Finalized: " << firstMove << "-" << c << endl;
			return (firstMove);
		};
		auto returnWin = [&](char c) {
			firstMove.rowVal = c;
			if (printScores(*printScoreNumPtr, 2))
				caroDebugOut << " Finalized: Win " << firstMove << "-" << c << endl;
			if (maximizingPlayer == false)
				firstMove.myScore = -firstMove.oppScore;

			firstMove.oppScore = 0;

			return (firstMove);
		};
		evalMaxConnected = 0;
		int sum = 0;
		int maxCombo4[] = {0, 0};
		//	todoList.resize(0);
		for (int row = 1; row < size; row++)
		{
			for (int col = 1; col < size; col++)
			{
				if (board[row][col].val & (E_NEAR | E_TNEAR))
				{
					scoreOfaMove.cellPtr = &board[row][col];
					scoreOfaMove = eval1Cell(evalPlay, scoreOfaMove.cellPtr, false);
					maxCombo4[0] = max(maxCombo4[0], scoreOfaMove.i.combo4 >> 4);
					maxCombo4[1] = max(maxCombo4[1], scoreOfaMove.i.combo4 & 0xF);
					if (printScores(*printScoreNumPtr, 4))
						caroDebugOut << "before-eval1cell=" << scoreOfaMove << endl;

					if (scoreOfaMove.i.connectedOrCost > 0)
					{
						scoreOfaMove.scale(evalPlay == aiVal);

						if (printScores(*printScoreNumPtr, 4))
							caroDebugOut << "eval1cell=" << scoreOfaMove << endl;
						bool newBetterValue = reSort(firstMove, secondMove, scoreOfaMove);
						bool newHighConnected = false;

						// at EVAL --  only terminate if own play is winning
						if (scoreOfaMove.i.connectedOrCost >= 5)
						{
							if (maximizingPlayer)
							{
								if (scoreOfaMove.i.winner == aiVal)
								{
									if (printScores(*printScoreNumPtr, 1))
										caroDebugOut << "AI-win=" << scoreOfaMove;

									return (returnWin('A'));
								}
							}
							else
							{
								if (scoreOfaMove.i.winner == humanVal)
								{
									if (printScores(*printScoreNumPtr, 1))
										caroDebugOut << "Hu-win=" << scoreOfaMove;
									return (returnWin('H'));
								}
							}
							//not a winning move, but a blocked-win move
							if (getWidth)
							{
								todoList.push_back(scoreOfaMove);
								return (firstMove); // if not 'terminal', what returns is not useful
							}
						}
						if (getWidth)
						{
							if (scoreOfaMove.i.connectedOrCost <= 3)
								sum = sum + (scoreOfaMove.oppScore + scoreOfaMove.myScore);
							boardScores.push_back(scoreOfaMove);
						}
					}
				}
			}
		}

		if (getWidth)
		{ /*
			if (maxCombo4[0] > 4 || maxCombo4[1] > 4)
			{
				if (maxCombo4[0] <= 4)
				{
					if (maximizingPlayer == false)
					{
						return (returnWin('c'));
					}
				}
				else if (maxCombo4[1] <= 4)
				{
					if (maximizingPlayer )
					{
						return (returnWin('C'));
					}
				}
				else
				{
					// technically, one of them win. But, to play safe, let evaluate further
					// Remove all -- but leaving combo4 moves
					for(auto e:boardScores) {
						if(e.i.combo4)
							todoList.push_back(e);
					}
					return (firstMove);
				}
			}*/
			if (printScores(*printScoreNumPtr, 4))
				caroDebugOut << "myCombo4=" << maxCombo4[0] << " OppCombo4=" << maxCombo4[1] << endl;

			createTodoList(getWidth, todoList, boardScores, evalMaxConnected, (sum / boardScores.size()));
			return (firstMove);
		}
		else
		{
			return (returnFinal('D'));
		}
	}
#endif
	/*
	 *
	 */
	void saveScoreVal(int row, int col,
					  aScore saveScoreArray[8][SAVErESTOREdISTANCE], aScore &s1Score,
					  int saveValArray[8][SAVErESTOREdISTANCE], int &s1Val)
	{
		rowCol crCurrCell;
		rowCol crDir;
		int rval;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++)
		{
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			int j = 0;
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell) & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				saveScoreArray[dir][j] = cellScore(crCurrCell);
				saveValArray[dir][j++] = cellVal(crCurrCell);
				if (rval == 3) // boundary
					break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
						 aScore saveScoreArray[8][SAVErESTOREdISTANCE], aScore &s1Score,
						 int saveValArray[8][SAVErESTOREdISTANCE], int &s1Val)
	{
		//cell *currCell;
		rowCol crCurrCell;
		rowCol crDir;

		int rval;
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
		for (int dir = East; dir <= SEast; dir++)
		{
			crCurrCell.setv(row, col);
			crDir.setDirection(dir);
			int j = 0;
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					crCurrCell.moveToDir(crDir);
					rval = cellVal(crCurrCell) & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				cellScore(crCurrCell) = saveScoreArray[dir][j];
				cellVal(crCurrCell) = saveValArray[dir][j++];
				if (rval == 3) // boundary
					break;
			}
		}
	}
#else
	int restoreCell(int saveVal, int row, int col)
	{
		cell *currCell;
		int rval;
		localCnt--;
		for (int dir = East; dir <= SEast; dir++)
		{
			currCell = &board[row][col];
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				currCell->val = currCell->val & ~(E_CAL);
				if (currCell->val & (E_TNEAR))
				{
					currCell->val = E_FAR; // only clear the cell with E_TNEAR (temporary NEAR) to FAR
				}
				else if (rval == 3) // boundary
					break;
			}
		}
		rval = board[row][col].val;
		board[row][col].val = saveVal & ~(E_CAL); // Return the val to prev
		return rval;
	}

	void setNEAR(int row, int col, int near)
	{
		cell *currCell;
		int rval;
		for (int dir = East; dir <= SEast; dir++)
		{
			currCell = &board[row][col];
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				currCell->val = currCell->val & ~(E_CAL);
				if (currCell->val == 0)
					currCell->val = near;
				if (rval == 3) // boundary
					break;
			}
		}
	}
	void saveScoreVal(int row, int col,
					  aScore saveScoreArray[8][SAVErESTOREdISTANCE], aScore &s1Score,
					  int saveValArray[8][SAVErESTOREdISTANCE], int &s1Val)
	{
		cell *currCell;
		int rval;
		s1Score = board[row][col].score;
		s1Val = board[row][col].val;
		for (int dir = East; dir <= SEast; dir++)
		{
			currCell = &board[row][col];
			int j = 0;
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				saveScoreArray[dir][j] = currCell->score;
				saveValArray[dir][j++] = currCell->val;
				if (rval == 3) // boundary
					break;
			}
		}
	}
	void restoreScoreVal(int row, int col,
						 aScore saveScoreArray[8][SAVErESTOREdISTANCE], aScore &s1Score,
						 int saveValArray[8][SAVErESTOREdISTANCE], int &s1Val)
	{
		cell *currCell;
		int rval;
		board[row][col].score = s1Score;
		board[row][col].val = s1Val;
		for (int dir = East; dir <= SEast; dir++)
		{
			currCell = &board[row][col];
			int j = 0;
			for (int i = 0; i < INSPECTdISTANCE; i++)
			{
				do
				{
					currCell = currCell->near_ptr[dir];
					rval = currCell->val & (PLAYERx | PLAYERo);
					if (rval)
						i = 0;
				} while ((rval != 3) && rval);
				currCell->score = saveScoreArray[dir][j];
				currCell->val = saveValArray[dir][j++];
				if (rval == 3) // boundary
					break;
			}
		}
	}
#endif

	class traceCell
	{
	public:
		cell *cellPtr;
		traceCell *prev;
		void extractTohistArray(hist &histArray)
		{
		}
	};

	void printDebugInfo(int row, int col, traceCell *trace, int depth)
	{
		caroDebugOut << board[row][col];
		//		printf("Play:%C",displayPlay(currPlay));
		printTrace();
		int i = depth;
		do
		{
			if (trace->cellPtr)
				caroDebugOut << i++ << *(trace->cellPtr) << "->";
			trace = trace->prev;
		} while (trace);
		caroDebugOut << endl;
	}
	void modifyDebugFeatures(int a);
	cell *inputCell()
	{
		int row, col;
		char ccol, inputstr[20];
		bool invalid = true;
		do
		{
			caroDebugOut << "Enter cell:";
			cin >> inputstr;
			sscanf(inputstr, "%d%c", &row, &ccol);
			col = ccol - 'a' + 1;
			if (((row > 0) && (row < board_size)) && ((col > 0) && (col < board_size)))
			{
				return (&board[row][col]);
			}
		} while (invalid);
	}
	void clearScore();
	Line extractLine(int dir, int x, int y, bool &ending);

	aScore score1Cell(const int setVal, const int row, const int col,
					  bool debugThis);

	scoreElement evalAllCell(int incrementalDepth, int val, int width, int depth, int min_depth,
							 bool maximizingPlayer, scoreElement alpha, scoreElement beta, bool debugThis,
							 bool &redo, traceCell *const trace, tracerArray &traceBestArray);

	scoreElement terminateScore;
	cellDebug cdbg;
	int nextWidth(int depth, int width)
	{
		int nw = width - (width / (depth + 1));
		if (nw <= 0)
			nw = 1;
		return (nw);
	}

	void reset();

private:
	int createNodeList(int thisWidth, int in_width,
					   std::vector<scoreElement> &bestScoreArray, const int highestConnected,
					   scoreElement &bestScore);

	int debugSetup(int thisWidth, int in_depth, int currPlay, bool topLevel,
				   bool debugThis, int terminated,
				   std::vector<scoreElement> &bestScoreArray, debugid &debug,
				   std::vector<scoreElement> &returnScoreArray);
};

class briefHist
{
public:
	vector<unsigned char> row;
	vector<unsigned char> col;
	vector<unsigned char> val;
	vector<unsigned char> order;
	int size = 2;

	scoreElement result;

	briefHist(int size = 80)
	{
		row.resize(size + 1);
		col.resize(size);
		val.resize(size);
		order.resize(size);
		row[0] = 0xff;
	}

	briefHist &operator=(const hist &c)
	{
		int i;
		for (i = 0; i < c.hArray.size(); i++)
		{
			row[i] = (char)(c.hArray[i].rowVal);
			col[i] = (char)(c.hArray[i].colVal);
			val[i] = (char)c.hArray[i].val;
			order[i] = (char)i;
		}
		row[i] = 0xFF;
		return *this;
	}

	friend ostream &operator<<(ostream &out, const briefHist &c)
	{
		int i = 0;
		out << "[BH]==>";
		while (c.row[i] != 0xFF)
		{
			out << (int)c.order[i] << "[" << (int)mapping(c.row[i])
				<< (char)convertValToCol(c.col[i]) << "]"
				<< displayPlay(c.val[i]) << "->";
			i++;
		}
		return out;
	}

	void addMove(int o, int r, int c, int v)
	{
		int i = 0;
		while (row[i++] != 0xFF)
			;
		i--;
		row[i] = r;
		col[i] = c;
		val[i] = v;
		order[i++] = o;
		row[i] = 0xFF;
	}
	void addMove(cell *aptr, int v)
	{
		int i = 0;
		while (row[i++] != 0xFF)
			;
		i--;
		row[i] = aptr->rowVal;
		col[i] = aptr->colVal;
		val[i] = v;
		order[i] = i;
		i++;
		row[i] = 0xFF;
	}
	// setCells is for restoring board with previously play game
	// with order maintained, FNEAR
	int setCells(caro *cr)
	{
		int i = 0;
		int mi = 0;
		while (row[i] != 0xFF)
		{
			int r = row[i];
			int c = col[i];
			int o = order[i];
			cr->addMove(o, &cr->board[r][c]);
			cr->setCell(val[i], r, c, E_FNEAR); // FNEAR
			if (o == 0)
				mi = i;
			i++;
		}
		return (row[mi]);
	}
	int setCells(caro *cr, int cnt)
	{
		int i = 0;
		int mi = 0;
		while (i <= cnt)
		{
			int r = row[i];
			if (r == 0xFF)
				break;
			int c = col[i];
			int o = order[i];
			cr->addMove(o, &cr->board[r][c]);
			cr->setCell(val[i], r, c, E_FNEAR); // FNEAR
			if (o == 0)
				mi = i;
			i++;
		}
		return (row[mi]);
	}
	void convertTo(vector<scoreElement> &se)
	{
		int i = 0;
		int mi = 0;
		while (row[i] != 0xFF)
		{
			int r = row[i];
			int c = col[i];
			int o = order[i];
			scoreElement e(r, c);
			se.push_back(e);
			if (o == 0)
				mi = i;
			i++;
		}
	}
	void convertTo(hist &hA)
	{
		int i = 0;
		int mi = 0;
		while (row[i] != 0xFF)
		{
			int r = row[i];
			int c = col[i];
			int o = order[i];
			cell e(r, c);
			hA.hArray.push_back(e);
			i++;
		}
	}

	// this is mainly for setting cells with no order, and for temporary
	// play
	int setCellswithNoOrder(caro *cr, int *sA)
	{
		int i = 0;
		while (row[i] != 0xFF)
		{
			int r = row[i];
			int c = col[i];
			sA[i] = cr->setCell(val[i], r, c, E_TNEAR);
			i++;
		}
		return (row[0]);
	}
	void extract(caro &aCaro)
	{
		int startingPoint = aCaro.histOfMoves_Index - aCaro.moveCnt;
		for (int i = 0; i < aCaro.moveCnt; i++)
		{
			int hmIndex = startingPoint + i;
			cell *ptr = aCaro.histOfMoves[hmIndex];
			aCaro.caroDebugOut << *ptr << "+";
			row[i] = ptr->rowVal;
			col[i] = ptr->colVal;
			order[i] = i;
			val[i] = ptr->val;
		}
		row[aCaro.moveCnt] = 0xFF;
	}
	void restoreCells(caro *cr, int *sA)
	{
		int i = 0;
		while (row[i] != 0xFF)
		{
			int r = row[i];
			int c = col[i];
			cr->restoreCell(sA[i], r, c);
			i++;
		}
	}
	void postResult(scoreElement &c)
	{
		result = c;
	}
};
class oneWork
{
public:
	int val, index;

	scoreElement work;
	pair<scoreElement, hist> result;
	friend ostream &operator<<(ostream &out, oneWork &c)
	{
		out << " work:" << c.work;
		if (c.result.first.cellPtr)
		{
			out << " Result:" << c.result.first;
		}
		return out;
	}
	cell *setCellswithNoOrder(caro *cr, int *sA)
	{
		sA[0] = cr->setCell(val, work.cellPtr->rowVal, work.cellPtr->colVal, E_TNEAR);
		return (work.cellPtr);
	}
	void restoreCells(caro *cr, int *sA)
	{
		cr->restoreCell(sA[0], work.cellPtr->rowVal, work.cellPtr->colVal);
	}
	void postResult(pair<scoreElement, hist> &c, int tid)
	{
		result = c;
		val = tid;
	}
};

class worksToDo
{
public:
	vector<oneWork> toDoList;
	unsigned int workTaken_index = 0;
	unsigned int workCompleted_index = 0;
	int cnt0 = 0, cnt1 = 0;
	int stageCnt = 0;
	int stage = 99;
	int threadCnt = 10;
	int chkPtArray[20]; // maximum 20 checkpoint only, only envision usage of a fews
	caro *chkPt_1stCaro;
	mutex mtx, stage_mtx, print_mtx, global_mtx, debug_mtx, chkPt_mtx;
	int lasti;
	aScore global_alpha, global_beta;
	void workToDo()
	{
		for (int i = 0; i < 20; i++)
			chkPtArray[i] = 0;
	}
	void checkPoint(caro *caroPtr, int howMany, int chkId, int sleep_ms)
	{
		chkPt_mtx.lock();
		bool doCmp = (chkPtArray[chkId] + 1) == howMany;
		if (doCmp)
		{
			if (caroPtr->compare(*chkPt_1stCaro) == false)
				pause();
		}
		else
			chkPt_1stCaro = caroPtr;
		chkPtArray[chkId]++;
		chkPt_mtx.unlock();
		while (chkPtArray[chkId] < howMany)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
	}
	void update_alpha(aScore alpha)
	{
		if (alpha.greaterValueThan(global_alpha))
		{
			global_mtx.lock();
			global_alpha = alpha;
			global_mtx.unlock();
		}
	}
	void update_beta(aScore beta)
	{
		if (global_beta.greaterValueThan(beta))
		{
			global_mtx.lock();
			global_beta = beta;
			global_mtx.unlock();
		}
	}
	oneWork *old;
	int redoCnt;
	oneWork *getWork(int redoAction)
	{
		mtx.lock();
		oneWork *aptr;

		if (workTaken_index == 0)
			redoCnt = 0;
		if (workTaken_index < toDoList.size())
		{

			if (redoAction && redoCnt)
			{
				aptr = old;
				redoCnt = 0;
			}
			else
			{
				aptr = &toDoList[workTaken_index++];
				old = aptr;
				redoCnt++;
			}
		}
		else
			aptr = nullptr;
		mtx.unlock();
		return aptr;
	}
	bool slaveId(int id)
	{
		return (id > 0);
	}

	/* stage:
	 * 99:reboot state
	 *  0: st_RESET
	 *  	reset stage;
	 *  	master: wait for all slave threads to inc cnt.
	 *  	Time-out and reset threadCnt to real
	 *  1: st_SETUP
	 *  	master: after read input; slave to copy agame.hist
	 *
	 *  2: st_HUMAN_MOVE
	 *  	master: after get input of human move
	 *     slaves: get input and calculate eval (-1),
	 *  3: st_GEN_WORKS
	 *  	master: after generate todolist
	 *     slaves: get works and complete it, until no-more work
	 *  4: st_AI_MOVE
	 *  	master: create final AI move
	 *     slave: copy final move
	 *  5: st_QUIT: All threads terminate
	 *
	 *
	 *  back to stage 2
	 *
	 */
	string stageName[6] = {"RESET", "READING INPUT", "Human-input",
						   "WORK-distribution", " AI-Move", "QUIT"};

	void slaves_sync(int stageId, int sleep_ms)
	{
		while (stage != stageId)
		{
			if (all_quit)
				return;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
		stage_mtx.lock();

		stageCnt++;
		stage_mtx.unlock();
	}

	int master_setSync(int stageId, int sleep_ms, int timeout_ms)
	{
		// stuck in previous stage, wait for it sync up with everyone
#ifdef old
		return 0;
#else
		int r = timeout_ms / sleep_ms;
		stage_mtx.lock();
		stageCnt = 1;
		stage = stageId;
		stage_mtx.unlock();
		while ((stageCnt < threadCnt))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
			if (timeout_ms > 0)
				if (r-- < 0)
					return stageCnt;
		}
		/*	caroDebugOut << stage << " Master starting Stage= " << stageName[stageId]
				 << " cnt=" << stageCnt
				 << "------------------------------------------" << endl;*/
		return stageCnt;
#endif
	}

	void addWork(oneWork &awork)
	{
		toDoList.push_back(awork);
	}

	void clear()
	{
		mtx.lock();
		toDoList.clear();
		workTaken_index = workCompleted_index = 0;
		mtx.unlock();
	}

	int worksRemain()
	{
		return (toDoList.size() - workTaken_index);
	}

	pair<scoreElement, hist> *getResult()
	{
		pair<scoreElement, hist> *aptr;
		if (workCompleted_index <= workTaken_index)
		{
			aptr = &toDoList[workCompleted_index].result;
			/*	cout << "HHHHHHHHH Comp=" << workCompleted_index << " Taken"
			 << workTaken_index << " "
			 << *toDoList[workCompleted_index].cptr << " result"
			 << *aptr;
			 */
			if (aptr->first.cellPtr)
			{
				workCompleted_index++;
				return (aptr);
			}
		}
		return nullptr;
	}

	bool allWorksCompleted()
	{
		/*
		 unsigned int lasti = 0;
		 if (workCompleted_index != lasti) {
		 debugOut << "WI=" << workTaken_index << " WCI=" << workCompleted_index
		 << endl;
		 lasti = workCompleted_index;
		 }
		 */
		return ((workTaken_index == (toDoList.size())) && (workTaken_index == workCompleted_index));
	}
};

scoreElement minimax(int depth, caro &game,
					 bool maximizingPlayer,
					 scoreElement alpha,
					 scoreElement beta, vector<scoreElement> &l);
#endif /* CARO_H_ */