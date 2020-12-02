#include "pch.h"

#include "MatchHistoryPlugin.h"

BAKKESMOD_PLUGIN(MatchHistory, "MatchHistory", "1.2", 0)

void MatchHistory::onLoad()
{
	cvarManager->log("Match History Plugin loaded!");

	gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard", bind(&MatchHistory::OpenScoreBoard, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard", bind(&MatchHistory::CloseScoreBoard, this, placeholders::_1));
	//Setup delegate event to fire on EndGame
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", bind(&MatchHistory::EndGame, this, placeholders::_1));
	notifierToken = gameWrapper->GetMMRWrapper().RegisterMMRNotifier(bind(&MatchHistory::UpdateStats, this, std::placeholders::_1));
	string filePath = getFilePath();
	getTotalLines(filePath);
}

void MatchHistory::getTotalLines(string filePath) {
	//cvarManager->log("Entering getTotalLines!");
	ifstream matchesFile(filePath);
	totalLines = 0;
	if (matchesFile.is_open()) {
		while (getline(matchesFile, getLine)) {
			if (totalLines == 0) {
				unParsed[0] = getLine;
			}
			totalLines++;
		}
		matchesFile.close();
	}
	getLastTen(filePath);
	//cvarManager->log("Exiting getTotalLines!");
}

void MatchHistory::getLastTen(string filePath) {
	//cvarManager->log("Entering getLastTen!");
	ifstream matchesFile(filePath);
	int i = 0;
	int j = displayTen - 1;
	if (matchesFile.is_open()) {
		if (totalLines >= 11) {
			while (getline(matchesFile, getLine)) {
				if ((totalLines - i < displayTen) && (j > 0)) {
					unParsed[j] = getLine;
					j--;
				}
				else {
					i++;
				}
			}
		}
		else {
			i = totalLines;
			while (getline(matchesFile, getLine)) {
				if ((i > 0) && (i != totalLines)) {  //we want to ignore the first line in the file in case of the file being less than 11 lines
					unParsed[i] = getLine;
				}
				i--;
			}
		}
		matchesFile.close();
		Parse();
	}
	//cvarManager->log("Exiting getLastTen!");
}

void MatchHistory::Parse() {
	//cvarManager->log("Entering Parse!");
	string tmp[displayTen][totalStats];
	for (int i = 0; i < displayTen; i++) {
		int j = 0;
		stringstream line(unParsed[i]);
		while (getline(line, getLine, ',')) {
			tmp[i][j] = getLine;
			j++;
		}
	}

	for (int i = 0; i < totalStats; i++) {
		indexer[tmp[0][i]] = i;
	}

	for (int i = 0; i < displayTen; i++) {
		if (i != 0) {
			if (i < totalLines) {
				string matchScore = tmp[i][indexer[defaultList[1]]] + "-" + tmp[i][indexer[defaultList[5]]];
				string playlist;
				for (int j = 0; j < defaultDisplayStats; j++) {
					if (j == 0) { // decide if the match was (R)anked or (C)asual
						if (stoi(tmp[i][indexer[defaultList[6]]]) == 1) {
							playlist = "R " + tmp[i][indexer[defaultList[j]]];
						}
						else {
							playlist = "C " + tmp[i][indexer[defaultList[j]]];
						}
						parsed[i][j] = playlist;
					}
					else if (j == 1) {
						parsed[i][j] = matchScore;
					}
					else if (j == 3) {
						parsed[i][j] = fuzzyTime(stoull(tmp[i][indexer[defaultList[j]]]));
						//cvarManager->log(tmp[i][indexer[defaultList[j]]]);
					}
					else {
						parsed[i][j] = tmp[i][indexer[defaultList[j]]];
					}
				}
			}
		}
		else {
			for (int j = 0; j < defaultDisplayStats; j++) {
				parsed[i][j] = defaultListPrint[j];
			}
		}
	}/*
	for (int i = 0; i < displayTen; i++) {
		if (i < totalLines) {
			for (int j = 0; j < defaultDisplayStats; j++) {
				//cvarManager->log(parsed[i][j] + " i=" + to_string(i) + " j=" + to_string(j) );
			}
		}
	}*/
	//cvarManager->log("Exiting Parse!");
}

