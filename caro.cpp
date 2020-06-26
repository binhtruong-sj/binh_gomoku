/*
 * caro.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: binht
 */
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>  // std::mutex
#include <thread> // std::thread
#include <vector>

using namespace std;
using namespace std::chrono;

//#define VERBOSE3 1
//#define DEBUGSCORING 1
#ifdef VERBOSE3
#define VERBOSE2 1
#endif

#ifdef VERBOSE2
#define VERBOSE 1
#endif

//#define VERBOSE2 1

#include "caro.h"
extern bool printScore;
extern int  printScoreNum;
extern vector<vector<scoreElement>> traceBestPath;

int search_depth = 5;
int search_width = 5;
//int setWidthDepth[][2] = { { 28,2 }, { 19,4 }, { 17, 6 }, { 15, 8 }, { 13, 12 } };
//int setWidthDepth[][2] =	{ { 28, 4 }, { 20, 6 }, { 16, 6 }, { 6, 10 }, { 4, 12 } };
int setWidthDepth[][3] = {{30, 4, 6}, {26, 8, 10}, {26, 8, 14}, {10, 8, 16}, {10, 10, 20}, {10, 10, 20}};
//int setWidthDepth[][2] = { { 14, 6 }, { 10, 8 }, { 6, 8 }, { 6, 8 }, { 4, 10 } };
//int setWidthDepth[][2] = { { 8, 2 }, { 9, 2 }, { 10, 2 }, { 11, 2 }, { 12, 4 } };
vector<char> setWidth = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};

int lowestDepth = 14;
int adjustCntArray[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
						100, 100, 100, 100};
int bitcntArray[] = {0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM6, NUM6,
					 NUM6, NUM6};
int bitcntArrayscale[] = {0, fscale(0, NUM1, 50), NUM1, fscale(NUM1, NUM2, 50),
						  NUM2, fscale(NUM2, NUM3, 50), NUM3, fscale(NUM3, NUM4, 50), NUM4, fscale(NUM4, NUM5, 50),
						  NUM5, fscale(NUM5, NUM6, 50), NUM6, NUM7, NUM8, NUM9, NUM9,
						  NUM6, NUM6};

#define BIAS_PERCENT 100 // more than opponent
#define biasDefendAdjust(val) val *BIAS_PERCENT / 100

#define dbestValue(depth)       \
	{                           \
		0x1EADBEEF, 0, in_depth \
	}
#define dworstValue(depth)      \
	{                           \
		0, 0x1D0CDEAD, in_depth \
	}
#define dbestMove(depth)                 \
	{                                    \
		0x1B1CBAD0, 0x1BIGBAD0, depth \
	}
#define dworstMove() \
	{                \
		0, 0, depth  \
	}

bool scoreAdd = true, scoreSub = false, scoreMax = false;
int debugIndexSequence[40];
int dsIndex;
bool flipCoinResult = true;
int gameOver = 0;
int gdebug = 0;
int debugScoring = 0;
int debugScoringd = 0;
int debugScoringAll = 0;

int toplevelCntMatch = 4;
int debugTid = -1;
int debugBestPath = 0;
int debugHash = 0;
int debugAI = 0;
int debugAIbest = 0, lowerMinDepth = 0, training = 0;
int debugRow, debugCol;
int interactiveDebug = 0;
int debugTrace = 0;
int underDebug = 0, lowerMin = 0, higherMin = 0;
int inspectCell;
int deltaMinDepth;
int all_quit = 0;
char whoPlay1st, whoPlay2nd;

extern worksToDo workQ;
extern bool doAlpha;

bool debugCell;
unsigned char iC[2];
bool oneShot = true;
char globalInputStr[80];
char *gisptr;
getInputStr input;

tScore::tScore()
{
	myScore = 0;
	oppScore = 0;
	ts_ret = {0, 0};
	cellPtr = nullptr;
}

tScore::tScore(scoreElement a)
{
	myScore = a.myScore;
	oppScore = a.oppScore;
	cellPtr = a.cellPtr;
	ts_ret = {0, 0};
}

hashTable ahash;

void caro::reset()
{
	moveCnt = 0;
	histOfMoves_Index = 0;
	for (int row = 1; row < size; row++)
		for (int col = 1; col < size; col++)
			board[row][col].val = E_FAR;
}

caro::~caro()
{
	// TODO Auto-generated destructor stub
}

/*
 * print out the entire table -- only for text base, new routine is needed for GUI
 */

void caro::clearScore()
{
	for (int row = 0; row <= size; row++)
		for (int col = 0; col <= size; col++)
		{
			board[row][col].score.myScore = 0;
			board[row][col].score.oppScore = 0;
			board[row][col].val &= ~(E_CAL);
		}
}

/*
 * Marking need-to-evaluate white spaces, these are nearby the X's and O's
 */

/*
 * extracting a line for the purpose of scoring
 * This will need to work with how scoring is done.  Extracting will need to go
 * in hand.  So, this will need to change depend on the method
 * ONGOING CODING -- NOT DONE
 */
