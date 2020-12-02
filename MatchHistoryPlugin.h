#pragma once
#pragma comment(lib, "BakkesMod.lib")

#include "bakkesmod/plugin/pluginwindow.h"
#include "version.h"
#include "bakkesmod\plugin\bakkesmodplugin.h"
#include "utils/parser.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include <vector>
#include <sstream>

using namespace std;

const unsigned int totalStats = 15;
const unsigned int displayStats = 7;
const unsigned int defaultDisplayStats = 5;
const unsigned int displayTen = 11;



class MatchHistory : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	string defaultList[displayStats] = { "Playlist","MyTeamGoals","MMR","Time","Win","OtherTeamGoals","Ranked" };
	string defaultListPrint[defaultDisplayStats] = { "Playlist","Score","MMR","Time","Win" };
	int xStart = 1455;
	unique_ptr<MMRNotifierToken> notifierToken;
	unsigned long second = 1;
	unsigned long minute = 60 * second;
	unsigned long hour = 60 * minute;
	unsigned long day = 24 * hour;
	unsigned long month = 30 * day;
	unsigned long year = 365 * day;
	float myMMR;

public:
	virtual void onLoad();
	virtual void onUnload();

	void EndGame(string eventName);
	void OpenScoreBoard(string eventName);
	void CloseScoreBoard(string eventName);
	void Parse();
	void Show();
	void Hide();
	void UpdateVals();
	void Render(CanvasWrapper canvas);
	void getTotalLines(string filePath);
	void getLastTen(string filePath);
	string getFilePath();
	string fuzzyTime(unsigned long timeRaw);
	void UpdateStats(UniqueIDWrapper id);

	map<string, string> matchValues;
	map<string, int> indexer;
	map<int, string> key2index;
	ofstream matchesFile;
	unsigned int sessionWins = 0;
	unsigned int sessionLosses = 0;
	unsigned int totalLines = 0;
	bool showStats = false;
	bool looking = false;
	bool drawablesAreRegistered = false;
	string getLine;
	string unParsed[displayTen];
	string parsed[displayTen][defaultDisplayStats];


};

