//============================================================================
// Name        : gomoku.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <mutex>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <unistd.h>

//#define PRIME
#define WaitTime 05
#define AUTO
using namespace std;
using namespace std::chrono;

#include "caro.h"
#define convertCharToCol(a) isalpha(a) ? (islower(a) ? (a - 'a' + 1) : (a - 'A' + 1)) : 1
#define isX(a) a == whoPlay1st ? PLAYERx : PLAYERo
#define oppositePlayer(a) a == whoPlay1st ? PLAYERo : PLAYERx
#define prompt(a)                  \
	{                              \
		gprint_mtx.lock();         \
		mainDebugOut << a << endl; \
		cin.get();                 \
		gprint_mtx.unlock();       \
	}
extern bool scoreAdd, scoreSub, scoreMax;
extern int debugIndexSequence[40];
extern int dsIndex;
extern bool flipCoinResult;
extern int gameOver;
extern int search_depth, search_width;
extern int debugScoring, debugScoringd, debugScoringAll, debugHash, docheck;
extern int underDebug, lowerMin, higherMin, training;
extern int interactiveDebug, lowerMinDepth, inspectCell;
extern int deltaMinDepth, all_quit, lowestDepth;
extern int debugTid, toplevelCntMatch;
extern int setWidthDepth[][3];
extern vector<char> setWidth;
extern unsigned char iC[2];
extern char whoPlay1st, whoPlay2nd;
int rc;
#include "deltaScoreTable.h"

bool aiIsX, humanIsX;
int humanVal, aiVal;

int CCHK_WIN = 9;
vector<pair<int, int>> stable{{0, 0}, {0, 0}, {4, 11}, {4, 47}, {4, 191}, {4, 191 * 4}};

int MAGICNUMBER = 5;
int maxDepth = 3;
bool printScore = false, newScoreScheme = true;
vector<int> printScoreNum;
int *printScoreNumPtr = &printScoreNum[0];
pair<int, int> printScoreHighNum(0, 0);
int printScoreLowNum = 0, verboseResult = 0;
int selectAl = 0;
vector<int> replayPrevCmd;
treeTracer TTracer;
bool traceAll = false, doSaveScore = true;

int tbidCnt = 0;
ofstream saveOut;
int redoAction = 0;
char lowerWhoPlay1st, lowerWhoPlay2nd;
char ostr[180];
ofstream mainDebugOut;
string debugoutName;
bool noSurpressOutput = true, pauseHist = false;

int t0_sel;
bool noFifo;
mutex gprint_mtx;
int winner = 0;
int no_of_threads = 6;
int debugThis;
bool doAlpha = true;
bool isThereTB = false;
#ifdef AUTO
int autorun = 1;
#else
int autorun = 0;
#endif
caro agame0(15), agame1(15), agame2(15), agame3(15), agame4(15), agame5(15),
	agame6(15), agame7(15);
worksToDo workQ;
briefHist bH;
int humanRow, humanCol, aiRow, aiCol;
int whichGame = 0;
int serialized = 0;
scoreElement MAX, MIN;
vector<string> prevFansStr;
scoreTable sTable;
briefHist ANbh;
int ANbhLevel;
int debugLdepth = -1;
void splitFans(const char *fans, vector<string> &l, char *c)
{
	char *astr;
	l.resize(0);
	char mfans[500];
	strcpy(mfans, fans);
	astr = strtok(mfans, c);
	while (astr != NULL)
	{
		l.push_back(astr);
		astr = strtok(NULL, c);
	}
}
void help()
{
	mainDebugOut << "-3 debugScoring " << endl
				 << "-4 debugBestPath specify path for debug, and at which depth to "
				 << endl
				 << "display detail of line scoring " << endl
				 << "-11 debug line scoring" << endl
				 << "-23 debugScoringd, like best path, but for all instead" << endl
				 << "-24 display score for every move" << endl;
}
#define oppChar(c) ((tolower(c) == 'x') ? 'o' : 'x')
#define getRowCol(str, r, c)     \
	sscanf(str, "%d%c", &r, &c); \
	c = convertCharToCol(c);     \
	r = mapping(r);

int parseListOfCell(const char *line, vector<pair<char, char>> &rList)
{
	vector<string> list;

	splitFans(line, list, "[");
	int i = 0;
	for (auto e : list)
	{
		vector<string> slist;
		char ec[20];
		strcpy(ec, e.c_str());
		splitFans(ec, slist, "]");
		string token = slist[0];
		if (isdigit(token[0]) && token.length() > 1)
		{
			char row, col;
			getRowCol(token.c_str(), row, col);
			pair<char, char> t(row, col);
			rList.push_back(t);
			i++;
		}
	}
	return (i);
}
int readBriefHist(char *line, briefHist &bH, int rdLevel)
{
	vector<pair<char, char>> rlist;
	int order = 0;
	if (parseListOfCell(line, rlist))
	{
		if (rdLevel == 0)
			rdLevel = rlist.size() - 1;
		for (auto e : rlist)
		{
			int row = e.first;
			int col = e.second;
			if (order % 2 == 0)
			{
				bH.addMove(order, row, col, PLAYERx);
			}
			else
			{
				bH.addMove(order, row, col, PLAYERo);
			}
			if (order++ == rdLevel)
				break;
		}
		// X always plays first. It can be human or AI.  Last play is done by Human.
		if ((order) % 2)
		{
			humanVal = PLAYERo;
			aiVal = PLAYERx;
		}
		else
		{
			humanVal = PLAYERx;
			aiVal = PLAYERo;
		}
		humanIsX = humanVal == PLAYERx;
		aiIsX = aiVal == PLAYERx;
		return (rdLevel);
	}
	else
	{
		return 0;
	}
}

bool getInputBriefHist(FILE *finput, caro &agame, briefHist &bH, int rdlevel)
{
	char aline[241], *cptr;
	int row, col;
	row = 1000;
	int order = 0;
	bool readbH = false;
	do
	{
		fgets(aline, 240, finput);
		if ((aline[0] == '[') && (aline[1] == 'B') && (aline[3] == ']'))
		{
			int rdLevel = readBriefHist(&aline[7], bH, rdlevel);
			readbH = true;
			if (rdLevel != 0)
				break;
			do
			{
				fgets(aline, 240, finput);
				cptr = aline;
				while (*cptr == ' ')
				{
					cptr++;
				}
			} while (*cptr != 'A');
			break;
		}
		cptr = aline;
		col = 0;
		while (char achar = tolower(*cptr++))
		{
			if ((achar != ' ') && (achar != '<') && (achar != '>'))
			{
				if (achar == '\!')
					break;
				if (achar == '\$')
				{
					cptr = cptr + 3;
					continue;
				}
				if ((row > 15) && (achar == 'b'))
				{
					row = 0;
					col = 0;
					break;
				}
				else if ((achar == 'l') || (achar == 'a'))
				{
					if (row == 1000)
						break;
					row = -100; // quit
					break;
				}
				else if (achar == 'b')
				{
					if (col > 1)
					{
						break;
					}
					else
					{
						row++;
						col = 0;
					}
				}
				else if (achar == lowerWhoPlay1st)
				{
					bH.addMove(order++, row, col, PLAYERx);
				}
				else if (achar == lowerWhoPlay2nd)
				{
					bH.addMove(order++, row, col, PLAYERo);
				}
				else if (isdigit(achar))
				{
					if ((row < 1) || (col < 1))
						break;
					cptr--;
					int num;
					char c;
					sscanf(cptr, "%d%c", &num, &c);
					c = tolower(c);
					//mainDebugOut << "num=" << num << "c=" << c << endl;
					while (*cptr++ != ' ')
						;
					if ((c == lowerWhoPlay1st) || (c == lowerWhoPlay2nd))
					{
						c = toupper(c);
						bH.addMove(num, row, col, isX(c));
					}
					else
						col--;
				}
				col++;
			}
		}
	} while (row >= 0);
	fgets(aline, 80, finput);
	agame.reset();

	bH.setCells(&agame);
	return (true);
}

void getInput(FILE *finput, caro &agame)
{
	char aline[80], *cptr;
	int row, col;
	row = 1000;
	do
	{
		fgets(aline, 80, finput);
		cptr = aline;
		col = 0;
		while (char achar = *cptr++)
		{
			if ((achar != ' ') && (achar != '<') && (achar != '>'))
			{
				if ((row > 15) && (achar == 'b'))
				{
					row = 0;
					col = 0;
					break;
				}
				else if ((achar == 'L') || (achar == 'A'))
				{
					if (row == 1000)
						break;
					row = -100; // quit
					break;
				}
				else if (achar == 'b')
				{
					if (col > 1)
					{
						break;
					}
					else
					{
						row++;
						col = 0;
					}
				}
				else if (achar == whoPlay1st)
				{
					agame.setCell(PLAYERx, row, col, E_FNEAR);
				}
				else if (achar == whoPlay2nd)
				{
					agame.setCell(PLAYERo, row, col, E_FNEAR);
				}
				else if (isdigit(achar))
				{
					if ((row <= 1) || (col <= 1))
						break;
					cptr--;
					int num;
					char c;
					sscanf(cptr, "%d%c", &num, &c);
					while (*cptr++ != ' ')
						;
					cptr -= 2;
					agame.addMove(num, &agame.board[row][col]); // addMove need setCell right after
					col--;
				}
				col++;
			}
		}
	} while (row >= 0);
	fgets(aline, 80, finput);
	mainDebugOut << endl;
	agame.clearScore();
	if (printScores(*printScoreNumPtr, 1))
	{
		hist histArray;
		agame.extractTohistArray(PLAYERx, histArray);
		agame.print(histArray, 35);
	}
}