#ifdef ROWCOL
Line caro::extractLine(int dir, int in_row, int in_col, bool &ending)
{
	Line aline;
	int bitcnt = 0;
	aline.val = 0;
	aline.cnt = 0;
	int unconnected = 0;
	ending = false;
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;
	rowCol crDir;
	rowCol in(in_row, in_col);
	rowCol crCurrCell(in_row, in_col);
	rowCol crOriCell = crCurrCell;
	rowCol crBackoffCell;
	// reverse the row to match up with old scheme
	crDir.setDirection(dir);
	val = cellVal(in);

	oppval = oppositeVal(val);

	crCurrCell = in;

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool isVal = false;
	aline.type = val;
	int freeEnd = 0;

	int spaced = 0;
	isVal = false;
	for (int i = 0; i < SEARCH_DISTANCE; i++)
	{
		if (cellVal(crCurrCell) & oppval)
		{
			if (isVal & (cellVal(crCurrCell) == oppval))
				aline.blocked = 1;
			break;
		}
		else if (cellVal(crCurrCell) == val)
		{
			crBackoffCell = crCurrCell;
		}
		else
		{
			if (spaced++ > 1)
			{
				freeEnd++;
				break;
			}
		}

		isVal = cellVal(crCurrCell) == val;
		crCurrCell.moveToDir(crDir);
	}
	//prompt("hit Enter q");
	if (aline.blocked || (spaced > 2))
	{
		crCurrCell = crBackoffCell;
	}
	else
	{
		// Did not find O_, switch back to ori and scan in reverse
		crCurrCell = crOriCell;
		crDir.reverse();
		if (debugCell)
		{
			caroDebugOut << endl
						 << board[in_row][in_col] << " <-- ";
		}
		int spaced = 0;
		isVal = false;
		for (int i = 0; i < SEARCH_DISTANCE; i++)
		{
			if (cellVal(crCurrCell) & oppval)
			{
				if (isVal & (cellVal(crCurrCell) == oppval))
					aline.blocked = 1;
				break;
			}
			else if (cellVal(crCurrCell) == val)
			{
				crBackoffCell = crCurrCell;
			}
			else
			{
				if (spaced++ > 1)
				{
					freeEnd++;
					break;
				}
			}
			isVal = cellVal(crCurrCell) == val;
			crCurrCell.moveToDir(crDir);
		}

		crCurrCell = crBackoffCell;
	}

	// Now reverse direction
	crDir.reverse();

	aline.val = 0;
	aline.connected = 0;
	aline.offset = 0;
	int save = 0;

	int prev_unconnected = 0;
	int continuous = 0;
	aline.continuous = 0;
	ending = false;
	for (int i = 0; i < (SEARCH_DISTANCE); i++)
	{
		aline.val = aline.val << 1;
		aline.cnt++;
		isVal = cellVal(crCurrCell) == val;
		if (cellVal(crCurrCell) == val)
		{
			if ((unconnected == 1))
			{ // double check this
				if (aline.continuous < 4)
				{
					continuous = aline.continuous;
					; // take just 1 bubble for less than 4, dont want XXXX_X
					aline.continuous = 0;
				}
				aline.connected = save + 1;
				//	if (aline.connected < 3)
				prev_unconnected++;
			}
			else
				aline.connected++;
			continuous++;
			if ((continuous >= 5) && (prev_unconnected == 0)) //&& (val == currPlay))
				ending = true;								  // need to correct true ending on upper level, where u know about
															  //current Play, i.e. block-abled or game over
			bitcnt++;
			aline.val = aline.val | 0x1;
			unconnected = 0;
		}
		else
		{
			if (aline.continuous == 0)
				aline.continuous = continuous;
			continuous = 0;
			if (aline.connected > save)
				save = aline.connected;
			aline.connected = 0;
			if (unconnected++ > 2)
				break;
		}
		crCurrCell.moveToDir(crDir);
		if (cellVal(crCurrCell) & oppval)
		{
			if (isVal & (cellVal(crCurrCell) == oppval))
				aline.blocked++;
			break;
		}
	}
	if (prev_unconnected > 1)
		aline.continuous--;

	if (aline.continuous == 0)
		aline.continuous = continuous;
	aline.cnt += freeEnd * 2;

	if (save > aline.connected)
		aline.connected = save;

	if (aline.connected == 1)
		aline.connected = 0;
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt < 5)
		aline.connected = 0;
	if (aline.connected < 4)
	{
		aline.offset -= prev_unconnected;
	}
	aline.connected = (aline.connected * 2);

	if (aline.blocked)
	{
		if (aline.connected <= 8)
			aline.offset -= 2;
	}
	if ((val == aiVal) && (aline.connected >= 6))
		aline.offset++;

	return aline;
}
#else
Line caro::extractLine(int dir, int row, int col, bool &ending)
{
	Line aline;
	int bitcnt = 0;
	aline.val = 0;
	aline.cnt = 0;
	int unconnected = 0;
	ending = false;
	// first scan for 1 set, bound by oposite or upto 8 total, bound by 3 spaces or 1 opposite
	// then scan for these special case X?xX?X, X?XxX?X
	int val, oppval;

	val = board[row][col].val;
	oppval = oppositeVal(board[row][col].val);
	cell *currCell = &board[row][col];
	cell *oriCell = currCell;
	cell *backoffCell; //, *lastone;

	// Scan for O_ (assuming X_ turn)

	aline.blocked = 0;
	bool prevVal = false;
	aline.type = val;
	int freeEnd = 0;
	//	lastone = currCell;
	int spaced = 0;
	prevVal = false;
	for (int i = 0; i < SEARCH_DISTANCE; i++)
	{
		if (currCell->val & oppval)
		{
			if (prevVal & (currCell->val == oppval))
				aline.blocked = 1;
			break;
		}
		else if (currCell->val == val)
		{
			backoffCell = currCell;
		}
		else
		{
			if (spaced++ > 1)
			{
				freeEnd++;
				break;
			}
		}
		prevVal = currCell->val == val;

		currCell = currCell->near_ptr[dir];
	}
	//prompt("hit Enter q");
	if (aline.blocked || (spaced > 2))
	{
		currCell = backoffCell;
	}
	else
	{
		// Did not find O_, switch back to ori and scan in reverse
		currCell = oriCell;
		dir = ReverseDirection(dir);

		int spaced = 0;
		prevVal = false;
		for (int i = 0; i < SEARCH_DISTANCE; i++)
		{
			if (currCell->val & oppval)
			{
				if (prevVal & (currCell->val == oppval))
					aline.blocked = 1;
				break;
			}
			else if (currCell->val == val)
			{
				backoffCell = currCell;
			}
			else
			{
				if (spaced++ > 1)
				{
					freeEnd++;
					break;
				}
			}
			prevVal = currCell->val == val;

			currCell = currCell->near_ptr[dir];
		}
		//	prompt("hit Enter q2");

		currCell = backoffCell;
	}

	// Now reverse direction
	dir = ReverseDirection(dir);

	aline.val = 0;
	aline.connected = 0;
	aline.offset = 0;
	int save = 0;
	int prev_unconnected = 0;
	int continuous = 0;
	aline.continuous = 0;
	ending = false;
	for (int i = 0; i < (SEARCH_DISTANCE); i++)
	{
		aline.val = aline.val << 1;
		aline.cnt++;
		prevVal = currCell->val == val;
		if (currCell->val == val)
		{
			if ((unconnected == 1))
			{ // double check this
				if (aline.continuous < 4)
				{
					continuous = aline.continuous;
					; // take just 1 bubble for less than 4, dont want XXXX_X
					aline.continuous = 0;
				}
				aline.connected = save + 1;
				//	if (aline.connected < 3)
				prev_unconnected++;
			}
			else
				aline.connected++;
			continuous++;
			if ((continuous >= 5) && (prev_unconnected == 0)) //&& (val == currPlay))
				ending = true;								  // need to correct true ending on upper level, where u know about
															  //current Play, i.e. block-abled or game over
			bitcnt++;
			aline.val = aline.val | 0x1;
			unconnected = 0;
		}
		else
		{
			if (aline.continuous == 0)
				aline.continuous = continuous;
			continuous = 0;
			if (aline.connected > save)
				save = aline.connected;
			aline.connected = 0;
			if (unconnected++ > 2)
				break;
		}
		currCell = currCell->near_ptr[dir];
		if (currCell->val & oppval)
		{
			if (prevVal & (currCell->val == oppval))
				aline.blocked++;
			break;
		}
	}
	if (prev_unconnected > 1)
		aline.continuous--;

	if (aline.continuous == 0)
		aline.continuous = continuous;
	aline.cnt += freeEnd * 2;

	if (save > aline.connected)
		aline.connected = save;
	/*
	 if ((aline.connected > 2) && (prev_unconnected >= 1)) {
	 aline.connected -= prev_unconnected - 1;
	 }
	 */
	if (aline.connected == 1)
		aline.connected = 0;
	if (aline.cnt > 8)
		aline.cnt = 8;
	if (aline.cnt < 5)
		aline.connected = 0;
	if (aline.connected < 4)
	{
		aline.offset -= prev_unconnected;
	}
	aline.connected = (aline.connected * 2);

	if (aline.blocked)
	{
		if (aline.connected <= 8)
			aline.offset -= 2;
	}
	if ((val == aiVal) && (aline.connected >= 6)) // Favor offensive
		aline.offset++;

	/*
	 if ((val == aiPlay) && (aline.connected > 0))
	 aline.connected--; // go first, assume that opponent to block  (1 less)
	 */

	return aline;
}
#endif
int Line::evaluate(bool ending)
{
	// rudimentary scoring -- need to change to hybrid table lookup + fallback rudimentary (that
	// self learning)
	score = 0;
	//	int bitcnt = 0;
	if (cnt < 5)
		score = 0;
	else if (ending)
	{
		score = MAGICNUMBER; // MAGICNUMBER is not terminating, unless with ownplay
	}
	else
	{
		/*int tval = val;
		 while (tval) {
		 bitcnt = bitcnt + (tval & 1); // recalculate bitcnt
		 tval = tval >> 1;
		 }*/
		score = bitcntArrayscale[connected + offset];
	}
#ifdef HASH
	ahash.addEntry(val, connected / 2, cnt, score);
#endif
	return score;
}