void MatchHistory::Show()
{
	//cvarManager->log("Entering Show!");
	this->showStats = true;
	UpdateVals();
	gameWrapper->UnregisterDrawables();
	Parse();
	gameWrapper->RegisterDrawable(bind(&MatchHistory::Render, this, placeholders::_1));
	this->drawablesAreRegistered = true;
	//cvarManager->log("Exiting Show!");
}

void MatchHistory::Hide()
{
	//cvarManager->log("Entering Hide!");
	this->showStats = false;
	gameWrapper->UnregisterDrawables();
	this->drawablesAreRegistered = false;
	//cvarManager->log("Exiting Hide!");
}

void MatchHistory::UpdateVals()
{
	//cvarManager->log("Entering UpdateVals!");
	if (showStats)
	{
		if (!drawablesAreRegistered)
		{
			gameWrapper->UnregisterDrawables();
			gameWrapper->RegisterDrawable(bind(&MatchHistory::Render, this, placeholders::_1));
			this->drawablesAreRegistered = true;
		}
		else
		{
			if (drawablesAreRegistered)
			{
				gameWrapper->UnregisterDrawables();
				this->drawablesAreRegistered = false;
			}
		}
	}
	else
	{
		gameWrapper->UnregisterDrawables();
		this->drawablesAreRegistered = false;
		return;
	}
	//cvarManager->log("Exiting UpdateVals!");
}

void MatchHistory::Render(CanvasWrapper canvas)
{
	//cvarManager->log("Entering Render!");
	int alphaVal;
	if (!looking) {//hide if not looking at scoreboard 
		alphaVal = 0;
	}
	else {
		alphaVal = 1;
	}
	int textSize = 1;

	int distancesX[displayTen][defaultDisplayStats - 1];//the amount of chars that each element of 2d array 'parsed' holds. 
	if (totalLines == 0) {
		for (int i = 0; i < defaultDisplayStats; i++) {
			int count = 0;
			for (int k = 0; parsed[0][i][k] != '\0'; k++) {
				count++;
			}
			distancesX[0][i] = count;

		}
	}
	else {
		for (int i = 0; i < displayTen; i++) {
			for (int j = 0; j < defaultDisplayStats - 1; j++) {
				if (i < totalLines) {
					int count = 0;
					for (int k = 0; parsed[i][j][k] != '\0'; k++) {
						count++;
					}
					distancesX[i][j] = count;
				}
				else {
					distancesX[i][j] = 0;
				}
			}
		}
	}
	/*
		for (int i = 0; i < defaultDisplayStats - 1; i++) {
			cvarManager->log(to_string(distancesX[0][i]));
		}
	*/


	int distanceX[defaultDisplayStats - 1];//The distance that each column should be spaced
	for (int i = 0; i < defaultDisplayStats - 1; i++) {
		int max = 0;
		for (int j = 0; j < displayTen; j++) {
			if (j <= totalLines) {
				if (distancesX[j][i] > max) {
					max = distancesX[j][i];
				}
			}
		}
		distanceX[i] = max + 1;
	}
	/*
	for (int i = 0; i < defaultDisplayStats - 1; i++) {
		cvarManager->log(to_string(distanceX[i]));
	}
	*/
	int totalX = xStart + 3;
	for (int i = 0; i < defaultDisplayStats - 1; i++) {
		totalX += (distanceX[i] * 8);
	}

	if (totalX < 1650) {
		totalX = 1650;
	}

	//draw black background box
	Vector2 draw1start;
	Vector2 draw1end;
	draw1start.X = 1450;
	draw1start.Y = 50;
	draw1end.X = totalX;
	draw1end.Y = 204;
	canvas.SetColor(0, 0, 0, 230 * alphaVal); //alpha value is opacity, 0-255, 229.5 = 90%
	canvas.DrawRect(draw1start, draw1end);//full black box 

	//draw lines of text
	Vector2 position;
	position.X = xStart;
	position.Y = 43;
	canvas.SetColor(255, 255, 255, 255 * alphaVal);
	string lastLine = "Current session: W:" + to_string(sessionWins) + " L:" + to_string(sessionLosses);
	for (int i = 0; i < displayTen + 1; i++) {
		position.Y += 12;
		position.X = xStart;

		if (i < displayTen) {
			string temp;
			for (int j = 0; j < defaultDisplayStats - 1; j++) {
				if (j != 0) {
					position.X += (distanceX[j - 1] * 8);
					//cvarManager->log(to_string(distanceX[j - 1]));
					//cvarManager->log(to_string(position.X));
				}
				if (i > 0 && i < totalLines) { //change color of text based on win or loss
					if (stoi(parsed[i][defaultDisplayStats - 1]) >= 1) {
						/*
						string test = parsed[i][defaultDisplayStats - 1];
						int testing = stoi(test);
						//cvarManager->log("Color set to green!");
						//cvarManager->log("value in parsed is " + test);
						//cvarManager->log("as an int it is " + to_string(testing) + " j: " + to_string(j) + " i: " + to_string(i)); */
						canvas.SetColor(0, 255, 0, 255 * alphaVal); //green text for victory
					}
					else if (stoi(parsed[i][defaultDisplayStats - 1]) == 0) {
						canvas.SetColor(255, 0, 0, 255 * alphaVal); //red text for loss
					}
					else {
						canvas.SetColor(255, 255, 255, 255 * alphaVal);
					}
				}
				temp = parsed[i][j];
				canvas.SetPosition(position);
				//cvarManager->log(to_string(position.X));
				canvas.DrawString(temp, textSize, textSize);
			}
		}
		else {
			canvas.SetColor(255, 255, 255, 255 * alphaVal);
			canvas.SetPosition(position);
			canvas.DrawString(lastLine, textSize, textSize);
		}
	}
	//cvarManager->log("Exiting Render!");
	return;
}