bool pbetterResult(oneWork &a, oneWork &b)
{
	return (a.result.first.greaterValueThan(b.result.first));
}

scoreElement topLevel(int val, int row, int col, int width, int depth,
					  bool maximizingPlayer, scoreElement alpha, scoreElement beta,
					  tracerArray &aTracer)
{
	pair<scoreElement, hist> *rResult;
	workQ.clear();
	//	agame0.print(RAWMODE, 331);
	agame0.printScoreNum = printScoreNum;
	agame0.printScoreNum.resize(maxDepth + 2);
	for (auto &e : agame0.printScoreNum)
	{
		e = printScoreLowNum;
	}
	agame0.printScoreNumPtr = &agame0.printScoreNum[0];
	int gameV = agame0.setCell(val, row, col, E_NEAR);
	if (gameV)
	{
		// switch player
		humanVal = oppositeVal(humanVal);
		aiVal = oppositeVal(aiVal);
		humanIsX = !humanIsX;
		aiIsX = !aiIsX;
		val = oppositeVal(val);
		agame0.eval1Cell(val, &agame0.board[row][col], 0);
	}
	else
	{
		agame0.eval1Cell(val, &agame0.board[row][col], 0);
	}
	mainDebugOut << agame0 << flush;

	workQ.master_setSync(st_HUMAN_MOVE, 200, -1);
	workQ.global_alpha = alpha;
	workQ.global_beta = beta;
	//scoreElement result;

	vector<scoreElement> todoList;

	int getWidth = setWidth[0];

	auto rst = agame0.currentBoardScoring(todoList, getWidth, maximizingPlayer);
	/*
	if (todoList.size() == 0)
	{
		return (rst);
	}
	*/
	if (todoList.size() == 0)
	{
		cout << "Game Over" << endl;
		exit(0);
	}
	if (printScores(printScoreLowNum, 0))
		mainDebugOut
			<< "Top-TodoList ";
	for (int i = 0; i < todoList.size(); i++)
	{
		oneWork onePath;
		if (printScores(printScoreLowNum, 0))
			mainDebugOut << todoList[i] << endl;
		onePath.work = todoList[i];
		onePath.index = i;
		onePath.val = oppositeVal(val);
		workQ.addWork(onePath);
	}
	/*
	 * SYNC
	 */
	workQ.master_setSync(st_GEN_WORKS, 200, -1);

	int i = 0;
	int scnt = 0;
	int ssize = workQ.toDoList.size();
	if (noSurpressOutput)
	{
		cout << "  /" << setw(2) << ssize << flush;
		cout << "\b\b\b";
	}
	while (workQ.allWorksCompleted() == false)
	{
		rResult = workQ.getResult();
		if (rResult == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(WaitTime));
		}
	}
	if (noSurpressOutput)
	{
		cout << "\b\b     \b\b\b\b\b";
	}
	sort(workQ.toDoList.begin(), workQ.toDoList.end(), pbetterResult);
	for (auto r : workQ.toDoList)
	{
		mainDebugOut << "BBB" << i << r.result.first << " Tid="
					 << r.val << " " << r.index << endl;
	}
	mainDebugOut << "Result= ";
	scoreElement best = workQ.toDoList[0].result.first;
	for (auto r : workQ.toDoList)
		r.result.first.printDelta(mainDebugOut, best);

	if (verboseResult)
	{
		int i = 0;
		for (auto r : workQ.toDoList)
		{
			r.result.first.printDelta(cout, best);
			if (++i == 3)
			{
				break;
			}
		}
	}
	if (traceAll)
	{
		ofstream saveBH;
		i = 0;
		for (auto r : workQ.toDoList)
		{
			hist histA;
			agame0.extractTohistArray(aiVal, histA);
			mainDebugOut << "\n[BH]==>" << histA;
			mainDebugOut << "===" << r.result.second << endl;
			if (verboseResult) {
				cout << "[BH]==>" << histA << "===" << r.result.second << " ";
			}
			agame0.print(mainDebugOut, r.result.second);

			if (++i == 3)
			{
				break;
			}
		}
	}

	return best;
}
bool isPrime(int n)
{
	// Corner case
	if (n <= 1)
		return false;

	// Check from 2 to n-1
	for (int i = 2; i < n; i++)
		if (n % i == 0)
			return false;

	return true;
}

void doPrime(int tid)
{
	/*	mainDebugOut << "WORKER" << endl;
	 mainDebugOut << *acaro << endl;
	 */
	int cnt = 0;
	for (int i = 87628374; i < 97628374634; i++)
	{
		if (isPrime(i))
		{
			if (cnt++ > 2)
				break;
		}
	}
	if (printScores(printScoreLowNum, 1))
		mainDebugOut << "T=" << tid << "cnt" << cnt << endl;
}
bool sbetterValue(scoreElement &a, scoreElement &b)
{
	return a.greaterValueThan(b);
}
#define stageName(i) ((i == TS_search_for_id) ? "search_id" : (i == TS_after_found_id) ? "found_id" : (i == TS_looking_for_best_move) ? "look_for_best" : "TS_off")

#define ActionName(i) ((i == TSA_none) ? " none " : (i == TSA_save_bestID) ? " save_bestID " : (i == TSA_save_score) ? " save_score " : " ")

TS_ACT traceBestStateMachine(bool &rerun, int depth, T_STAGE stage, T_STAGE *statePtr,
							 scoreElement &traceBestPathElement, scoreElement &e)
{
	TS_ACT action = TSA_none;
	if (stage == statePtr[0])
	{
		cout << "Depth= " << depth << " At stage " << stageName(stage);
		if (stage == TS_after_found_id)
			cout << " TB-e=" << traceBestPathElement;
		cout << " e=" << e;
		switch (stage)
		{
		case TS_search_for_id:
		{
			if ((traceBestPathElement.compareCell(e)))
			{
				statePtr[0] = TS_after_found_id;
				statePtr[1] = TS_looking_for_best_move;
				action = TSA_save_score;
			}
		}
		break;
		case TS_after_found_id:
		{
			traceBestPathElement = e;
			statePtr[0] = TS_off;
			action = TSA_found_id;
			rerun = false;
		}
		break;
		case TS_looking_for_best_move:
		{
			traceBestPathElement = e;
			statePtr[0] = TS_search_for_id;
			action = TSA_save_score;
			rerun = true;
		}
		default:
			break;
		}
		cout << " next-stage " << stageName(statePtr[0]) << ", Action = " << ActionName(action) << endl;
	}
	return action;
}
/*
 * workerBee is slave threads -- all agame's data are used
 */