void FourLines::print(ofstream &out)
{
	for (int dir = East; dir < West; dir++)
	{
		Xlines[dir].print(out);
	}
}

//#else
aScore caro::score1Cell(const int currPlay, const int row, const int col,
						bool debugThis)
{
	FourLines astar;
	bool redothisone = false;
	int doit = 1;
	if (doit)
		if (board[row][col].val & E_CAL)
			return board[row][col].score;
	aScore rtn;
	rtn.i.connectedOrCost = 0;
	do
	{
		points scores[2] = {0, 0};
		int opnVal = oppositeVal(aiVal);
		int tempVal[2];
		bool ending;
		tempVal[0] = aiVal;
		tempVal[1] = opnVal;
		//	saveScoreVal(row, col, saveScores, saveVals);
		int saveVal = board[row][col].val;
		int lookForWinner = saveVal & (PLAYERx | PLAYERo);
		for (int j = 0; j < 2; j++)
		{
			int chk = 0;
			int curVal = tempVal[j];
			int ill_6, ill_4, ill_3;
			ill_6 = ill_4 = ill_3 = 0;
			setCell(curVal, row, col, E_FAR);
			Line compareLine;
			for (int dir = East; dir < West; dir++)
			{
				astar.Xlines[dir] = extractLine(dir, row, col, ending);
				points tscore = astar.Xlines[dir].evaluate(ending);
				if (lookForWinner)
				{
					if ((astar.Xlines[dir].continuous == 5) && (lookForWinner & PLAYERx))
					{
						gameOver = PLAYERx;
					}
					else if ((astar.Xlines[dir].continuous >= 5) && (lookForWinner & PLAYERo))
					{
						gameOver = PLAYERo;
					}
					if (gameOver)
						caroDebugOut << "GAMEOVER" << endl;
				}

				if (astar.Xlines[dir].continuous >= (6))
				{
					ill_6 = 1;
				}
				else if (astar.Xlines[dir].blocked == 0)
				{
					if (astar.Xlines[dir].connected == (3 * 2))
					{
						ill_3++;
					}
					else if (astar.Xlines[dir].connected == (4 * 2))
					{
						ill_4++;
					}
				}
				if ((astar.Xlines[dir].cnt > 4) || (astar.Xlines[dir].blocked < 2))
				{
					if (astar.Xlines[dir].continuous >= 5)
					{
						chk = 4;
						//	caroDebugOut << "HS<-dir=" << dir << " " << astar.Xlines[dir];
					}
					else if ((astar.Xlines[dir].continuous == 4))
					{
						if (astar.Xlines[dir].blocked == 1)
							chk = 2; // is 3 but blocked on 1 side,
						else
							chk = 3;
						//caroDebugOut << "HS<-dir=" << dir << " " << astar.Xlines[dir];
					}
					else if ((astar.Xlines[dir].continuous == 3))
					{
						if (astar.Xlines[dir].blocked == 0)
							chk = 1;
					}
				}
				if ((tscore > NUM1) && (scores[j] > NUM1))
				{
					if (tscore < scores[j])
						scores[j] = scores[j] * 2 + tscore * 1;
					else
						scores[j] = scores[j] * 1 + tscore * 2;
				}
				else
					scores[j] = scores[j] + tscore;
				scores[j] = MIN(scores[j], MAGICNUMBER); // not to overflow
				if (chk > abs(rtn.i.connectedOrCost))
					rtn.i.connectedOrCost = chk;
				if (debugThis && debugScoring)
				{
					caroDebugOut << "CurrPlay=" << currPlay << " " << astar.Xlines[dir]
								 << displayPlay(curVal) << "_ dir=" << dir
								 << " ctnuos=" << astar.Xlines[dir].continuous
								 << " connected=" << astar.Xlines[dir].connected
								 << " blk=" << astar.Xlines[dir].blocked << " cnt="
								 << astar.Xlines[dir].cnt << "-chk" << chk
								 << "-rtn.c" << rtn.i.connectedOrCost << "-end"
								 << ending << "-score=" << scores[j] << "," << tscore
								 << " -ill" << ill_6 << ill_4 << ill_3 << endl;
				}
			}

			if ((ill_6 || (ill_4 > 1) || (ill_3 > 1)) && (curVal == PLAYERx))
			{				   // curVal, not currPlay
				scores[j] = 0; //-0x200; TODO
				rtn.i.connectedOrCost = 0;
			}
			else if (chk > abs(rtn.i.connectedOrCost))
			{
				rtn.i.connectedOrCost = chk;
			}
			if (curVal == aiVal)
				rtn.i.connectedOrCost = -rtn.i.connectedOrCost; // neg for O_ -- cheezy

			// doing this here intead of after for loop bc of how setcell works
			board[row][col].val = saveVal;
			if (lookForWinner)
				return rtn;
		}

		rtn.myScore = scores[0];
		rtn.oppScore = scores[1];
		board[row][col].score = rtn;
		// only turn on CAL if NEAR or TNEAR
		if (doit)
			if (abs(rtn.i.connectedOrCost) < 3)
				if (board[row][col].val & (E_NEAR | E_TNEAR))
				{								  // redudant!
					board[row][col].val |= E_CAL; // will not be re-evaluate
					if ((row == board_size) || (col == 16))
					{
						caroDebugOut << "col=" << col << " row=" << row << endl;
						char ach;
						cin >> ach;
					}
				}
		/*if ((debugThis && (debugCell || debugScoring))) {
		 caroDebugOut << board[row][col] << board[row][col].score << endl;
		 }*/
		/*if (saveCheck) {
		 if (redothisone) {
		 debugScoringAll = 0;
		 redothisone = false;
		 break;
		 }
		 if (!(testSaveScore == rtn)) {
		 caroDebugOut << "SHORTCUT different, saveScore =" << testSaveScore
		 << " newScore =" << rtn;
		 print(SYMBOLMODE2);
		 redothisone = true;
		 debugScoringAll = 1;
		 }
		 }*/
	} while (redothisone);
	return rtn;
}
//#endif
/*
 *
 *
 *
 *
 *
 */