void MatchHistory::OpenScoreBoard(string eventName) {
	//cvarManager->log("Entering OpenScoreBoard!");
	looking = true;
	Show();
	//cvarManager->log("Exiting OpenScoreBoard!");
	return;
}

void MatchHistory::CloseScoreBoard(string eventName) {
	//cvarManager->log("Entering CloseScoreBoard!");
	looking = false;
	Hide();
	//cvarManager->log("Exiting CloseScoreBoard!");
	return;
}

void MatchHistory::EndGame(string eventName)
{
	//cvarManager->log("Entering EndGame!");
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (!sw.IsNull()) {
		//cvarManager->log("sw is not null!");
		GameSettingPlaylistWrapper playlist = sw.GetPlaylist();
		//cvarManager->log("playlist gotten!");
		if (!playlist.IsLanMatch() && !playlist.IsPrivateMatch()) {
			//cvarManager->log("playlist not lan or private!");
			PlayerControllerWrapper localPrimaryPlayerController = sw.GetLocalPrimaryPlayer();
			if (!localPrimaryPlayerController.IsNull()) {
				//cvarManager->log("playercontroller is not null!");
				PriWrapper localPrimaryPlayer = localPrimaryPlayerController.GetPRI();
				if (!localPrimaryPlayer.IsNull()) {
					//cvarManager->log("localprimaryplayer is not null!");
					TeamWrapper matchWinner = sw.GetMatchWinner();
					ArrayWrapper<TeamWrapper> teams = sw.GetTeams();
					if (!matchWinner.IsNull()) {
						//cvarManager->log("matchwinner is not null!");
						matchValues.clear();

						int myScore = teams.Get(localPrimaryPlayer.GetTeamNum()).GetScore();
						int otherScore;
						if (localPrimaryPlayer.GetTeamNum() == 0) {
							otherScore = teams.Get(1).GetScore();
						}
						else {
							otherScore = teams.Get(0).GetScore();
						}
						matchValues["Win"] = to_string((localPrimaryPlayer.GetTeamNum() == matchWinner.GetTeamNum()));

						if (localPrimaryPlayer.GetTeamNum() == matchWinner.GetTeamNum()) {
							sessionWins++;
						}
						else {
							sessionLosses++;
						}
						int playlistId = playlist.GetPlaylistId();
						matchValues["Playlist"] = playlist.GetTitle().ToString();
						matchValues["MyTeamGoals"] = to_string(myScore);
						matchValues["OtherTeamGoals"] = to_string(otherScore);
						matchValues["Ranked"] = to_string(playlist.GetbRanked());
						UniqueIDWrapper uniqueID;
						uniqueID = gameWrapper->GetUniqueID();
						matchValues["MMR"] = to_string((int)myMMR);
						time_t now = time(NULL);
						tm nowInfo;
						localtime_s(&nowInfo, &now);
						char timestamp[32];
						strftime(timestamp, sizeof(timestamp), "%X %x", &nowInfo);
						matchValues["Timestamp"] = string(timestamp);
						matchValues["Time"] = to_string(now);
						matchValues["MVP"] = to_string(localPrimaryPlayer.GetbMatchMVP());
						matchValues["Points"] = to_string(localPrimaryPlayer.GetMatchScore());
						matchValues["Goals"] = to_string(localPrimaryPlayer.GetMatchGoals());
						matchValues["Assists"] = to_string(localPrimaryPlayer.GetMatchAssists());
						matchValues["Saves"] = to_string(localPrimaryPlayer.GetMatchSaves());
						matchValues["Shots"] = to_string(localPrimaryPlayer.GetMatchShots());
						matchValues["Demos"] = to_string(localPrimaryPlayer.GetMatchDemolishes());
						gameWrapper->SetTimeout([this, uniqueID, playlistId](GameWrapper* gameWrapper) {
							//cvarManager->log("Entering SetTimeout!");
							string filePath = getFilePath();
							if (!ifstream(filePath)) {
								matchesFile.open(filePath, ios_base::app);

								unsigned int idx = 1;
								for (const pair<string, string>& matchValue : matchValues) {
									matchesFile << matchValue.first;

									if (idx < matchValues.size()) {
										matchesFile << ",";
									}

									idx++;
								}
								matchesFile << endl;
								matchesFile.close();
							}
							matchesFile.open(filePath, ios_base::app);
							unsigned int idx = 1;
							for (const pair<string, string>& matchValue : matchValues) {
								matchesFile << matchValue.second;

								if (idx < matchValues.size()) {
									matchesFile << ",";
								}

								idx++;
							}

							matchesFile << endl;

							matchesFile.close();
							getTotalLines(filePath);
							//cvarManager->log("Exiting SetTimeout!");
							}, 6.f);
					}
				}
			}
		}
	}
	//cvarManager->log("Exiting EndGame!");
}