void setScoreNum(vector<int> &sn, int dl, int dh, int high, int low)
{
	for (int i = 0; i <= maxDepth; i++)
	{
		if ((i > dl) && (i < (dh + 1)))
			sn[i] = high;
		else
			sn[i] = low;
	}
}
vector<hist> top3Trace;
void workerBee(int tid)
{
	caro *acaro;
	scoreElement alpha, beta;
	tracerArray currArray(T_ARR_MAX);
	float elapse;
	bool redonext = false;
	char tidName = '0' + tid;

	if (tid == 0)
		acaro = &agame0;
	else if (tid == 1)
		acaro = &agame1;
	else if (tid == 2)
		acaro = &agame2;
	else if (tid == 3)
		acaro = &agame3;
	else if (tid == 4)
		acaro = &agame4;
	else if (tid == 5)
		acaro = &agame5;
	else
		acaro = &agame6;
	//	int &printScoreNum = acaro->printScoreNum;

	// announcing your existence (setting up threadCnt)
	workQ.slaves_sync(st_RESET, 50);
	// getting data from inputfile
	workQ.slaves_sync(st_SETUP, 10);

	if ((acaro != &agame0) && (tid < no_of_threads))
	{ // agame0 has been set by 'master'
		acaro->debugOutopen(debugoutName + tidName);
		workQ.print_mtx.lock();
		bH.setCells(acaro);
		acaro->clearScore();
		workQ.print_mtx.unlock();
	}
	else
	{
		if (acaro == &agame0)
			acaro->debugOutopen(debugoutName + tidName);
	}

	//
	//	serialized = debugThis | 1;
	acaro->clearScore(); // to be removed
	while (1)
	{
		workQ.slaves_sync(st_HUMAN_MOVE, WaitTime);
		flipCoinResult = flipCoin();
		if (all_quit)
			return;
		// getting a cell here
		acaro->highestDepth = 24;
		acaro->tid = tid;
		int depth = acaro->highestDepth;
		int min_depth = acaro->highestDepth - 12;
		bool maximizingPlayer = true;
		scoreElement result;
		hist histA;

		if ((acaro != &agame0) && (tid < no_of_threads))
		{ // agame0 has been set by 'master'
			/*
			bH.setCells(acaro);
			acaro->clearScore();
			*/
			if (printScores(printScoreLowNum, 1))
				acaro->caroDebugOut << *acaro << endl;

			acaro->setCell(humanVal, humanRow, humanCol, E_NEAR);
			acaro->printScoreNum.resize(maxDepth + 2);
			for (auto &e : acaro->printScoreNum)
			{
				e = printScoreLowNum;
			}
			acaro->printScoreNumPtr = &acaro->printScoreNum[0];
		}
		acaro->caroTTracer = TTracer;

		workQ.slaves_sync(st_GEN_WORKS, WaitTime);
		if ((tid >= no_of_threads))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		else
		{
			vector<scoreElement> tidResults;
			while (workQ.worksRemain())
			{
				for (unsigned int i = 0; i < 200; i++)
				{
					oneWork *workItem_ptr;
					workItem_ptr = workQ.getWork(redoAction);
					if (workItem_ptr)
					{
						workQ.print_mtx.lock();
						if (printScores(printScoreLowNum, 1))
							acaro->caroDebugOut << "ISSUE jobt to Thread (tid) " << tid << " Qsize = "
												<< workQ.toDoList.size() << " i=" << workItem_ptr->index
												<< *workItem_ptr << endl;

						workQ.print_mtx.unlock();
#ifdef PRIME
						doPrime(tid);
#else
						int saveArray[2];
						bool redonext;
						/*	acaro->print(SYMBOLMODE2,7);
						 acaro->debugOut << "BEFORE";*/
#endif
#ifdef CHECK
						caro backup(15);
						acaro->save(backup);
#endif

						aScore saveScores[8][SAVErESTOREdISTANCE], s1Score;
						int saveVals[8][SAVErESTOREdISTANCE], s1Val;
						cell *cPtr = workItem_ptr->work.cellPtr;
						acaro->saveScoreVal(cPtr->rowVal, cPtr->colVal,
											saveScores, s1Score, saveVals, s1Val);
						result = workItem_ptr->work;
						workItem_ptr->setCellswithNoOrder(
							acaro, saveArray);

						//	acaro->print(SYMBOLMODE,8);
						if (serialized)
						{
							acaro->caroDebugOut.flush();
							workQ.debug_mtx.lock();
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
							if (printScores(printScoreLowNum, 1))
							{

								acaro->caroDebugOut << endl
													<< "---------------------------------------------------------------------------------------------" << endl;
								acaro->caroDebugOut << "Starting Thread tid=" << tid << " with " << result << endl;
								acaro->caroDebugOut << "---------------------------------------------------------------------------------------------" << endl;
							}
						}
						if (printScores(*printScoreNumPtr, 3))
							acaro->caroDebugOut << *acaro;

						int saveMax = acaro->highestDepth;
						int playVal;
						do
						{
							redonext = false;
							acaro->localCnt = 0;

							alpha = workQ.global_alpha;
							beta = workQ.global_beta;
							acaro->evalCnt = 0;

							//--------------------------------------

							maximizingPlayer = true;
							playVal = humanVal;
							if (printScores(*printScoreNumPtr, 2))
							{
								acaro->caroDebugOut << "Begin simulating ";
								acaro->caroDebugOut << result << endl;
								currArray.tarray[0].assignValues(result, workItem_ptr->index);
								acaro->caroDebugOut << currArray.tarray[0] << endl;
							}
							auto start = high_resolution_clock::now();
							scoreElement rrr;
							if (selectAl == 0)
							{
								acaro->dcnt = 0;
								acaro->evalPlay = playVal;
								vector<scoreElement> ownList;
								int ft = acaro->caroTTracer.TraceConfig(0, result, ownList);
								if (ft > 0)
								{
									setScoreNum(acaro->printScoreNum, 0, 0 + printScoreHighNum.second, printScoreHighNum.first, printScoreLowNum);
									acaro->caroDebugOut << endl
														<< "Found Trace " << *workItem_ptr->work.cellPtr << endl;
									for (int i = 0; i < 128; i++)
										tableTracker[i] = 0;
								}
								if ((acaro->caroTTracer.enableOn && (ft != -1)) || (acaro->caroTTracer.enableOn == false))
								{
									rrr = minimax(0, *acaro, false, alpha, beta, ownList);
								}
								else
								{
									rrr.myScore = rrr.oppScore = 0;
								}
								if (acaro->caroTTracer.printList(acaro->caroDebugOut, ownList))
								{
									//	acaro->print(ownList);
									acaro->extractTohistArray(aiVal, histA);
									acaro->caroDebugOut << "BestPath[BH]==>" << histA;
									acaro->caroTTracer.printCompact(acaro->caroDebugOut, ownList, histA.hArray.size());
									char eCond = ownList.back().rowVal;
									histA.extract(ownList);
									//	acaro->print(acaro->caroDebugOut, histA);
									acaro->caroDebugOut << acaro << endl;
									histA.hArray.back().name[0] = eCond;
									acaro->caroDebugOut << "BestPath[BT]==>" << histA << "\n"
														<< "End of TraceBest info"
														<< "\n"
														<< endl;
									;
									top3Trace.push_back(histA);
									acaro->caroTTracer.resetCurrentTracing();
									/*
									for (int i = 0; i < 128; i++)
									{
										char b[10];
										toBinary(i, b);
										acaro->caroDebugOut << b
															<< " " << tableTracker[i]
															<< "\n";
									}
									acaro->caroDebugOut << endl;
									*/
								}
								/*
								for (auto e : ownList)
								{
									acaro->caroDebugOut << *(e.cellPtr);
								}
								acaro->caroDebugOut << endl;
								*/
								if (ft > 0)
								{
									setScoreNum(acaro->printScoreNum, 0, 0, printScoreLowNum, printScoreLowNum);
								}
							}
							else
							{
								rrr = acaro->evalAllCell(0, playVal, 8, depth - 1,
														 min_depth, !maximizingPlayer, alpha,
														 beta, debugThis, redonext, nullptr,
														 currArray);
							}
							result.getScore(rrr, 1);
							//result.getScore(rrr);
							auto stop = high_resolution_clock::now();
							auto duration = duration_cast<microseconds>(
								stop - start);
							elapse = ((float)duration.count() / 1000000.0);
							if (printScores(printScoreLowNum, 1))
							{
								acaro->caroDebugOut << "\nBee Result =" << result << " rrr = " << rrr;
								acaro->caroDebugOut << " Time taken by function: " << elapse
													<< " seconds" << endl;
							}
#ifdef TRACER
							if (redonext)
								delete aTracer.next;
#endif
						} while (redonext);
						pair<scoreElement, hist> rr(result, histA);
						tidResults.push_back(result);
						//	acaro->debugOut << aTracer;
						//	aTracer.print(acaro->ofile);
						acaro->highestDepth = saveMax;
						bool newAlpha, newBeta;
						if (maximizingPlayer == true)
						{
							alpha = asMAX(newAlpha, result, alpha);
							if (newAlpha)
								workQ.update_alpha(alpha);
						}
						else
						{
							beta = asMIN(newBeta, result, beta);
							if (newBeta)
								workQ.update_beta(beta);
						}
						workItem_ptr->restoreCells(acaro, saveArray);
						acaro->restoreScoreVal(cPtr->rowVal, cPtr->colVal,
											   saveScores, s1Score, saveVals, s1Val);
#ifdef CHECK
						if (!acaro->compare(backup))
						{
							acaro->debugOut << "OUTSIDE " << endl;
						}
#endif
						if (printScores(printScoreLowNum, 1))
							acaro->caroDebugOut << "Done with " << result << " ElapseTime="
												<< elapse << " Tid=" << tid << endl;
						workItem_ptr->postResult(rr, tid);
						workItem_ptr->index = acaro->dcnt;
						if (serialized)
						{
							if (printScores(printScoreLowNum, 1))
							{
								acaro->caroDebugOut << endl
													<< "---------------------------------------------------------------------------------------------" << endl;
								acaro->caroDebugOut << "Completed Thread tid=" << tid << endl;
								acaro->caroDebugOut
									<< "---------------------------------------------------------------------------------------------" << endl;

								acaro->caroDebugOut.flush();
							}
							workQ.debug_mtx.unlock();
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}
					}
					else
						break;
				}
			}
			if (printScores(printScoreLowNum, 1))
			{
				sort(tidResults.begin(), tidResults.end(), sbetterValue);
				acaro->caroDebugOut << " Tid " << tid << " Result=" << endl;
				for (int i = 0; i < tidResults.size(); i++)
				{
					acaro->caroDebugOut << tidResults[i] << endl;
				}
				acaro->caroDebugOut << endl;
			}
		}
		workQ.slaves_sync(st_AI_MOVE, WaitTime);
		workQ.debug_mtx.lock();
		if (tid < no_of_threads)
		{
			acaro->setCell(aiVal, aiRow, aiCol, E_NEAR);
		}
		workQ.debug_mtx.unlock();
	}
}