/*
 *
 *
 *
 */
void caro::modifyDebugFeatures(int debugId)
{
	char ffn[80] = "savefile.txt";

	switch (debugId)
	{

	case -23:
		caroDebugOut << "Turn " << FLIP(debugScoringd) << " debugScoringd" << endl;

		break;
	case -24:
		caroDebugOut << "Turn " << FLIP(debugScoringAll) << " debugScoringAll" << endl;

		break;
	case -25:
		caroDebugOut << "Turn " << FLIP(debugHash) << " debugHash" << endl;

		break;
	case -26:
		caroDebugOut << "Turn " << FLIP(debugTrace) << " debugTrace" << endl;
		break;

	case -4:
		caroDebugOut << "Turn " << FLIP(underDebug) << " underDebug" << endl;
	case -5:
		caroDebugOut << "Turn " << FLIP(lowerMin) << " lowerMin" << endl;
	case -6:
		caroDebugOut << "Turn " << FLIP(higherMin) << " higherMin" << endl;
	case -3:
		caroDebugOut << "Turn " << FLIP(debugScoring) << " debugScoring" << endl;

		break;
	case -98:
		caroDebugOut << "Turn " << FLIP(docheck) << " docheck" << endl;
		break;
	case -99:
		caroDebugOut << "Turn " << FLIP(interactiveDebug) << " interactiveDebug"
					 << endl;
		break;
	case -13:

		for (int i = 1; i < 1000; i++)
		{
			sprintf(ffn, "savefile%d.txt", i);

			caroDebugOut << "filename =" << ffn;
			ifstream ifile(ffn);
			if (ifile)
			{
				ifile.close();
				continue;
			}
			ofstream ofile(ffn);
			if (ofile)
			{
				char ans[80];
				caroDebugOut << " Writing to " << ffn << " ?" << endl;
				cin >> ans;
				if ((ans[0] == 'n') || (ans[0] == 'N'))
				{
					ofile.close();
					continue;
				}
				ofile << *this;
				caroDebugOut << *this;
				ofile.close();
				break;
			}
		}
		break;
	default:
		break;
	}
}
bool betterMove(scoreElement &a, scoreElement &b)
{
	return ((a.myScore + a.oppScore) > (b.myScore + b.oppScore));
}
bool betterValue(scoreElement &a, scoreElement &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) > scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}
bool betterSubtractiveValue(scoreElement &a, scoreElement &b)
{
	return ((a.myScore - a.oppScore) > (b.myScore - b.oppScore));
}
bool betterValue(aScore &a, scoreElement &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) > scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}
bool betterSubtractiveValue(aScore &a, scoreElement &b)
{
	return (scoreMATH(0, 0, a.myScore, a.oppScore) > scoreMATH(0, 0, b.myScore, b.oppScore));
}
bool betterValue(scoreElement &a, aScore &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) > scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}
bool betterSubtractiveValue(scoreElement &a, aScore &b)
{
	return (scoreMATH(0, 0, a.myScore, a.oppScore) > scoreMATH(0, 0, b.myScore, b.oppScore));
}
bool lessValue(scoreElement &a, scoreElement &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) < scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}
bool lessValue(scoreElement &a, aScore &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) < scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}
bool lessValue(aScore &a, scoreElement &b)
{
	return (scoreMATH(scoreAdd, scoreMax, a.myScore, a.oppScore) < scoreMATH(scoreAdd, scoreMax, b.myScore, b.oppScore));
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
auto start = high_resolution_clock::now();

inline int caro::createNodeList(int thisNodeCnt, int in_NodeCnt,
								vector<scoreElement> &bestScoreArray, const int highestConnected,
								scoreElement &bestScore)
{
	sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
	thisNodeCnt = MIN(in_NodeCnt, (int)bestScoreArray.size()); // size of bestScoreArray can be smaller than "width"
	// prunning -- necessary to remove bad moves
	if (highestConnected > 3)
	{
		int size = 0;
		for (int i = 0; i < (int)(bestScoreArray.size()); i++)
		{
			if (bestScoreArray[i].bestMove() == 0)
				break;
			if (printScores(*printScoreNumPtr,3))
				caroDebugOut << bestScoreArray[i] << " i" << i << " c" << bestScoreArray[i].i.connectedOrCost << " hc" << highestConnected;
			int deltaCcnt = abs(abs(bestScoreArray[i].i.connectedOrCost) - (highestConnected));

			if ((abs(bestScoreArray[i].i.connectedOrCost) >= (highestConnected)) ||
				(((highestConnected) > 3) && (deltaCcnt < 3) && (abs(bestScoreArray[i].i.connectedOrCost) > 2)))
			{
				bestScoreArray[size++] = bestScoreArray[i];
				if (printScores(*printScoreNumPtr,3))
					caroDebugOut << "  i" << i << " c" << bestScoreArray[i].i.connectedOrCost;
			}
			if (printScores(*printScoreNumPtr,3))
				caroDebugOut << endl;
		}
		thisNodeCnt = size;
		// highestConnected = 0;
	}
	else
	{
		int i;
		for (i = 0; i < (int)(bestScoreArray.size()); i++)
			if (bestScoreArray[i].bestMove() <= MINSCORE)
				break;
		thisNodeCnt = MIN(i, in_NodeCnt);
	}
	bestScore = bestScoreArray[0]; // BestScore
	return thisNodeCnt;
}

int caro::debugSetup(int thisNodeCnt, int in_depth, int currPlay, bool topLevel,
					 bool debugThis, int terminated, vector<scoreElement> &bestScoreArray,
					 debugid &debug, vector<scoreElement> &returnScoreArray)
{
	char ach;
	int inputRemain = 0;
	do
	{
		for (int i = 0; i < thisNodeCnt; i++)
		{
			sprintf(ostr, "\t<%d,%d>", in_depth, i);
			caroDebugOut << ostr;

			sprintf(ostr, "%C", convertCellToStr(currPlay));
			caroDebugOut << ostr;

			caroDebugOut << bestScoreArray[i] << "|";
			if (i % 2 == 0)
				caroDebugOut << endl;
		}
		caroDebugOut << endl;
		if (topLevel && !debugThis)
			break;
		if (debugThis && (terminated == 0))
		{
			int n;
			int l, row, col;
			char inputStr[40], ccol;
			debug.id.clear();
			vector<scoreElement> saveList = bestScoreArray;
			int saveW = thisNodeCnt;
			do
			{
				n = 1;
				thisNodeCnt = saveW;
				bestScoreArray = saveList;
				caroDebugOut
					<< "Enter # for debug (e={new order}, a ={all, no debug}, i={in order, with debug on specify #}"
					<< " width=" << thisNodeCnt << endl;
				for (int size = 0; size <= thisNodeCnt;)
				{
					inputRemain = input.mygetstr(inputStr);
					l = strlen(inputStr);
					if (inputStr[0] == 'e')
						break;
					else if (inputStr[0] == 'a')
						break;
					else if (inputStr[0] == 'A')
						break;

					if (islower(inputStr[l - 1]))
					{
						sscanf(inputStr, "%d%c", &row, &ccol);
						col = ccol - 'a' + 1;
						returnScoreArray.push_back(bestScoreArray[size]);
						returnScoreArray[size].cellPtr = &board[row][col];
						size++;
					}
					else
					{
						sscanf(inputStr, "%d", &n);
						if (n < 0)
						{
							modifyDebugFeatures(n);
							break;
						}
						else
						{
							returnScoreArray.push_back(bestScoreArray[n]);
							debug.id.push_back(n);
							size++;
						}
					}
				}
			} while (n < 0);
			if (inputStr[0] == 'e')
			{
				debug.id.clear();
				for (unsigned int i = 0; i < returnScoreArray.size(); i++)
				{
					debug.id.push_back(i);
				}
				bestScoreArray.clear();
				bestScoreArray = returnScoreArray;
			}
			else
			{
				// a or A
				returnScoreArray.clear();
				for (int i = 0; i < thisNodeCnt; i++)
				{
					returnScoreArray.push_back(bestScoreArray[i]); // initialize with prelim
				}
				if (inputStr[0] == 'a')
					debug.id.clear();
			}
			thisNodeCnt = returnScoreArray.size();
		}
		for (int i = 0; i < (int)(returnScoreArray.size()); i++)
		{
			if (debug.find(i))
				caroDebugOut << "Debug ";
			else
				caroDebugOut << "      ";

			caroDebugOut << returnScoreArray[i] << endl;
		}
		if (inputRemain == 0)
		{
			caroDebugOut << "Is debug specification correct?" << endl;
			cin >> ach;
		}
		else
		{
			ach = 'y';
		}
	} while (ach != 'y');
	return thisNodeCnt;
}

string gidCnt(string gid, char a)
{
	return (gid + "@" + a);
}

scoreElement caro::evalAllCell(int incrementalDepth, const int currPlay, int in_NodeCnt, const int in_depth,
							   const int fixedMinDepth, bool aisMax, scoreElement alpha, scoreElement beta, bool debugThis,
							   bool &redo, traceCell *const callerTrace, tracerArray &localCurr)
{
	const int min_depth = fixedMinDepth;
	bool isMax = -aisMax;
	cell *cPtr;
	scoreElement scoreOfaMove, returnScore, termScore;
	int nextPlay = oppositeVal(currPlay);
	int terminated = 0;
	int thisNodeCnt = in_NodeCnt;
	debugid debug;
	bool redoNext;
	int nextLevelWidth = in_NodeCnt; //nextWidth(depth,width);
	tracerArray localBest(T_ARR_MAX);
	vector<scoreElement> bestScoreArray, returnScoreArray;
	vector<scoreElement> saveReturnScore;
	char gids[40], *gid;

	if (in_depth > 0)
	{
		gid = gids;
		sprintf(gids, "%02d#", incrementalDepth);
		localCurr.tarray[incrementalDepth].getstr(&gids[3]);
	}
	else
		gid = "--Top-#";

	debugCell = false;
	int debugThisIndex = -1;
	if (debugIndexSequence[incrementalDepth] >= 0)
	{
		if ((incrementalDepth == 0) || ((incrementalDepth > 0) && (debugIndexSequence[incrementalDepth - 1] < 0)))
		{
			debugThisIndex = debugIndexSequence[incrementalDepth];
			if ((in_depth < 0) || ((int)localCurr.tarray[incrementalDepth].index != debugThisIndex))
			{
				debugThisIndex = -1;
			}
			else
			{
				debugIndexSequence[incrementalDepth] = -1; // single shot -- removing it
				caroDebugOut << gidCnt(gid,'a')  << "BBT "
							 << "Depth=" << incrementalDepth << " index " << debugThisIndex << " on " << endl;
				if (debugIndexSequence[incrementalDepth + 1] == -1)
				{
					debugThisIndex = -2;
					print();
				}
			}
		}
	}
	if (debugThis)
	{
		caroDebugOut << "__________________" << endl;
		printTrace();
	}
	if (lowerMin)
	{
		in_NodeCnt -= 2;
		lowerMin = 0;
	}
	if (higherMin)
	{
		in_NodeCnt += 2;
		higherMin = 0;
	}
	termScore.myScore = termScore.oppScore = 0;
	if (0)
	{
		print(SYMBOLMODE2, 0);
		print(SCOREMODE, 0);

		char ach;
		cin >> ach;
	}
	// reset boardScore -- this is score for the previous move made
	scoreElement bestMoveinAll;
	int highestConnected = 0;
	//localCurr.copyBeforeIndex(incrementalDepth, topBest);

	for (int row = 1; row < size; row++)
	{
		if (terminated && (in_depth >= min_depth))
		{
			break;
		}
		for (int col = 1; col < size; col++)
		{
			if (board[row][col].val & (E_NEAR | E_TNEAR))
			{
				scoreOfaMove.cellPtr = &board[row][col];
				scoreOfaMove = eval1Cell(currPlay, scoreOfaMove.cellPtr, debugThis);
				//		cout << "bestScore " << bestScore << endl;
				//		altScore = score1Cell(currPlay, row, col, debugThis);
				//		cout << "AltScore " << altScore << endl;
				//char abcd;
				//			cin >> abcd;
				//bestMoveinAll += bestScore; // wth is this?
				scoreOfaMove.scale(currPlay == aiVal);
				scoreOfaMove.cellPtr = &board[row][col];
				if (printScores(*printScoreNumPtr,3))
					caroDebugOut << gidCnt(gid,'b')  << scoreOfaMove << endl;

				bool newBetterMove = betterMove(scoreOfaMove, bestMoveinAll);
				bool newHighConnected = false;
				bestMoveinAll = newBetterMove ? scoreOfaMove : bestMoveinAll;
				bestMoveinAll.i.connectedOrCost = incrementalDepth;
				if (abs(scoreOfaMove.i.connectedOrCost) > 2)
				{
					newHighConnected = (abs(scoreOfaMove.i.connectedOrCost) > abs(highestConnected));
					if (newHighConnected)
					{
						highestConnected = scoreOfaMove.i.connectedOrCost;
						if (debugThis || printScores(*printScoreNumPtr,2))
						{
							caroDebugOut << "HC=" << highestConnected << "-C"
										 << scoreOfaMove.i.connectedOrCost << " bestScore" << scoreOfaMove << endl;
						}
					}
				}
				int lowScore, lowConnected;
				if (newHighConnected || newBetterMove)
				{
					lowScore = bestMoveinAll.bestMove() * LOW_THRESH_HOLD / 100;
					lowConnected = abs(highestConnected) * 50 / 100;
				}
				if ((scoreOfaMove.bestMove() > lowScore) || (abs(scoreOfaMove.i.connectedOrCost) >= lowConnected))
				{
					bestScoreArray.push_back(scoreOfaMove);
				}
				// at EVAL --  only terminate if own play is winning
				scoreOfaMove.i.connectedOrCost = incrementalDepth;
				if (scoreOfaMove.myScore >= MAGICNUMBER)
				{
					if (isMyPlay(currPlay,aiVal))
					{ // 100% verified -- Do Not Change
						scoreOfaMove.oppScore = 0;
						termScore = scoreOfaMove;
						bestMoveinAll.oppScore = 0;
						terminated = 1;
						localCurr.tarray[incrementalDepth + 1].terminate = 'W';
						localCurr.tarray[incrementalDepth + 1].assignValues(scoreOfaMove, 0xFF);
						//	caroDebugOut << "lc" << localCurr.tarray[incrementalDepth + 1] << endl;
						if (debugThis || printScores(*printScoreNumPtr,0))
							caroDebugOut << gidCnt(gid,'c')  << "AI-win=" << scoreOfaMove;
						//	break;
						return (scoreOfaMove);
					}
				}
				else if (scoreOfaMove.oppScore >= MAGICNUMBER)
				{
					if (!(isMyPlay(currPlay,aiVal)))
					{
						scoreOfaMove.myScore = 0;
						termScore = scoreOfaMove;
						bestMoveinAll.myScore = 0;
						terminated = 1;
						localCurr.tarray[incrementalDepth + 1].terminate = 'W';
						localCurr.tarray[incrementalDepth + 1].assignValues(scoreOfaMove, 0xFF);
						//	caroDebugOut << "lc" << localCurr.tarray[incrementalDepth + 1] << endl;

						if (debugThis || printScores(*printScoreNumPtr,0))
							caroDebugOut
								<< gidCnt(gid,'e')  << "Human-win=" << scoreOfaMove;
						//	break;
						return (scoreOfaMove);
					}
				}
			}
		}
	}

	if (0)
	{
		print(SYMBOLMODE2, 1);
		print(SCOREMODE, 1);
		char ach;
		cin >> ach;
	}
#ifdef DUAL_CHECK
	if (in_depth > 0)
		workQ.checkPoint(this, 2, 0, 1);
#endif
	if (printScores(*printScoreNumPtr,2))
		caroDebugOut << gidCnt(gid,'f')  << "Terminated=" << terminated << " in_depth=" << in_depth << " min_depth=" << min_depth << endl;

	if (debugThisIndex == -2)
	{
		sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
		int ss = MIN(10, bestScoreArray.size());
		if (printScores(*printScoreNumPtr,4))
			caroDebugOut << gidCnt(gid,'g')  << "bss" << bestScoreArray.size() << " ss=" << ss << endl;
		for (int i = 0; i < ss; i++)
			caroDebugOut << gidCnt(gid,'h')  << "P" << incrementalDepth << "=" << bestScoreArray[i] << endl;
	}

	if (in_depth == -1)
	{
		int i;
		caroDebugOut << gidCnt(gid,'i')  << "INDEPTH=" << in_depth << endl;
		sort(bestScoreArray.begin(), bestScoreArray.end(), betterMove);
		for (i = 0; i < (int)bestScoreArray.size(); i++)
		{
			if (bestScoreArray[i].bestMove() <= MINSCORE)
				break;
			possMove[i] = bestScoreArray[i].cellPtr;
			if (i >= 39)
				break;
		}
		possMove[i] = nullptr;
		return scoreOfaMove;
	}
	else if (in_depth == -3)
	{
		int ahc = abs(highestConnected);
		int swidth = setWidthDepth[ahc][0];
		lowestDepth = setWidthDepth[ahc][2];
		int in_NodeCnt = swidth;
		thisNodeCnt = createNodeList(thisNodeCnt, in_NodeCnt, bestScoreArray,
									 ahc, scoreOfaMove);
		caroDebugOut << gidCnt(gid,'j')  << "HighestConnected=" << ahc << " array Size="
					 << bestScoreArray.size() << " SetWidth=" << swidth
					 << " NodeCnt=" << thisNodeCnt << endl;
		for (int i = 0; i < thisNodeCnt; i++)
		{
			oneWork onePath;
			// This code is only 1 depth as supposed to 2 depth
			if (debugIndexSequence[dsIndex] == 0)
				caroDebugOut << gidCnt(gid,'k')  << "I" << incrementalDepth << "=" << i << " " << bestScoreArray[i] << endl;
			onePath.work = bestScoreArray[i];
			onePath.index = i;
			onePath.val = currPlay;
			workQ.addWork(onePath);
		}
		caroDebugOut << gidCnt(gid,'L')  << endl;
		return bestScoreArray[0];
	}
	else if ((in_depth == min_depth) && (terminated == 0))
	{
		localCurr.tarray[incrementalDepth + 1].assignValues(bestMoveinAll, 0xFF);
		localCurr.tarray[incrementalDepth + 1].terminate = 'M';
		//	localBest.copyFromIndex(incrementalDepth + 1, localCurr);
		scoreOfaMove = bestMoveinAll;
		if (printScores(*printScoreNumPtr,0))
		{
			caroDebugOut << gidCnt(gid, 'm') << " Terminal Node " << bestMoveinAll << endl;
		}
	}
	else
	{
		// SORT picking the best move. NOT bestValue
		evalCnt++;
		int next_swidth = setWidthDepth[highestConnected][0];
		int sdepth = setWidthDepth[highestConnected][1];
		int lowestDepth = setWidthDepth[highestConnected][2];
		int next_min_depth = (highestDepth - sdepth);
		in_NodeCnt = next_swidth;
		if (printScores(*printScoreNumPtr,5))
		{
			caroDebugOut << " bestScoreArray size =" << bestScoreArray.size() << endl;
		}
		thisNodeCnt = createNodeList(thisNodeCnt, in_NodeCnt, bestScoreArray,
									 highestConnected, scoreOfaMove);
		bestScoreArray.resize(in_NodeCnt);

		if (printScores(*printScoreNumPtr,3))
		{
			caroDebugOut << gidCnt(gid, 'n') << " ccnt=" << highestConnected << " swidth" << next_swidth << " sdep";
			caroDebugOut << sdepth << " lowestD" << lowestDepth << " lowerMin" << lowerMinDepth;
			caroDebugOut << " thisNodeCnt=" << thisNodeCnt << " Terminate=" << terminated << " in_depth=" << in_depth << " min_depth=" << min_depth << endl;
		}

		sdepth = lowerMinDepth ? sdepth - 2 : sdepth;
		/*
		if (in_depth > (highestDepth - sdepth))
		{
			min_depth = (highestDepth - sdepth);
			if (min_depth < (lowestDepth + deltaMinDepth))
				min_depth = (lowestDepth + deltaMinDepth);
		}
		*/
		//---------------------------------------
		if (in_depth == (highestDepth - 1))
		{
			toplevelCnt++;
		}
		//----------------------------------
		int passStart = (in_depth == highestDepth) ? 1 : 1;
		for (int pass = passStart; pass < 2; pass++)
		{

			if (debugThis || (in_depth == highestDepth))
			{
				caroDebugOut << gidCnt(gid, 'p') << "DebugSetup "
							 << "Depth= " << in_depth;
				if (debugThis || printScores(*printScoreNumPtr,0))
				{
					caroDebugOut << "w=" << next_swidth << ", sd=" << sdepth << "maxD="
								 << highestDepth << ", in_depth = " << in_depth
								 << ", min_depth=" << min_depth << endl;
					caroDebugOut << "Alpha " << alpha << " beta " << beta << endl;
				}

				thisNodeCnt = debugSetup(thisNodeCnt, in_depth, currPlay,
										 (in_depth == highestDepth), debugThis, terminated,
										 bestScoreArray, debug, returnScoreArray);
			}
			// unless this is the last depth, playing the next hand (of the previous best fews)
			// Recursively call to evaluate that play. The next call is for the opponent hand.
			//	bestScore.connectedOrCost = depth;
			scoreOfaMove = bestMoveinAll;
			//--------------------------------------
#ifdef CHECK
			caro backup(15);
			if (docheck)
				save(backup);
#endif
			int adjustDepth = 0;
			auto topLevelStart = high_resolution_clock::now();
			bool tooLong = false;

			if ((terminated == 0) && (in_depth > min_depth))
			{
				if (isMax)
					scoreOfaMove = dworstValue(0);
				else
					scoreOfaMove = dbestValue(0);
				if (in_depth == highestDepth)
				{
					desiredRuntime = runtimeInMicroSecond / (thisNodeCnt) / 8;
				}
				if (printScores(*printScoreNumPtr,incrementalDepth) || (debugThisIndex >= 0))
				{
					for (int ii = 0; ii < (MIN(10,thisNodeCnt)); ii++)
					{
						oneWork onePath;
						// This code is only 1 depth as supposed to 2 depth
						caroDebugOut << gidCnt(gid, 'q') << "P" << incrementalDepth << "=" << ii << "/" << thisNodeCnt << " " << displayPlay(currPlay) << bestScoreArray[ii] << endl;
					}
					caroDebugOut << endl;
				}
				for (int i = 0; i < thisNodeCnt; i++)
				{
					if (terminated)
					{
						caroDebugOut << gidCnt(gid, 'r') << "Terminated" << endl;
						break;
					}
					cPtr = bestScoreArray[i].cellPtr;
					localCurr.tarray[incrementalDepth + 1].assignValues(bestScoreArray[i], i);
					localCurr.tarray[incrementalDepth + 1].terminate = 'N';
					//	caroDebugOut << "lc1 " << localCurr.tarray[incrementalDepth + 1] << endl;
					aScore saveScores[8][SAVErESTOREdISTANCE], s1Score;
					int saveVals[8][SAVErESTOREdISTANCE], s1Val;
					saveScoreVal(cPtr->rowVal, cPtr->colVal,
								 saveScores, s1Score,
								 saveVals, s1Val);
					redoNext = false;
					setCell(currPlay, cPtr->rowVal, cPtr->colVal, E_TNEAR);
#ifdef GLOBAL_ALPHA_BETA
					if (!alpha.equal(workQ.global_alpha))
					{
						caroDebugOut << gid << "NOT EQUAL alpha=" << alpha << " Glo_alpha=" << workQ.global_alpha << endl;
					}
					if (!beta.equal(workQ.global_beta))
					{
						caroDebugOut << gid << "NOT EQUAL beta=" << beta << " Glo_beta=" << workQ.global_beta << endl;
					}
					alpha = workQ.global_alpha;
					beta = workQ.global_beta;
#endif
					do
					{
						bool debugNext = false;
						debugNext = debug.find(i);
						if (in_depth == highestDepth)
						{
							start = high_resolution_clock::now();
						}
						//	int new_depth = min_depth + adjustDepth;
						returnScore = evalAllCell(incrementalDepth + 1, nextPlay, nextLevelWidth, in_depth - 1,
												  next_min_depth,
												  !isMax, alpha, beta, debugNext, redoNext,
												  nullptr, localCurr);
					} while (redoNext);
					if (debugThisIndex >= 0)
						saveReturnScore.push_back(returnScore);

					if (debugThis)
					{
						if (isMax)
							caroDebugOut << "\nMAX-";
						else
							caroDebugOut << "\nMIN-";
						sprintf(ostr, "i=%d,", i);
						caroDebugOut << ostr;

						returnScoreArray[i] = returnScore;
					}
					bool found_better = false;
					if (isMax)
					{
						found_better = betterValue(returnScore, scoreOfaMove);
						if (found_better)
						{
							scoreOfaMove = returnScore;
							bool r;
							alpha = (r = betterValue(scoreOfaMove, alpha)) ? scoreOfaMove : alpha;
							if (printScores(*printScoreNumPtr,0) && r)
								caroDebugOut << gid << "update alpha " << alpha << endl;
#ifdef GLOBAL_ALPHA_BETA
							if (newAlpha)
								workQ.update_alpha(alpha);
#endif
						}
					}
					else
					{
						found_better = betterValue(scoreOfaMove, returnScore);
						if (found_better)
						{
							scoreOfaMove = returnScore;
							bool r;
							beta = (r = betterValue(beta, scoreOfaMove)) ? scoreOfaMove : beta;
							if (printScores(*printScoreNumPtr,0) && r)
								caroDebugOut << gidCnt(gid, 's' )<< "update beta " << beta << endl;
#ifdef GLOBAL_ALPHA_BETA

							if (newBeta)
								workQ.update_beta(beta);
#endif
						}
					}

					if (found_better)
					{
						scoreOfaMove.cellPtr = bestScoreArray[i].cellPtr;
						if (printScores(*printScoreNumPtr,0))
						{
							caroDebugOut << gidCnt(gid,'T') << "New BestScore I=" << i << " bestScoreArray" << bestScoreArray[i] << " bestScore" << scoreOfaMove << endl;
							//localCurr.print(incrementalDepth+1);
							//localBest.print(incrementalDepth+1);
						}
						localBest.copyFromIndex(incrementalDepth + 1, localCurr);
						if (printScores(*printScoreNumPtr,0)) //&& (incrementalDepth == 0))
						{					//printScores(*printScoreNumPtr,1))) {
							caroDebugOut << gidCnt(gid, 't');
							caroDebugOut << gidCnt(gid,'u')  << "Trace-best:  " << incrementalDepth << "'";
							localBest.print(caroDebugOut,incrementalDepth);
							print(localBest);
						}
					}
					auto stop = high_resolution_clock::now();
					auto duration = duration_cast<microseconds>(stop - start);
					if (0 && (debugThis == 0) && !underDebug)
					{
						if (in_depth == highestDepth)
						{
							int ratio = (duration.count() * 50 / desiredRuntime);
							int save_adjustDepth = adjustDepth;
							auto tduration = duration_cast<microseconds>(stop - topLevelStart);
							tooLong = tduration.count() > runtimeInMicroSecond;
							tooLong = false;
							if (tooLong)
							{
								caroDebugOut << gidCnt(gid,'x')  << " Total exe time too long, i =" << i << " out of " << thisNodeCnt << endl;
								thisNodeCnt = i + 1;
							}
							if (ratio <= 1)
							{					  // quick execution time
								adjustDepth -= 2; // increase run time by reducing min_depth, take care not to increase too fast
							}
							else if (ratio > 200)
							{					  // Long
								adjustDepth += 2; // reduce runtime
							}
							if (abs(adjustDepth) > 10)
							{
								adjustDepth = save_adjustDepth;
							}
							if ((adjustDepth + min_depth) > in_depth)
								adjustDepth = in_depth - min_depth - 2;
							adjustDepth = 0;
						}
						else
						{
							//tooLong = duration.count() > desiredRuntime*16;
							tooLong = 0;
							if (tooLong)
							{
								if (in_depth == highestDepth - 1)
									caroDebugOut << gidCnt(gid,'y')  << "\nleaf exe time too long=" << duration.count() << " at depth ="
												 << in_depth << " i =" << i << " out of " << thisNodeCnt;
							}
							adjustDepth = 0;
						}
					}
					else
					{
						adjustDepth = 0;
					}
					restoreScoreVal(cPtr->rowVal, cPtr->colVal,
									saveScores, s1Score,
									saveVals, s1Val);
					if ((alpha.greaterValueThan(beta)))
					{
						if (debugThis || printScores(*printScoreNumPtr,0))
							caroDebugOut << gidCnt(gid,'z')  << "Alpha " << alpha << " greater tha beta " << beta << endl;
						if (doAlpha)
						{
							terminated = 1; // ???????????
							thisNodeCnt = i + 1;
							localCurr.tarray[incrementalDepth + 1].terminate = 'A';
						}
					}
				}
				if (debugThis || ((in_depth > 0) && (incrementalDepth == 0)))
				{
					printTrace();
					if (isMax)
						caroDebugOut << gidCnt(gid,'A')  << "Max";
					else
						caroDebugOut << gidCnt(gid,'B')  << "Min";
					caroDebugOut << endl;
				}
			}
		}
		//caroDebugOut << gidCnt(gid,'C') ;
		localCurr.copyFromIndex(incrementalDepth + 1, localBest);
	}
	if ((in_depth == min_depth) && (localCurr.tarray[incrementalDepth + 1].terminate == 'N'))
	{
		//	localBest.tarray[incrementalDepth + 1].terminate = 'M';
		//	localBest.copyFromIndex(incrementalDepth + 1, localCurr);
	}
	if (debugThisIndex >= 0)
	{
		for (int ii = 0; ii < thisNodeCnt; ii++)
			caroDebugOut << gidCnt(gid,'D')  << "ZZZ" << dec << ii << " " << bestScoreArray[ii] << saveReturnScore[ii] << endl;
		debugThisIndex = -1;
	}

	if (printScores(*printScoreNumPtr,0) && (in_depth == min_depth))
	{
		caroDebugOut << gidCnt(gid,'E')  << "Trace-end:  ";
		localCurr.print(caroDebugOut,incrementalDepth);
		print(localCurr);
	}

	derivativeScaling(scoreOfaMove,currPlay);
	return (scoreOfaMove);
}

hashTable::hashTable()
{
	arrayE_cnt = 0;
}
void swap2E(hEntry &a, hEntry &b)
{
	hEntry t;
	t = a;
	a = b;
	b = t;
}
void hashTable::addEntry(int line, int connected, int bitcnt, int score)
{
	int i;
	lowest = 99999999;
	lowestI = 99999999;

	for (i = 0; i < arrayE_cnt; i++)
	{
		if (arrayE[i].refcnt < lowest)
		{
			lowest = arrayE[i].refcnt;
			lowestI = i;
		}
		if ((arrayE[i].line == line) && (arrayE[i].bitcnt == bitcnt))
		{
			if ((arrayE[i].refcnt++ > lowest) && (i > lowestI))
			{
				swapcnt++;
				swap2E(arrayE[lowestI], arrayE[i]);
			}
			return;
		}
	}
	if (i > 1280)
	{
		cout << "hashE cnt too small" << endl;
	}
	else
	{
		arrayE[arrayE_cnt].line = line;
		arrayE[arrayE_cnt].connected = connected;
		arrayE[arrayE_cnt].bitcnt = bitcnt;
		arrayE[arrayE_cnt].refcnt = 1;
		arrayE[arrayE_cnt++].score = score;
		if (debugHash)
		{
			int i = arrayE_cnt - 1;
			char binary[9];
			int val = arrayE[i].line;
			toBinary(val, binary);
			sprintf(ostr, "HASH %d %8x %8d %8d %8d %8d %8s\n", i, arrayE[i].line,
					arrayE[i].connected, arrayE[i].bitcnt, arrayE[i].score,
					arrayE[i].refcnt, binary);
			cout << ostr;
		}
	}
}

void hashTable::print()
{
	cout << "Hash Table: count = " << arrayE_cnt << "swapcnt =" << swapcnt
		 << endl;
	swapcnt = 0;
	sprintf(ostr, "%4s %8s %8s %8s %8s %8s %8s", "no", "Line", "connected", "bitcnt",
			"score", "RefCnt", "Binary");
	cout << ostr;

	cout << endl;
	for (int i = 0; i < arrayE_cnt; i++)
	{
		char binary[9];
		int val = arrayE[i].line;
		toBinary(val, binary);
		sprintf(ostr, "%4d %8x %8d %8d %8d %8d %8s\n", i, arrayE[i].line,
				arrayE[i].connected, arrayE[i].bitcnt, arrayE[i].score,
				arrayE[i].refcnt, binary);
		cout << ostr;
	}
}