string MatchHistory::getFilePath() {
	//cvarManager->log("Entering getFilePath!");
	UniqueIDWrapper uniqueID;
	uniqueID = gameWrapper->GetUniqueID();
	string filePath = gameWrapper->GetDataFolder().string() + "\\" + uniqueID.str() + "_MatchHistory.csv";
	return filePath;
	//cvarManager->log("Exiting getFilePath!");
}

string MatchHistory::fuzzyTime(unsigned long timeRaw) {
	//cvarManager->log("Entering fuzzyTime!");
	time_t temp = time(NULL);
	unsigned long now = temp;
	unsigned long difference = now - timeRaw;
	string tmp;
	//cvarManager->log(to_string(temp));
	//cvarManager->log(to_string(timeRaw));
	//cvarManager->log(to_string(difference));
	if (difference <= second) {
		tmp = "one second ago";
	}
	else if (difference < minute) {
		tmp = to_string(difference) + " seconds ago";
	}
	else if (difference < 2 * minute) {
		tmp = "a minute ago";
	}
	else if (difference < hour) {
		tmp = to_string(difference / minute) + " minutes ago";
	}
	else if (difference < 2 * hour) {
		tmp = "an hour ago";
	}
	else if (difference < day) {
		tmp = to_string(difference / (hour)) + " hours ago";
	}
	else if (difference < 2 * day) {
		tmp = "yesterday";
	}
	else if (difference < month) {
		tmp = to_string(difference / (day)) + " days ago";
	}
	else if (difference < year) {
		int months = difference / (month);
		if (months <= 1) {
			tmp = "one month ago";
		}
		else {
			tmp = to_string(months) + " months ago";
		}
	}
	else {
		int years = difference / (year);
		if (years <= 1) {
			tmp = "one year ago";
		}
		else {
			tmp = to_string(years) + " years ago";
		}
	}
	//cvarManager->log("Exiting fuzzyTime!");
	return tmp;
}

void MatchHistory::UpdateStats(UniqueIDWrapper id)
{
	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (!sw.IsNull()) {
		//cvarManager->log("sw is not null!");
		GameSettingPlaylistWrapper playlist = sw.GetPlaylist();
		//cvarManager->log("playlist gotten!");
		if (!playlist.IsLanMatch() && !playlist.IsPrivateMatch()) {
			int playlistId = playlist.GetPlaylistId();
			UniqueIDWrapper uniqueID;
			uniqueID = gameWrapper->GetUniqueID();
			myMMR = gameWrapper->GetMMRWrapper().GetPlayerMMR(uniqueID, playlistId);
		}
	}
}

void MatchHistory::onUnload()
{

}