void gettingNextMove(int getInput, FILE *fr, char fans[10])
{
	if (getInput)
	{
		mainDebugOut << "WAIT FOR PIPE" << endl;

		while (fgets(fans, 10, fr) == nullptr)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		fans[strlen(fans) - 1] = 0; // replace \n with null
		mainDebugOut << "Receiving " << fans;
		//	pause()
		;
	}
	else
	{
		cin >> fans;
	}
}
int reduceFans(char *fans, int index)
{
	if (fans[index++] == ';')
	{
		char ch, *fptr = fans;
		while ((ch = fans[index++]))
		{
			*fptr++ = ch;
		}
		fptr = 0;
		return 1;
	}
	else
		return 0;
}
bool parseStr(string &f, char *s, int format,
			  vector<vector<string>> &resultS, char *helpMsg)
{
	int slen = strlen(s);
	int r = strncmp(&f[0], s, slen);
	if (r == 0)
	{
		vector<string> list;
		if (f[slen] == ' ')
		{
			splitFans(&f[slen + 1], list, ",");
		}
		else
		{
			splitFans(&f[slen], list, ",");
		}
		switch (format)
		{
		case 1:
		{
			resultS.resize(1);
			for (auto e : list)
			{
				resultS[0].push_back(e);
			}
			if (list.size() == 0)
				resultS[0].push_back("1");
			break;
		}
		case 2:
		{

			int i = 0;
			for (auto e : list)
			{
				resultS.resize(i + 1);
				vector<string> l;
				splitFans(&e[0], l, "-");
				for (auto l1 : l)
				{
					resultS[i].push_back(l1);
				}
				i++;
			}
			break;
		}
		}
	}
	else
	{
		if (f[0] == '?')
			cout << s << ": " << helpMsg << endl;
	}
	return (r == 0);
}
bool parseROWCOL(string s, int &r, int &c)
{
	char cc;
	sscanf(s.c_str(), "%d%c", &r, &cc);
	convertROWCCOL(r, cc, c);
	return ((r >= 0) && (r <= 15) && (c >= 0) && (c <= 15));
}
int wh = -1, wl = -1;
void setWidthFunc(int wh, int wl)
{
	int delta = ((wh - wl) * 100) / (maxDepth);
	setWidth.resize(0);
	if (noSurpressOutput)
	{
		cout << "setting new width=";
	}
	setWidth.push_back(wh * 2);
	for (int i = 1; i < maxDepth + 2; i++)
	{
		int d = ((i - 1) * delta + 50) / 100;
		d = max((wh - d), 2);

		setWidth.push_back(d);
		if (noSurpressOutput)
		{
			cout << (d) << " ";
		}
	}
	if (noSurpressOutput)
	{
		cout << endl;
	}
}

void initThreads(int no_of_threads)
{
	if (noSurpressOutput)
	{
		cout << "No_of_threads=" << no_of_threads << endl;
	}
	bH.extract(agame0); // extract to bH so that other threads can copy
	caro *ccaro;
	agame0.clearScore();

	ccaro = &agame1;
	ccaro->reset();
	bH.setCells(ccaro);
	ccaro->clearScore();

	ccaro = &agame2;
	ccaro->reset();
	bH.setCells(ccaro);
	ccaro->clearScore();

	ccaro = &agame3;
	ccaro->reset();
	bH.setCells(ccaro);
	ccaro->clearScore();

	ccaro = &agame4;
	ccaro->reset();
	bH.setCells(ccaro);
	ccaro->clearScore();

	ccaro = &agame5;
	ccaro->reset();
	bH.setCells(ccaro);
	ccaro->clearScore();
}
void reCalScoreTable(vector<vector<pair<char, char>>> &v, vector<vector<int>> &table, vector<vector<pair<int, char>>> &r)
{
	int ixo = 0;
	for (auto xo : v)
	{
		int ir = 0;
		for (auto e : xo)
		{
			int deltaScore = e.first;
			int cCnt = e.second;
			int Score = (table[cCnt][0] * cCnt + deltaScore) + table[cCnt][1];
			r[ixo][ir].first = Score;
			r[ixo][ir].second = cCnt;
			ir++;
		}
		ixo++;
	}
}
ofstream histF;
void processFans(char *ifans)
{
	static int curTTrow;
	debugIndexSequence[0] = 0;
	dsIndex = 0;
	printScore = false;
	rc = 0;
	for (int i = 0; i < 40; i++)
	{
		debugIndexSequence[i] = -1;
	}
	vector<string> list;
	bool found = false;
	string RF_fn = "";
	do
	{
		bool saveCmd = true;
		mainDebugOut << ifans << "\n";
		string ifansSave(ifans);
		splitFans(ifans, list, ";");
		for (auto fans : list)
		{
			vector<vector<string>> ps;
			char *TPI_h = "Trace Path init -- when change debug path";
			char *TP_h = "Trace Path: # (how many),  [cell] [cell] [cell] ---, copy from TraceBest printout";
			char *TB_h = "Trace Best: cell-cell-cell,cell,cell...";
			char *TA_h = "Trace all paths";
			char *EV_h = "Eval all -- for testing purpose";
			char *SH_h = "Stop history of cmd";
			char *QU_h = "Quit";
			char *SU_h = "surpress printing -- use when running batch tests";
			char *TH_h = "no of threads use ";
			char *PL_h = "Print level";
			char *PH_h = "print leven with trace path is activated, PH #A,#B, A for level, B for how far to continue printing";
			char *RBH_h = "RBH#,[BH]==>... read briefHistory can be used to read in new game, same format as save game";

			char *RF_h = "RH #,filename, # to specify how many";
			char *WF_h = "Write file";
			char *UN_h = "undo";
			char *DE_h = "search depth";
			char *WI_h = "search width";
			char *SST_h = "Set Scoring Table: base#,offset#";
			saveCmd = true;

			if (parseStr(fans, "TPI", 1, ps, TPI_h))
			{
				TTracer.init(1, true);
			}
			else if (parseStr(fans, "CC", 1, ps, "Clear scores, forced recalculating"))
			{
				initThreads(no_of_threads);
			}
			else if (parseStr(fans, "TP", 1, ps, TP_h))
			{
				int rdLevel;
				string path;
				if (ps[0].size() > 1)
				{
					sscanf(ps[0][0].c_str(), "%d", &rdLevel);
					path = ps[0][1];
				}
				else
				{
					path = ps[0][0];
					rdLevel = -1;
				}
				vector<pair<char, char>> rlist;
				int s = parseListOfCell(path.c_str(), rlist);
				if (rdLevel < 0)
					rdLevel = s;
				//TTracer.init(1, true);
				for (auto e : rlist)
				{
					int r = e.first, c = e.second;
					TTracer.addTTraceItem(r, c);
					if (rdLevel-- == 0)
						break;
				}
				if (noSurpressOutput)
				{
					cout << "Trace paths=" << endl;
					cout << TTracer << endl;
				}
				TTracer.addRow();
			}
			else if (parseStr(fans, "TB", 2, ps, TB_h))
			{
				TTracer.init(ps.size(), true);
				int cnt = 0;
				for (auto l : ps)
				{
					for (auto l1 : l)
					{
						int r, c;
						parseROWCOL(l1, r, c);
						TTracer.addTTraceItem(r, c);
					}
					TTracer.addRow();
				}
				if (noSurpressOutput)
				{
					cout << "Trace paths=" << endl;
					cout << TTracer << endl;
				}
			}
			else if (parseStr(fans, "TA", 1, ps, TA_h))
			{
				TTracer.init(ps.size(), true);
				traceAll = (ps[0][0][0] != '0');
				if (noSurpressOutput)
				{
					cout << "Tracing all path =" << traceAll << endl;
				}
			}
			else if (parseStr(fans, "SCW", 1, ps, "Set CCK_WIN"))
			{
				sscanf(fans.c_str(), "SCW%d", &CCHK_WIN);
			}
			else if (parseStr(fans, "SDLD", 1, ps, "SET DEBUG LDEPTH #"))
			{
				sscanf(fans.c_str(), "SDLD%d", &debugLdepth);
				cout << "debugLdepth= " << debugLdepth << endl;
			}
			else if (parseStr(fans, "EV", 1, ps, EV_h))
			{
				vector<scoreElement> todoList;
				int howmany;
				MAGICNUMBER = MAGICNUMBER << 8;
				sscanf(ps[0][0].c_str(), "%d", &howmany);
				cout << aiIsX ? "AI is X" : "Human is X";
				cout << "Eval " << howmany << endl;
				agame0.printScoreNumPtr = &printScoreLowNum;
				agame0.printScoreNum.push_back(printScoreLowNum);
				agame0.clearScore();
				agame0.currentBoardScoring(todoList, howmany, true);

				saveOut.open("EVout.txt");
				cout << "TODOLIST:"
					 << "\n";
				for (auto e : todoList)
				{
					cout << e << "\n";
					saveOut << e << "\n";
				}
				MAGICNUMBER = MAGICNUMBER >> 8;
				saveOut.close();
			}
			else if (parseStr(fans, "SH", 1, ps, SH_h))
			{
				pauseHist = ps[0][0][0] == '0' ? false : true;
			}
			else if (parseStr(fans, "QU", 1, ps, QU_h))
			{
				exit(0);
			}
			else if (parseStr(fans, "qu", 1, ps, QU_h))
			{
				exit(0);
			}
			else if (parseStr(fans, "q", 1, ps, QU_h))
			{
				exit(0);
			}
			else if (parseStr(fans, "SU", 1, ps, SU_h))
			{
				noSurpressOutput = ps[0][0][0] == '0' ? true : false;
			}
			else if (parseStr(fans, "TH", 1, ps, TH_h))
			{
				no_of_threads = ps[0][0][0] - '0';
				initThreads(no_of_threads);
			}
			else if (parseStr(fans, "PL", 1, ps, PL_h))
			{
				printScoreLowNum = ps[0][0][0] - '0';

				printScoreNum.push_back(printScoreLowNum);
				printScoreNumPtr = &printScoreNum[0];
				printScore = true;
				if (noSurpressOutput)
				{
					cout << "Print low level num=" << printScoreLowNum << endl;
				}
			}
			else if (parseStr(fans, "PH", 1, ps, PH_h))
			{
				sscanf(&ps[0][0][0], "%d", &printScoreHighNum.first);
				if (ps[0].size() > 1)
					sscanf(&ps[0][1][0], "%d", &printScoreHighNum.second);
				printScore = true;
				if (noSurpressOutput)
				{
					cout << "Print high level num=" << printScoreHighNum.first << " " << printScoreHighNum.second << endl;
				}
			}
			else if (parseStr(fans, "PC", 1, ps, "Play cell[location]  will change AI to new O/X"))
			{
				char row;
				char col;
				string astr = ps[0][0];
				sscanf(fans.c_str(), "PC[%d%c]", &row, &col);
				row = mapping(row);
				col = convertCharToCol(col);
				cout << "H=" << convertValToXO(humanVal) << " Ai=" << convertValToXO(aiVal) << endl;
				agame0.setCell(humanVal, row, col, E_NEAR);
				aiVal = oppositeVal(aiVal);
				humanVal = oppositeVal(humanVal);
				humanIsX = humanVal == PLAYERx;
				aiIsX = aiVal == PLAYERx;
				cout << "Human is " << (humanIsX ? "X" : "O") << endl;
			}
			else if (parseStr(fans, "LBH", 1, ps, "Load BH path, to be used with SABH# for analysis"))
			{
				if (readBriefHist(&ps[0][0][7], ANbh, 0) == 0)
				{
					break;
				}
				else
				{
					cout << ANbh << endl;
				}
			}
			else if (parseStr(fans, "SBH", 1, ps, "Set BH #, to load game upto # for further manual testing"))
			{
				sscanf(ps[0][0].c_str(), "%d", &ANbhLevel);
				agame0.reset();
				ANbh.setCells(&agame0, ANbhLevel);
				initThreads(no_of_threads);
				if (ANbhLevel % 2)
					humanVal = PLAYERx;
				aiVal = oppositeVal(humanVal);

				humanIsX = humanVal == PLAYERx;
				aiIsX = aiVal == PLAYERx;
			}
			else if (parseStr(fans, "RBH", 1, ps, RBH_h) || parseStr(fans, "DBT", 1, ps, "Display Best Trace"))
			{
				int rdLevel;
				int index = 0;
				if (fans[0] == 'R')
				{
					sscanf(ps[0][0].c_str(), "%d", &rdLevel);
					index = 1;
				}
				briefHist abh;
				if (readBriefHist(&ps[0][index][4], abh, rdLevel) == 0)
					break;
				if (index == 1)
				{
					if (noSurpressOutput)
					{
						cout << abh << endl;
					}
					agame0.reset();

					abh.setCells(&agame0);
					initThreads(no_of_threads);

					if (noSurpressOutput)
					{
						cout << agame0 << flush;
					}
				}
				else
				{
					vector<scoreElement> tl;
					abh.convertTo(tl);
					hist hA;
					hA.extract(tl);
					agame0.print(cout, hA);
				}
			}
			else if (parseStr(fans, "SST", 1, ps, SST_h))
			{
				int baseCcnt;
				sscanf(ps[0][0].c_str(), "%d", &baseCcnt);
				int offsetScaleCcnt;
				sscanf(ps[0][1].c_str(), "%d", &offsetScaleCcnt);
				if (noSurpressOutput)
					for (int i = 0; i < 6; i++)
					{
						cout << "{" << stable[i].first << ","
							 << stable[i].second << "}" << endl;
					}
				for (int i = 2; i < 6; i++)
				{
					stable[i].first = baseCcnt;
					stable[i].second = (stable[i].second * offsetScaleCcnt) / 100;
				}
				if (noSurpressOutput)
					for (int i = 0; i < 6; i++)
					{
						cout << "{" << stable[i].first << ","
							 << stable[i].second << "}" << endl;
					}
			}
			else if (parseStr(fans, "RF", 1, ps, RF_h))
			{
				int rdLevel;
				string rdfilename = "";
				if ((ps[0].size() > 1) || (isalpha(ps[0][0][0]) == false))
				{
					sscanf(ps[0][0].c_str(), "%d", &rdLevel);
					if (ps[0].size() > 1)
					{
						rdfilename = ps[0][1];
					}
					else
					{
						if (RF_fn.length() > 0)
							rdfilename = RF_fn;
						else
							break;
					}
				}
				else
				{
					rdfilename = ps[0][0];
				}
				RF_fn = rdfilename;

				if (noSurpressOutput)
				{
					cout << rdfilename << endl;
				}
				briefHist abh;
				FILE *rdbh = fopen(&rdfilename[0], "r");

				if (rdbh)
				{
					agame0.reset();
					if (getInputBriefHist(rdbh, agame0, abh, rdLevel))
					{
						if (noSurpressOutput)
						{
							cout << abh << endl;
							//	mainDebugOut << agame0 << flush;
						}
						if (noSurpressOutput)
							cout << "Human=" << convertValToXO(humanVal) << endl;
						initThreads(no_of_threads);
					}
				}
				else
				{
					if (noSurpressOutput)
					{
						cout << rdfilename << " is not readable!" << endl;
					}
				}

				aiIsX = (aiVal == PLAYERx);
				humanIsX = (humanVal == PLAYERx);
			}
			else if (parseStr(fans, "WF", 1, ps, WF_h))
			{
				if (strlen(&fans[0]) < 4)
				{
					cout << "Error, format: \"w filename\"" << fans << endl;
				}
				string fn = ps[0][0];
				if (noSurpressOutput)
				{
					cout << "writing to " << fn << endl;
				}
				saveOut.open(fn);
				hist histArray;
				agame0.extractTohistArray(aiVal, histArray);
				//	mainDebugOut << histArray;
				saveOut << "[BH]==>" << histArray << endl;
				agame0.print(saveOut, histArray);

				saveOut.close();
			}
			else if ((parseStr(fans, "AG", 1, ps, "Animate Game")) || (parseStr(fans, "ABH", 1, ps, "Animate BriefHist[BH]==...")))
			{
				hist histArray;
				int time;
				sscanf(ps[0][0].c_str(), "%d", &time);
				if ((time < 4) || (time > 30))
					time = 2;
				if (fans[1] == 'G')
				{
					agame0.extractTohistArray(aiVal, histArray);
				}
				else
				{
					int index = 0;
					if (ps[0].size() > 1)
					{
						index = 1;
					}
					briefHist ANbh;
					if (readBriefHist(&ps[0][index][7], ANbh, 0) == 0)
					{
						break;
					}
					else
					{
						ANbh.convertTo(histArray);
					}
				}
				int l = histArray.hArray.size();
				for (int i = 1; i < l; i++)
				{
					for (int j = 0; j < time; j++)
					{
						system("clear");
						agame0.lprint(cout, histArray, i - 1);
						usleep(100000);
						system("clear");
						agame0.lprint(cout, histArray, i);
						usleep(900000);
					}
				}
			}
			else if ((parseStr(fans, "RP", 1, ps, "Replay")) || (parseStr(fans, "!", 1, ps, "Replay")))
			{
				replayPrevCmd.resize(0);
				for (auto p : ps[0])
				{
					int r;
					sscanf(p.c_str(), "%d", &r);
					replayPrevCmd.push_back(r);
				}
				saveCmd = false;
			}
			else if (parseStr(fans, "SA", 1, ps, " "))
			{
				selectAl = ps[0][0][0] - '0';
				if (noSurpressOutput)
				{
					cout << "Algorithem=" << selectAl << endl;
				}
			}
			else if (parseStr(fans, "UN", 1, ps, UN_h))
			{
				agame0.undo1move();
			}
			else if (parseStr(fans, "DG", 1, ps, "Display Game"))
			{
				if (ps[0][0][0] == '1')
				{
					hist histArray;
					agame0.extractTohistArray(aiVal, histArray);
					agame0.print(cout, histArray);
				}
				else
					cout << agame0;
			}

			else if (parseStr(fans, "DE", 1, ps, DE_h))
			{
				sscanf(ps[0][0].c_str(), "%d", &maxDepth);
				if (noSurpressOutput)
				{
					cout << "setting new depth=" << maxDepth << endl;
				}
				if (wh > 0)
					setWidthFunc(wh, wl);
			}
			else if (parseStr(fans, "WI", 1, ps, WI_h))
			{
				wh = wl = -1;
				if (ps[0].size() > 1)
				{
					sscanf(ps[0][0].c_str(), "%d", &wh);
					sscanf(ps[0][1].c_str(), "%d", &wl);
				}
				else
				{
					sscanf(ps[0][0].c_str(), "%d", &wl);
					wh = wl;
				}
				setWidthFunc(wh, wl);
			}
			else if (parseStr(fans, "h", 1, ps, WI_h))
			{
				saveCmd = false;
				int i = prevFansStr.size() - 1;
				for (auto l : prevFansStr)
					cout << "(" << i-- << ")" << l << endl;
			}
			else if (parseStr(fans, "PST", 1, ps, "Print score table"))
			{
				ofstream stFN;
				stFN.open("SCORETABLE.h");
				sTable.printTable(stFN);
				stFN.close();
			}
			else if (parseStr(fans, "VE", 1, ps, "Verbose Answers"))
			{
				verboseResult = ps[0][0][0] - '0';
			}
			else if (isdigit(fans[0]))
			{
				strcpy(ifans, &fans[0]);
				found = true;
			}
		}
		if (saveCmd)
		{
			if (pauseHist == false)
			{
				histF << ifansSave << endl;
				histF.flush();
			}
		}
		if (replayPrevCmd.size() > 0)
		{
			string as;
			int j = prevFansStr.size() - 1;
			int rr = replayPrevCmd[0];
			replayPrevCmd.erase(replayPrevCmd.begin());
			as = prevFansStr[j - rr];
			if (noSurpressOutput)
			{
				cout << ">" << as << endl;
			}
			strcpy(ifans, as.c_str());
		}
		else if (!found)
		{
			if (noSurpressOutput)
				cout << ">" << flush;
			cin >> ifans;
		}
	} while (!found);
}

int main(int argc, char *argv[])
{

	ofstream fw;
	FILE *fr;

	noFifo = (argc > 2);
	int row, col, mode, dir;
	char name, testType[80];
	FILE *finput;
	Line tempLine;
	bool redonext = false;
	float elapse = 0.0;
	int getInput;
	string gameFileName;
	bool maximizingPlayer = true;
	extern hashTable ahash;
	tracerArray TraceBestArray(20), cTracer(20);

	scoreElement result;
	scoreElement aMax, aMin;
	aMax = INF(0);
	aMin = NINF(0);
	string iline;
	//if (autorun) {
	ifstream myfile("input.txt");
	//}
	if (argc > 1)
	{
		mainDebugOut.open(argv[1]);
		debugoutName = argv[1];
	}
	else
	{
		mainDebugOut.copyfmt(std::cout);						//1
		mainDebugOut.clear(std::cout.rdstate());				//2
		mainDebugOut.basic_ios<char>::rdbuf(std::cout.rdbuf()); //3
	}
	/* generate ten random number lower than 10 */

//agame.print()
#if 1
	if (printScores(printScoreLowNum, 1))
		mainDebugOut
			<< "Enter no_of_threads  thread0_select deltaDepth debugTid WhoPlayFirst"
			<< endl;
	int seed;

	if (autorun == 0)
	{
		cin >> no_of_threads >> deltaMinDepth >> whoPlay1st >> whoPlay2nd >> getInput >> gameFileName >> seed;
	}
	else
	{
		char padding[80];

		getline(myfile, iline);
		getline(myfile, iline);
		if (printScores(printScoreLowNum, 1))
			mainDebugOut << iline << endl;
		stringstream(iline) >> padding >> no_of_threads >> padding >> deltaMinDepth >> padding >> whoPlay1st >> padding >> whoPlay2nd >> padding >> getInput >> padding >> gameFileName >> padding >> seed;
	}
	if (printScores(printScoreLowNum, 1))
		mainDebugOut << "No of Threads: " << no_of_threads << " Delta-mindept:" << deltaMinDepth << " WhoPlayFirst:" << whoPlay1st << " whoPlay2nd:" << whoPlay2nd << " getInput:" << getInput << " gameFileName:" << gameFileName << " Seed:" << seed << endl;
	MAX = INF(0);
	MIN = NINF(0);
	if (printScores(printScoreLowNum, 1))
		mainDebugOut << gameFileName << endl;
	lowerWhoPlay1st = tolower(whoPlay1st);
	lowerWhoPlay2nd = tolower(whoPlay2nd);

	thread th0(workerBee, 0);
	thread th1(workerBee, 1);
	thread th2(workerBee, 2);
	thread th3(workerBee, 3);
	thread th4(workerBee, 4);
	thread th5(workerBee, 5);
	thread th6(workerBee, 6);

	/*
	 */
	// getting accurate threadCnt -- should be exactly
	/*
	 * SYNC
	 */
	workQ.threadCnt = workQ.master_setSync(st_RESET, 100, 2000);
	if (printScores(printScoreLowNum, 1))
	{
		mainDebugOut << "THREAD CNT" << workQ.threadCnt << endl;
		mainDebugOut << "Enter filename " << endl;
	}
	string aname;
	if (autorun == 0)
	{
		cin >> aname;
	}
	else
	{
		getline(myfile, iline);
		char padding[80];
		stringstream(iline) >> padding >> aname;
	}
	string fn = "testinput.txt";
	fn = fn + aname;
	if (printScores(printScoreLowNum, 1))
		mainDebugOut << fn << endl;
	finput = fopen(fn.c_str(), "r");
	if (finput)
	{
		bool ending = false;

		getInputBriefHist(finput, agame0, bH, 1000);
		/*
		 * SYNC
		 */
		//	agame0.print(SCOREMODE, 111);
		fscanf(finput, "%d", &mode);
		dir = 0;
		while (dir < 8)
		{
			if (mode)
				fscanf(finput, "%s %d %d %c", testType, &dir, &row, &name);
			else
				scanf("%s %d %d %c", testType, &dir, &row, &name);
			if (printScores(printScoreLowNum, 1))
				mainDebugOut << "------------------" << testType << " " << dir << " " << row
							 << " " << name << endl;
			col = convertValToCol(name);
			int testc;
			if (testType[0] == '-')
			{
				sscanf(testType, "%d", &testc);
			}
			else
			{
				testc = testType[0];
			}

			switch (testc)
			{
			case -3:
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << endl
								 << "Enter Tid: ";
				cin >> debugTid;
				break;
			case -4:
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << "Turn " << FLIP(underDebug) << " underDebug" << endl;
				break;
			case -5:
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << "Turn " << FLIP(lowerMin) << " lowerMin" << endl;
				if (lowerMin)
					lowestDepth -= 2;
				lowerMin = 0;
				break;

			case -6:
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << "Turn " << FLIP(higherMin) << " higherMin" << endl;
				if (higherMin)
					lowestDepth += 2;
				higherMin = 0;
				break;
			case -7:
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << "Turn " << FLIP(lowerMinDepth) << " moreDepth" << endl;

				break;

			case 'q':
				break;
			case 'e':
				do
				{
					if (printScores(printScoreLowNum, 1))
						mainDebugOut << "enter X dir row col" << endl;
					scanf("%s %d %d %c", testType, &dir, &row, &name);
					col = convertValToCol(name);
					if (printScores(printScoreLowNum, 1))
						mainDebugOut << row << " " << col << "name=" << name << testType
									 << endl;
					agame0.setCell(isX(testType[0]), row, col, E_NEAR);
					//					agame0.print(SYMBOLMODE, 10);

					tempLine = agame0.extractLine(dir, row, col, ending);
					if (printScores(printScoreLowNum, 1))
						tempLine.print(mainDebugOut);
					agame0.restoreCell(0, row, col);

				} while (dir < 10);
				break;
			case '4':
			{
				FourLines astar;
				for (int dir = 0; dir < 4; dir++)
				{
					astar.Xlines[dir] = agame0.extractLine(dir, row, col,
														   ending);
				}
				if (printScores(printScoreLowNum, 1))
					astar.print(mainDebugOut);
				break;
			}
			case 's':
			{
				Line tempLine = agame0.extractLine(dir, row, col, ending);
				tempLine.evaluate(ending);
				if (printScores(printScoreLowNum, 1))
					tempLine.print(mainDebugOut);
			}
			break;

			case 'X': // score 1 cell
				debugScoringAll = 1;
				mainDebugOut << "Score ";
				mainDebugOut << agame0.score1Cell(PLAYERx, row, col, true);
				mainDebugOut << " for X at row " << row;
				mainDebugOut << "col " << displayPlay(col) << endl;
				break;
			case 'O': // score 1 cell
				debugScoringAll = 1;

				mainDebugOut << "Score " << agame0.score1Cell(PLAYERo, row, col, true)
							 << " for O at row " << row << "col "
							 << displayPlay(col) << endl;
				break;
				;

			case 'G':
				mode = 0;
				col = 1;
				if (printScores(printScoreLowNum, 1))
				{
					mainDebugOut << "NEW GAME" << endl;
					mainDebugOut << "Enter width, depth : " << endl;
				}
				cin >> search_width >> search_depth;
				agame0.reset();
				break;

			case 'g':
				if (printScores(printScoreLowNum, 1))
					mainDebugOut << "Opponent is White==O,  Black==X ;"
								 << "Enter X/O:";
				char gameCh;
				if (autorun)
				{
					getline(myfile, iline);
					stringstream(iline) >> gameCh;
				}
				else
					cin >> gameCh;
				if (gameCh == whoPlay1st)
				{ // X plays first
					humanVal = PLAYERo;
					aiVal = PLAYERx; // AI plays first

					if (agame0.moveCnt == 0)
					{
						agame0.setCell(aiVal, 8, 8, E_NEAR);
					}
				}
				else
				{
					humanVal = PLAYERx;
					aiVal = PLAYERo;
				}
				if (noFifo == false)
				{
					string inputFileName = "/tmp/" + gameFileName + displayPlay(humanVal);
					string outputFileName = "/tmp/" + gameFileName + displayPlay(aiVal);
					char ifn[40], ofn[40];
					strcpy(ifn, inputFileName.c_str());
					strcpy(ofn, outputFileName.c_str());
					mkfifo(ifn, 0666);
					if (printScores(printScoreLowNum, 1))
						mainDebugOut << "\nInputFile=" << inputFileName << " OutputFile="
									 << outputFileName << endl;

					fw.open(ofn);
					fr = fopen(ifn, "r");
				}
				/*
				 * if starting from empty board, 1st player is force to play [8h], 2nd play will receive and create next move.
				 * Basically, both player will wait for nextmove to be send over, but 1st player plays default move.  2nd player will endup do the first calculation
				 *
				 * if starting from a saved board, we can not do default move anymore. In this case, 2nd player will 'replay'/resend the last move.  This will then
				 * trigger the first player  to play first.
				 */
				if (gameCh == whoPlay1st)
				{
					if (agame0.moveCnt == 0)
					{
						if (noFifo)
							agame0.board[8][8].cellLocation(cout);
						else
							agame0.board[8][8].cellLocation(fw);
					}
				}
				else
				{
					if (agame0.moveCnt > 0)
					{
						if (printScores(printScoreLowNum, 1))
							mainDebugOut << "Starting game with ";
						cell *cptr = agame0.lastMoveMade();
						cptr->cellLocation(mainDebugOut);

						if (noFifo)
						{
							cptr->cellLocation(cout);
						}
						else
						{
							cptr->cellLocation(fw);
						}
					}
				}

				bH.extract(agame0); // extract to bH so that other threads can copy
				workQ.master_setSync(st_SETUP, 200, -1);

				while (col < 20)
				{
					int redo = 1;
					char fans[80], ccol;
					do
					{
						agame0.dcnt = 0;
						/*
						result = agame0.evalAllCell(0, humanVal, 20, -1, 0,
													maximizingPlayer,
													aMax,
													aMin, 0, redonext,
													nullptr, TraceBestArray);
													*/
						//	agame0.print(RAWMODE, 226);

						/*
			 *  How the game is run:
			 *  set of commands are used to 'setup' the game, also to define who play 1st, if save-game is
			 *  used, as well as changing parameters.  These commands is save in an 'input.txt' file. At the
			 *  end of the setup, a cmd switch to stdio for actual game playing, enter row/col for each move
			 *
			 *  "autorun" explain:
			 *  autorun is a mode where the code read its "cmd" from 'input.txt' instead of keyboard
			 *  one of the command is to stop reading from input.txt and switch to reading from 'keyboard'
			 *  to get game input ("Move location").
			 *  If autorun is defined, then instead of reading from keyboard, it reads from '/tmp/files"
			 *  "move location" and also write to /tmp/ instead of screen
			 */
						{
							string line;
							int lcnt = 0;
							ifstream mfile("gomokuHist.txt");
							if (mfile.is_open())
							{
								while (getline(mfile, line))
								{
									int found = -1;

									for (int i = 0; i < prevFansStr.size(); i++)
									{
										if (line == prevFansStr[i])
										{
											found = i;
											break;
										}
									}
									if (found >= 0)
									{
										prevFansStr.erase(prevFansStr.begin() + found);
									}
									prevFansStr.push_back(line);
									if (lcnt++ > 80)
									{
										prevFansStr.erase(prevFansStr.begin());
									}
								}
								mfile.close();
							}
							histF.open("gomokuHist.txt");
							histF << "8h\n";
							for (auto line : prevFansStr)
							{
								histF << line << endl;
							}
						}

						if (autorun)
						{
							getline(myfile, iline);
							stringstream(iline) >> fans;
						}
						else
						{
							if (noFifo)
							{
								if (noSurpressOutput)
									cout << ">";
								cin >> fans;
								processFans(fans);
							}
							else
								gettingNextMove(getInput, fr, fans);
						}
						if (gameOver || (fans[0] == 'q'))
						{
							if (gameOver)
							{
								mainDebugOut << "---Game Over "
											 << displayPlay(gameOver) << " wins. "
											 << endl;
								if (noFifo)
									cout << "q" << endl;
								else
									fw << "q" << endl;
							}
							else
								mainDebugOut << "uiting" << endl;

							all_quit = 1;
#ifdef HASH
							ahash.print();
#endif
							//	workQ.master_setSync(st_QUIT, 100, 2000);
							th0.join();
							th1.join();
							th2.join();
							th3.join();
							th4.join();
							th5.join();
							th6.join();
							if (noFifo == false)
							{
								fclose(fr);
								fw.close();
							}
							return (0);
						}
						else if (fans[0] == 'r')
						{
							if (printScores(printScoreLowNum, 1))
								mainDebugOut << "Setting score array" << endl;
							int w, d, l;
							for (int i = 0; i < 6; i++)
							{
								if (autorun)
								{
									getline(myfile, iline);
									stringstream(iline) >> w >> d >> l;
								}
								else
									cin >> w >> d >> l;
								l = agame0.highestDepth - l; // calculate lowestDepth
								// setWidthDepth is array to change width/depth of search based on how many is 'connected'
								// w - width, d - depth, l - is the lowest depth allowed ... depth is dynamically changed
								// l - is the lowest level ----
								setWidthDepth[i][0] = w;
								setWidthDepth[i][1] = d;
								setWidthDepth[i][2] = l;
								if (printScores(printScoreLowNum, 1))
									mainDebugOut << "(" << w << "," << d << "," << l << ")    ";
							}
							if (printScores(printScoreLowNum, 1))
								mainDebugOut << endl;
							if (autorun)
							{
								getline(myfile, iline);
								stringstream(iline) >> fans;
							}
							else
							{
								if (noSurpressOutput)
									cout << ">";
								cin >> fans;
								processFans(fans);
							}
						}
						if (fans[0] == 's')
						{
							autorun = 0;
							TTracer.init(1, false);

							if (printScores(printScoreLowNum, 1))
								mainDebugOut << "switching input " << endl;
							if (noFifo)
							{
								//	cout << ">";
								cin >> fans;
								processFans(fans);
							}
							else
								gettingNextMove(getInput, fr, fans);
						}
						if (islower(fans[strlen(fans) - 1]))
						{
							sscanf(fans, "%d%c", &row, &ccol);
						}
						else
						{
							sscanf(fans, "%d", &row);
							ccol = '=';
						}

						if (row <= -1)
						{
							agame0.cdbg.reset();
							switch (row)
							{
							case -11:
							{
								int row, col;
								char ccol[10];
								int depth, debugLine;
								agame0.cdbg.reset();

								do
								{
									if (printScores(printScoreLowNum, 1))
										mainDebugOut << "Enter row:  (-1 to quit)";
									cin >> row;
									if (row < 0)
										break;
									if (printScores(printScoreLowNum, 1))
										mainDebugOut << "Enter col: ";
									cin >> ccol;
									col = ccol[0] - 'a' + 1;
									if (printScores(printScoreLowNum, 1))
										mainDebugOut << "Enter Depth: ";
									cin >> depth;
									if (printScores(printScoreLowNum, 1))
										mainDebugOut << "Enter debugLine: ";
									cin >> debugLine;
									agame0.cdbg.add(&agame0.board[row][col],
													depth, debugLine);

								} while (row > 0);
								break;
							}

							case -13:
							{
								char ffn[80] = "savefile.txt";
								for (int i = 1; i < 1000; i++)
								{
									sprintf(ffn, "savefile%d.txt", i);
									if (printScores(printScoreLowNum, 1))
										mainDebugOut << "filename =" << ffn;
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
										if (printScores(printScoreLowNum, 1))
											mainDebugOut << " Writing to " << ffn << " ?"
														 << endl;
										cin >> ans;
										if ((ans[0] == 'n') || (ans[0] == 'N'))
										{
											ofile.close();
											continue;
										}
										ofile << agame0;
										ofile.close();
										break;
									}
								}
							}
							break;
							case -1:
								agame0.undo1move();
								agame0.print(SYMBOLMODE, 19);
								break;

							case -2:
								agame0.redo1move();
								agame0.print(SYMBOLMODE, 20);
								break;

							default:
								break;
							}
						}
						else
							redo = 0;
						if (fans[0] == '?')
							help();

					} while (redo);
					int orow;
					if ((ccol == '='))
					{
						cell *aptr = agame0.possMove[row - 1]; // -1 to not use zero
						col = aptr->colVal;
						orow = aptr->rowVal;
						row = mapping(orow);
					}
					else
					{
						col = ccol - 'a' + 1;
						orow = mapping(row);
					}

					debugThis = interactiveDebug;
					agame0.highestDepth = 24;
					row = reverseMapping(row);
					//	agame0.trace.clear();
					auto start = high_resolution_clock::now();
					humanRow = row;
					humanCol = col;
					/*
					 * SYNC
					 */
					//agame0.bestPath.clear();
					/* initialize random seed with actual date-time */
					std::srand(seed);

					result = topLevel(humanVal, row, col, 10,
									  agame0.highestDepth, maximizingPlayer,
									  aMax, aMin, TraceBestArray);

					auto stop = high_resolution_clock::now();
					auto duration = duration_cast<microseconds>(stop - start);
					elapse = ((float)duration.count() / 1000000.0);
					mainDebugOut << "Time taken by function: " << elapse << " seconds"
								 << endl;
					mainDebugOut << flush;

					if (verboseResult)
					{
						cout << "\tTime " << elapse << " ";
					}
					if (noFifo)
					{
						result.cellPtr->cellLocation(cout);
					}
					else
					{
						result.cellPtr->cellLocation(fw);
					}

					char ans[80];
					if (training)
					{
						cin >> ans;
					}
					else
					{
						ans[0] = 'y';
					}
					if (ans[0] == 'y')
					{
						agame0.setCell((int)aiVal, result.cellPtr->rowVal,
									   result.cellPtr->colVal,
									   E_NEAR);
						aiRow = result.cellPtr->rowVal;
						aiCol = result.cellPtr->colVal;
					}
					else
					{
						char ccol;
						int row;
						sscanf(ans, "%d%c", &row, &ccol);
						int col;
						convertROWCCOL(row, ccol, col);
						agame0.setCell((int)aiVal, row, col, E_NEAR);
						agame0.score1Cell(aiVal, row, col, 0);
						if (gameOver)
						{
							mainDebugOut << "Game Over " << displayPlay(gameOver)
										 << " wins. " << endl;
							all_quit = 1;
							th0.join();
							th1.join();
							th2.join();
							th3.join();
							th4.join();
							th5.join();
							th6.join();
							fclose(fr);
							fw.close();
							return 0;
						}
						aiRow = row;
						aiCol = col;
					}
					/*
					 * SYNC
					 */
					workQ.master_setSync(st_AI_MOVE, 200, -1);
				}
				agame0.reset();
				break;
			}
		}
	}
#endif
	th0.join();
	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	mainDebugOut << endl
				 << "Life is good!" << endl;
	return 0;
}

//#define SEmaxx(a, b) (betterValue(a, b) ? a : b)
//#define SEminn(a, b) (betterValue(a, b) ? b : a)

scoreElement SEmax(scoreElement &a, scoreElement &best, bool &greater)
{

	int diff = abs(a.i.connectedOrCost - best.i.connectedOrCost) / 2;
	int s;
	if (diff)
	{
		s = a.myScore >> (4 - diff);
		if (a.i.connectedOrCost > best.i.connectedOrCost)
		{
			greater = (a.myScore - s) > best.myScore;
		}
		else
		{
			greater = (a.myScore + s) > best.myScore;
		}
	}
	else

		greater = a.myScore > best.myScore;
	return (greater ? a : best);
}

scoreElement SEmin(scoreElement &a, scoreElement &best, bool &less)
{
	int diff = abs(a.i.connectedOrCost - best.i.connectedOrCost) / 2;
	int s;
	if (diff)
	{
		s = a.myScore >> (4 - diff);
		if (a.i.connectedOrCost > best.i.connectedOrCost)
		{
			less = (a.myScore - s) < best.myScore;
		}
		else
		{
			less = (a.myScore + s) < best.myScore;
		}
	}
	else

		less = a.myScore < best.myScore;
	return (less ? a : best);
}
/*
enum T_STAGE
{
	TS_off_check_if_ON,
	TS_search_for_id,
	TS_after_found_id,
	TS_looking_for_best_move
};
*/
#define modify(result, todo) result = result + todo
scoreElement minimax(int depth, caro &game,
					 const bool maximizingPlayer,
					 scoreElement alpha,
					 scoreElement beta,
					 vector<scoreElement> &parentL)
{
	int foundTrace;
	bool gfoundTrace = false;
	vector<int> &printScoreNum = game.printScoreNum;
	int ldepth = game.dcnt++;
	int lnum = 4;
	int *printScoreNumPtr;

	if (ldepth == debugLdepth)
	{
		printScoreNumPtr =
			game.printScoreNumPtr = &lnum;
		game.clearScore();
	}
	else
	{
		printScoreNumPtr = game.printScoreNumPtr = &game.printScoreNum[depth + 1];
	}
	scoreElement rst;
	vector<scoreElement> todoList;
	// Terminating condition. i.e
	// leaf node is reached
	char getWidth = (char)setWidth[depth + 1];
	if (depth == maxDepth)
	{
		rst = game.currentBoardScoring(todoList, 0, maximizingPlayer);
		rst.i.connectedOrCost = depth;
		string r = "MaxDepth";
		game.caroTTracer.terminalNode(parentL, rst, r);
		return (rst);
	}
	scoreElement doItem = game.currentBoardScoring(todoList, getWidth, maximizingPlayer);
	if (todoList.size() == 0)
	{
		if (printScores(*printScoreNumPtr, 3))
		{
			game.caroDebugOut << endl
							  << "D" << depth;
			if (doItem.i.connectedOrCost < 5)
				game.caroDebugOut << " blocked";
			game.caroDebugOut << " Win " << doItem << endl;
		}
		doItem.i.connectedOrCost = depth;
		string r = " ";
		game.caroTTracer.terminalNode(parentL, doItem, r);
		return doItem;
	}

	vector<scoreElement> myOwnL;
	aScore saveScores[8][SAVErESTOREdISTANCE], s1Score;
	int saveVals[8][SAVErESTOREdISTANCE], s1Val;

	if (printScores(*printScoreNumPtr, 1))
	{
		if (printScores(*printScoreNumPtr, 3))
		{
			hist histA;
			game.extractTohistArray(aiVal, histA);
			game.caroDebugOut << "[BH]==>" << histA;
			game.caroDebugOut << game << endl;
		}
		if (printScores(*printScoreNumPtr, 1))
		{
			game.caroDebugOut << "D" << depth << "," << ldepth << "TODOLIST^^^" << endl;
			game.caroDebugOut << todoList << endl;
		}
	}
	int setPlay = maximizerToPlay(maximizingPlayer);
	scoreElement best;
	if (maximizingPlayer)
	{
		best = MIN;
		// Recur for left and
		// right children
		for (int i = 0; i < todoList.size(); i++)
		{
			doItem = todoList[i];
			int row = doItem.cellPtr->rowVal;
			int col = doItem.cellPtr->colVal;

			game.saveScoreVal(row, col,
							  saveScores, s1Score,
							  saveVals, s1Val);
			game.setCell(setPlay, row, col, E_TNEAR);
			if (printScores(*printScoreNumPtr, 2))
			{
				if (printScores(*printScoreNumPtr, 2))
				{
					if (depth < maxDepth)
						game.caroDebugOut << "d" << depth << "+" << i << "," << ldepth << "(" << *doItem.cellPtr << " Csetting " << displayPlay(setPlay) << endl;
				}
			}
			foundTrace = game.caroTTracer.TraceConfig(depth + 1, doItem, myOwnL);
			gfoundTrace = (gfoundTrace || foundTrace >= 0);

			if (foundTrace > 0)
			{
				setScoreNum(game.printScoreNum, depth + 1, depth + 1 + printScoreHighNum.second, printScoreHighNum.first, printScoreLowNum);
				game.caroDebugOut << endl
								  << "Found Trace " << doItem << endl;
				game.clearScore();
			}
			rst = minimax(depth + 1, game,
						  false, alpha, beta, myOwnL);
			if (foundTrace > 0)
			{
				setScoreNum(game.printScoreNum, 0, 0, printScoreLowNum, printScoreLowNum);
			}
			doItem.getScore(rst);
			todoList[i] = doItem;
			game.restoreScoreVal(row, col,
								 saveScores, s1Score,
								 saveVals, s1Val);
			bool gt;
			best = SEmax(doItem, best, gt);
			if ((gt && (game.caroTTracer.trTraceActiveDepth < (depth + 1))) || (foundTrace >= 0))
				game.caroTTracer.moveTo(myOwnL, parentL);
			if (gt)
			{
				alpha = SEmax(best, alpha, gt);
			}

			// Alpha Beta Pruning

			if (beta <= alpha)
				break;
		}

		if ((printScores(*printScoreNumPtr, 2) || gfoundTrace))
		{
			game.caroDebugOut << endl
							  << "D" << depth << "," << ldepth << "TODOLIST-END" << endl;
			game.caroDebugOut << todoList;
			game.caroDebugOut << " >" << depth << best << "<b " << endl;
		}
	}
	else
	{
		best = MAX;

		// Recur for left and
		// right children
		for (int i = 0; i < todoList.size(); i++)
		{
			doItem = todoList[i];

			int row = doItem.cellPtr->rowVal;
			int col = doItem.cellPtr->colVal;
			game.saveScoreVal(row, col,
							  saveScores, s1Score,
							  saveVals, s1Val);
			game.setCell(setPlay, row, col, E_TNEAR);
			if (printScores(*printScoreNumPtr, 2))
			{
				if (printScores(*printScoreNumPtr, 3))
				{
					if (depth < maxDepth)
						game.caroDebugOut << "d" << depth << "+" << i << "," << ldepth << "(" << *doItem.cellPtr << " Csetting " << displayPlay(setPlay) << endl;
				}
			}
			foundTrace = game.caroTTracer.TraceConfig(depth + 1, doItem, myOwnL);
			gfoundTrace = (gfoundTrace || foundTrace >= 0);
			if (foundTrace > 0)
			{
				setScoreNum(game.printScoreNum, depth + 1, depth + 1 + printScoreHighNum.second,
							printScoreHighNum.first, printScoreLowNum);
				game.caroDebugOut << endl
								  << "Found Trace " << doItem << endl;
				game.clearScore();
			}
			rst = minimax(depth + 1, game,
						  true, alpha, beta, myOwnL);
			if (foundTrace > 0)
			{
				setScoreNum(game.printScoreNum, 0, 0, printScoreLowNum, printScoreLowNum);
			}
			doItem.getScore(rst);
			todoList[i] = doItem;
			game.restoreScoreVal(row, col,
								 saveScores, s1Score,
								 saveVals, s1Val);
			bool lt;
			best = SEmin(doItem, best, lt);
			if ((lt && (game.caroTTracer.trTraceActiveDepth < (depth + 1))) || (foundTrace >= 0))
				game.caroTTracer.moveTo(myOwnL, parentL);
			if (lt)
			{
				beta = SEmin(best, beta, lt);
			}

			// Alpha Beta Pruning

			if (beta <= alpha)
				break;
		}
		if (printScores(*printScoreNumPtr, 2) || (gfoundTrace))
		{
			game.caroDebugOut << endl
							  << "D" << depth << "," << ldepth << "TODOLIST-END" << endl;
			game.caroDebugOut << todoList;
			game.caroDebugOut << " >" << depth << best << "<l " << endl;
		}
	}
	if (printScores(*printScoreNumPtr, 3))
		game.caroDebugOut << game << endl;

	return best;
}
