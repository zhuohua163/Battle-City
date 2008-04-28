#include "CInGame.h"

/***************************************************************
 * Constructor
 *
 **************************************************************/
CInGame::CInGame(CGame *game) {
	this->p = game;
	this->IsBuilding = 0;
	this->Cash = 0;
	this->ShowBuildMenu = 0;

	this->chatLine1.clear();
	this->chatLine2.clear();
	this->chatLine3.clear();
	this->chatLine4.clear();
	this->chatLine5.clear();
	this->chatLine6.clear();
	this->chatLine7.clear();
	this->chatLine8.clear();
	this->ChatLine.clear();

	this->infoLine1.clear();
	this->infoLine2.clear();
	this->infoLine3.clear();
	this->infoLine4.clear();
	this->infoLine5.clear();
	this->infoLine6.clear();
	this->infoLine7.clear();
	this->infoLine8.clear();

	this->IsChatting = 0;
	this->ShowMap = 0;
	this->Successor = 0;
	this->HasLaser = 0;
	this->HasRocket = 0;
	this->HasUpLink = 0;
	this->updateTick = 0;
	this->lastTick = 0;
	this->hospTick = 0;
	this->TCPPing = 0;
	memset(this->TCPPingValues, 0, sizeof(this->TCPPingValues));
	this->PingTick = 0;
	this->PingTimer = 0;
	this->Minimized = 0;
	this->timeDeath = 0;
	this->timeLastAttack = 0;

	for (int i = 0; i < 27; i++) {
		this->CanBuild[i] = 0;
	}

	memset(this->HasSector, 0, sizeof(this->HasSector));
	memset(this->RequestedSector, 0, sizeof(this->RequestedSector));

}

/***************************************************************
 * Destructor
 *
 **************************************************************/
CInGame::~CInGame() {
}

/***************************************************************
 * Function:	Cycle
 *
 **************************************************************/
void CInGame::Cycle() {
	float curTick = this->p->Tick;

	// For each possible player,
	for (int j = 0; j < MAX_PLAYERS; j++) {

		// Run the Player cycle
		this->p->Player[j]->Cycle();
	}

	// Run the Bullet cycle
	this->p->Bullet->Cycle();
	this->p->Explode->Cycle();
	this->p->Item->Cycle();
	this->p->Build->Cycle();
	this->p->Inventory->Cycle();

	// Draw the game
	this->p->Draw->DrawGame();

	// If the player update timer is up, and this player is still alive,
	if ( (curTick > updateTick) && (this->p->Player[this->p->Winsock->MyIndex]->isDead == false)) {
		sCMUpdate packet;
		int me = this->p->Winsock->MyIndex;
		this->p->Player[me]->SectorX = (char)((this->p->Player[me]->X/48) / SectorSize);
		this->p->Player[me]->SectorY = (char)((this->p->Player[me]->Y/48) / SectorSize);
		packet.x = (int)this->p->Player[me]->X;
		packet.y = (int)this->p->Player[me]->Y;
		packet.dir = (unsigned char)this->p->Player[me]->Direction;
		packet.move = (unsigned char)(this->p->Player[me]->isMoving + 1);
		packet.turn = (unsigned char)(this->p->Player[me]->isTurning + 1);
		this->p->Winsock->SendData(cmUpdate, (char *)&packet, sizeof(packet));
		this->updateTick = curTick + 150;
	}

	// If the Ping timer is up,
	if (curTick > PingTick) {
		this->CheckRefVariables();
		this->p->Winsock->SendData(cmTCPPing, " ");
		this->PingTimer = p->Tick;
		this->PingTick = curTick + 5000;
	}

	// If the player is under attack,
	if (this->isUnderAttack) {

		// If the Under Attack timer is up,
		if (this->p->Tick > this->timeUnderAttack) {

			// Turn off Under Attack
			this->setIsUnderAttack(false);
		}
	}

	this->lastTick = this->p->Tick;
}

/***************************************************************
 * Function:	ReturnRank
 *
 * @param Points
 **************************************************************/
string CInGame::ReturnRank(int Points) {
	string rank;

	if (Points < 100) {
		rank = "Private";
	}
	else if (Points < 200) {
		rank = "Corporal";
	}
	else if (Points < 500) {
		rank = "Sergeant";
	}
	else if (Points < 1000) {
		rank = "Sergeant Major";
	}
	else if (Points < 2000) {
		rank = "Lieutenant";
	}
	else if (Points < 4000) {
		rank = "Captain";
	}
	else if (Points < 8000) {
		rank = "Major";
	}
	else if (Points < 16000) {
		rank = "Colonel";
	}
	else if (Points < 30000) {
		rank = "Brigadier";
	}
	else if (Points < 45000) {
		rank = "General";
	}
	else if (Points < 60000) {
		rank = "Baron";
	}
	else if (Points < 80000) {
		rank = "Earl";
	}
	else if (Points < 100000) {
		rank = "Count";
	}
	else if (Points < 125000) {
		rank = "Duke";
	}
	else if (Points < 150000) {
		rank = "Archduke";
	}
	else if (Points < 200000) {
		rank = "Grand Duke";
	}
	else if (Points < 250000) {
		rank = "Lord";
	}
	else if (Points < 300000) {
		rank = "Chancellor";
	}
	else if (Points < 350000) {
		rank = "Royaume";
	}
	else if (Points < 400000) {
		rank = "Emperor";
	}
	else if (Points < 500000) {
		rank = "Auror";
	}
	else {
		rank = "King";
	}

	return rank;
}

/***************************************************************
 * Function:	PrintWhoData
 *
 **************************************************************/
void CInGame::PrintWhoData() {
		string Players;
		int PlayerCount = 0;
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (p->Player[i]->Name.length() > 0)
			{
				PlayerCount++;
				if (Players.length() == 0)
					Players += p->Player[i]->Name;
				else
				{
					Players += ", ";
					Players += p->Player[i]->Name;
				}

			}
		}

		ostringstream Converter;
		Converter << "There ";
		Converter << (PlayerCount == 1 ? "is " : "are ");
		Converter << PlayerCount;
		Converter << (PlayerCount == 1 ? " player" : " players");
		Converter << " currently online:  ";
		Converter << Players;
		this->AppendInfo(Converter.str(), COLOR_SYSTEM);
}

/***************************************************************
 * Function:	AppendChat
 *
 * @param chatText
 * @param color
 **************************************************************/
void CInGame::AppendChat(string chatText, COLORREF color) {
	unsigned int MessageLength = this->p->Draw->chatBarWidth;
	while (chatText.length() > MessageLength) {
		AppendLine(chatText.substr(0, MessageLength), color);
		chatText = chatText.substr(MessageLength, chatText.length());
	}

	AppendLine(chatText, color);
}

/***************************************************************
 * Function:	AppendInfo
 *
 * @param chatText
 * @param color
 **************************************************************/
void CInGame::AppendInfo(string chatText, COLORREF color) {
	unsigned int MessageLength = this->p->Draw->chatBarWidth;
	while (chatText.length() > MessageLength) {
		AppendInfoLine(chatText.substr(0, MessageLength), color);
		chatText = chatText.substr(MessageLength, chatText.length());
	}

	AppendInfoLine(chatText, color);
}

/***************************************************************
 * Function:	AppendLine
 *
 * @param chatText
 * @param color
 **************************************************************/
void CInGame::AppendLine(string chatText, COLORREF color) {
	chatLine1 = chatLine2;
	chatLine2 = chatLine3;
	chatLine3 = chatLine4;
	chatLine4 = chatLine5;
	chatLine5 = chatLine6;
	chatLine6 = chatLine7;
	chatLine7 = chatLine8;
	chatLine8 = chatText;
	chatColor1 = chatColor2;
	chatColor2 = chatColor3;
	chatColor3 = chatColor4;
	chatColor4 = chatColor5;
	chatColor5 = chatColor6;
	chatColor6 = chatColor7;
	chatColor7 = chatColor8;
	chatColor8 = color;	
}

/***************************************************************
 * Function:	AppendInfoLine
 *
 * @param infoText
 * @param color
 **************************************************************/
void CInGame::AppendInfoLine(string infoText, COLORREF color) {
	infoLine1 = infoLine2;
	infoLine2 = infoLine3;
	infoLine3 = infoLine4;
	infoLine4 = infoLine5;
	infoLine5 = infoLine6;
	infoLine6 = infoLine7;
	infoLine7 = infoLine8;
	infoLine8 = infoText;
	infoColor1 = infoColor2;
	infoColor2 = infoColor3;
	infoColor3 = infoColor4;
	infoColor4 = infoColor5;
	infoColor5 = infoColor6;
	infoColor6 = infoColor7;	
	infoColor7 = infoColor8;
	infoColor8 = color;	
}

/***************************************************************
 * Function:	ClearOut
 *
 **************************************************************/
void CInGame::ClearOut() {
	CBuilding *bld;

	p->Draw->ClearDrawing();

	lastTick = 0;
	updateTick = 0;

	ShowBuildMenu = 0;
	Successor = 0;
	p->Personnel->DenyApplicants = 0;
	IsBuilding = 0;
	Cash = 0;

	HasLaser = 0;
	HasRocket = 0;

	timeDeath = 0;
	timeLastAttack = 0;

	chatLine1.clear();
	chatLine2.clear();
	chatLine3.clear();
	chatLine4.clear();
	chatLine5.clear();
	ChatLine.clear();

	IsChatting = 0;
	BombsAreActivated = 0;

	p->Personnel->DenyApplicants = 0;

	for (int i = 0; i < 27; i++) {
		CanBuild[i] = 0;
	}

	// For each building,
	bld = this->p->Build->buildingListHead;
	while (bld) {

		// If the city is not a CC, delete it (which gets the next building)
		if (bld->isCC() == false) {
			bld = this->p->Build->delBuilding(bld);
		}

		// Else (didn't delete), get the next building
		else {
			bld = bld->next;
		}
	}

	// For each item,
	while (this->p->Item->itemListHead) {

		// Delete the item
		this->p->Item->delItem(this->p->Item->itemListHead);
	}

	// For each explosion,
	while (this->p->Explode->explosionListHead) {

		// Delete the explosion
		this->p->Explode->delExplosion(this->p->Explode->explosionListHead);
	}

	// For each bullet,
	while (this->p->Bullet->bulletListHead) {

		// Delete the bullet
		this->p->Bullet->delBullet(this->p->Bullet->bulletListHead);
	}

	// For each inventory item,
	while (this->p->Inventory->itemListHead) {

		// Delete the item
		this->p->Inventory->delItem(this->p->Inventory->itemListHead);
	}
	p->Inventory->SelectedItemType = 0;

	// For each possible player,
	for (int j = 0; j < MAX_PLAYERS; j++) {

		// Clear the player from the game
		p->Player[j]->InGameClear();
	}

	// Clear all sectors
	p->InGame->ClearAllSectors();
}

/***************************************************************
 * Function:	CheckRefVariables
 *
 **************************************************************/
void CInGame::CheckRefVariables() {
	int me = p->Winsock->MyIndex;
	int HP = p->Player[me]->HP;
	int HP1 = (HP^2-31)*2;
	int HP2 = -(HP);

	if (p->Player[me]->refHP1 != HP1) {
		p->Winsock->SendData(cmBan, "refHP1gdmde");
		p->Process->ProcessKicked();
		return;
	}

	if (p->Player[me]->refHP2 != HP2) {
		p->Winsock->SendData(cmBan, "refHP2gdmde");
		p->Process->ProcessKicked();
		return;
	}

	CheckCheatTools();
}

/***************************************************************
 * Function:	CheckCheatTools
 *
 **************************************************************/
void CInGame::CheckCheatTools() {
	HWND wnd = 0;
	wnd = FindWindow(0 , "WPE PRO");
	if (wnd > 0) {
		p->Winsock->SendData(cmBan, "wsckpkedtrpro");
		p->Process->ProcessKicked();
	}

	wnd = FindWindow(0 , "TSearch");
	if (wnd > 0) {
		p->Winsock->SendData(cmBan, "tsch");
		p->Process->ProcessKicked();
	}
}

/***************************************************************
 * Function:	RefreshArea
 *
 **************************************************************/
void CInGame::RefreshArea() {
	bool requestedSector = false;
	char me = p->Winsock->MyIndex;
	sCMSector sector;

	// For each sector in the nearby grid of 9 sectors,
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {

			// If the sector is on the map,
			if (
				((p->Player[me]->SectorX+i) >= 0)
				&&
				((p->Player[me]->SectorX+i) <= MaxSectors) 
				&&
				((p->Player[me]->SectorY+j) >= 0)
				&&
				((p->Player[me]->SectorY+j) <= MaxSectors)
			) {

				// If the player doesn't have the sector, and hasn't already requested it,
				if (
					(! this->HasSector[(p->Player[me]->SectorX+i)][(p->Player[me]->SectorY+j)])
					&&
					(! this->RequestedSector[(p->Player[me]->SectorX+i)][(p->Player[me]->SectorY+j)])
				) {

					// Get the bounds of the sector
					sector.x = p->Player[me]->SectorX + i;
					sector.y = p->Player[me]->SectorY + j;

					ClearSector(sector.x, sector.y);
					this->RequestedSector[sector.x][sector.y] = true;
					p->Winsock->SendData(cmRequestSector, (char *)&sector, sizeof(sector));
					requestedSector = true;
				}
			}
		}
	}

	// For each sector,
	for (int i = 0; i <= MaxSectors; i++) {
		for (int j = 0; j <= MaxSectors; j++) {

			// If the player has the sector,	
			if( this->HasSector[i][j] ) {

				// If the player is no longer near the sector, clear it
				if ((abs(i - p->Player[me]->SectorX) > 1) | (abs(j - p->Player[me]->SectorY) > 1)) {
					ClearSector(i, j);
				}
			}
		}
	}

	// If we requested a sector, and the minimap is open, 
	if ((requestedSector==true) && (p->InGame->ShowMap == 1)) {

		// Request the map
		p->Winsock->SendData(cmMiniMap, " ");
	}
}

/***************************************************************
 * Function:	ClearSector
 *
 * @param SectorX the X position of the sector
 * @param SectorY the Y position of the sector
 **************************************************************/
void CInGame::ClearSector(int SectorX, int SectorY) {
	CBuilding *bld;
	CItem *itm;

	// If the sector is on the map, and the minimap is not open,
	if (SectorX >= 0 && SectorY >= 0 && SectorX <= MaxSectors && SectorY <= MaxSectors && p->InGame->ShowMap == 0) {
		p->InGame->HasSector[SectorX][SectorY] = 0;
		p->InGame->RequestedSector[SectorX][SectorY] = false;

		// For each item,
		itm = p->Item->itemListHead;
		while (itm) {

			// If the item is in the sector,
			if ((itm->X / SectorSize) == SectorX && (itm->Y / SectorSize) == SectorY) {

				// Delete the item (from the client)
				itm = p->Item->delItem(itm);
			}

			// Else (item not in the sector), get the next item
			else {
				itm = itm->next;
			}
		}

		// For each building,
		bld = p->Build->buildingListHead;
		while (bld) {

			// If the building is in the sector (and not a CC),
			if ((bld->X / SectorSize) == SectorX && (bld->Y / SectorSize) == SectorY && bld->Type != 6) {

				// Delete the item (from the client)
				bld = p->Build->delBuilding(bld);
			}

			// Else (building is not in the sector),
			else {

				// Get the next building
				bld = bld->next;
			}
		}
	}
}

/***************************************************************
 * Function:	ClearAllSectors
 *
 **************************************************************/
void CInGame::ClearAllSectors() {
	CBuilding *bld;
	CItem *itm;

	memset(HasSector, 0, sizeof(HasSector));
	memset(RequestedSector, 0, sizeof(RequestedSector));

	// For each item,
	itm = p->Item->itemListHead;
	while (itm) {

		// Delete the item (from the client)
		itm = p->Item->delItem(itm);
	}

	// For each building,
	bld = p->Build->buildingListHead;
	while (bld) {

		// If the is not a CC,
		if (bld->isCC() == false) {

			// Delete the building (from the client)
			bld = p->Build->delBuilding(bld);
		}

		// Else (building is not in the sector),
		else {

			// Get the next building
			bld = bld->next;
		}
	}
}

/***************************************************************
 * Function:	PrintFinanceReport
 *
 **************************************************************/
void CInGame::PrintFinanceReport() {
	long netIncome = this->getGrossIncome();

	p->Draw->ClearPanel();

	p->Draw->PanelMode = modeFinance;
	p->Draw->PanelLine1 = "City Finance Report:";
	
	p->Draw->PanelLine2 = "Income: ";
	p->Draw->PanelLine2 += p->Draw->CashFormat(p->InGame->Income);

	p->Draw->PanelLine3 = "Research: -";
	p->Draw->PanelLine3 += p->Draw->CashFormat(p->InGame->Research);

	p->Draw->PanelLine4 = "Items: -";
	p->Draw->PanelLine4 += p->Draw->CashFormat(p->InGame->Items);

	p->Draw->PanelLine5 = "Hospitals: -";
	p->Draw->PanelLine5 += p->Draw->CashFormat(p->InGame->Hospital);

	p->Draw->PanelLine6 = "Total: ";
	if (netIncome < 0) {
		p->Draw->PanelLine6 += "-";
	}
	p->Draw->PanelLine6 += p->Draw->CashFormat(abs(netIncome));
}

/***************************************************************
 * Function:	getGrossIncome
 *
 **************************************************************/
int CInGame::getGrossIncome() {
	return this->Income - this->Hospital - this->Items - this->Research;
}

/***************************************************************
 * Function:	setIsUnderAttack
 *
 * @param isUnderAttack
 **************************************************************/
void CInGame::setIsUnderAttack(bool isUnderAttack) {

	// If isUnderAttack is true, set isUnderAttack true and timeUnderAttack
	if (isUnderAttack) {
		this->isUnderAttack = true;
		this->timeUnderAttack = this->p->Tick + TIMER_UNDER_ATTACK;
	}

	// Else (isUnderAttack is false), set isUnderAttack false and timeUnderAttack 0
	else {
		this->isUnderAttack = false;
		this->timeUnderAttack = 0;
	}

}

/***************************************************************
 * Function:	requestAutoBuild
 *
 * @param chatLine
 **************************************************************/
void CInGame::requestAutoBuild(string chatLine) {
	string fileName = this->getFileNameFromChatLine(chatLine);
	CPlayer* player = this->p->Player[this->p->Winsock->MyIndex];
	int* buildingInfo;
	sCMAutoBuild request;
	
	// If there is no filename, return
	if (fileName.length() == 0) {
		this->AppendInfo("Please specify a city design name!", COLOR_SYSTEM);
		return;
	}

	// If the player is dead, return
	if (player->isDead) {
		this->AppendInfo("You must be alive to load a city design!", COLOR_SYSTEM);
		return;
	}

	// If the player is not mayor, return
	if (player->isMayor == false) {
		this->AppendInfo("You must be mayor to load a city design!", COLOR_SYSTEM);
		return;
	}

	strcpy(request.filename, fileName.c_str());
	this->p->Winsock->SendData(cmAutoBuild,(char *)&request,sizeof(request));
}

/***************************************************************
 * Function:	saveCity
 *
 * @param chatLine
 **************************************************************/
void CInGame::saveCity(string chatLine) {
	CBuilding *bld = this->p->Build->buildingListHead;
	stringstream ss;
	string fileName = this->getFileNameFromChatLine(chatLine);
	string folderName;
	int city = this->p->Player[this->p->Winsock->MyIndex]->City;
	CPlayer* player = this->p->Player[this->p->Winsock->MyIndex];
	string cityName = CityList[city];
	FILE *cityFile;

	// If not mayor, return
	if (player->isMayor == false) {
		this->AppendInfo("You must be mayor to save the city design!", COLOR_SYSTEM);
		return;
	}
	
	// If there is no filename, return
	if (fileName.length() == 0) {
		this->AppendInfo("Please specify a city design name!", COLOR_SYSTEM);
		return;
	}

	// Create the directory (does nothing if directory already exists)
	folderName = FILE_CITIES_FOLDER;
	CreateDirectory(folderName.c_str(), NULL);
	folderName = folderName + "/" + cityName;
	CreateDirectory(folderName.c_str(), NULL);

	// Get the filename from the ChatLine
	fileName = folderName + "/" + fileName + FILE_CITIES_EXTENSION;

	// Open the file for writing
	cityFile = fopen(fileName.c_str(),"w+");

	// If the file couldn't be opened, error
	if (!cityFile) {
		this->AppendInfo("Unable to open the file \"" + fileName + "\"!", COLOR_SYSTEM);
		return;
	}

	// For each building in the linked list,
	while (bld) {

		// If the building belongs to this city, and building isn't a CC,
		if ((bld->City == city) && (bld->isCC() == false)) {

			// Write the building's info to the file
			ss.str("");
			ss << bld->rawType << " " << bld->X << " " << bld->Y << endl;
			fwrite(ss.str().c_str(), ss.str().size(), 1, cityFile); 
		}

		// Move the pointer to the next building in the linked list
		bld = bld->next;
	}

	// Close the file
	fclose(cityFile);
	this->AppendInfo("Saved as: \"" + fileName + "\"!", COLOR_SYSTEM);
}

/***************************************************************
 * Function:	getFileNameFromChatLine
 *
 * @param chatLine
 **************************************************************/
string CInGame::getFileNameFromChatLine(string chatLine) {
	string fileName;
	int fileNameStartIndex = chatLine.find(" ",0) + 1;
	int fileNameEndIndex = chatLine.find(" ",fileNameStartIndex) + 1;

	// If there was no space, set filenameEndIndex to the end of the chatline
	if (fileNameEndIndex == 0) {
		fileNameEndIndex = chatLine.length();
	}

	// If there is no filename, return
	if (fileNameStartIndex == fileNameEndIndex) {
		return "";
	}

	// Get the filename from the chatLine
	return chatLine.substr(fileNameStartIndex, fileNameEndIndex-fileNameStartIndex);
}

/***************************************************************
 * Function:	splitStringIntoInts
 *
 * @param chatLine
 **************************************************************/
int* CInGame::splitStringIntoInts(char* buildingLine) {
	int counter = 0;
	int *buildingInts = new int[3];
	string buf;

	// Insert the string into a stream
	stringstream buildingStream(buildingLine); 

	// Add each "word" in the string (separated by whitespace) to th
	while (buildingStream >> buf) {
		buildingInts[counter] = atoi(buf.c_str());
		counter++;
	}

	return buildingInts;
}

/***************************************************************
 * Function:	createBuilding
 *
 * @param type
 * @param x
 * @param y
 * @param isAutoBuild
 **************************************************************/
bool CInGame::createBuilding(unsigned char type, unsigned short x, unsigned short y, bool isAutoBuild) {
	sCMBuild buildingPacket;
	int collision;

	// If the player is not an admin, check for conditions...
	if (! this->p->Player[this->p->Winsock->MyIndex]->isAdmin()) {

		// If the city can't afford a building, return
		if (this->Cash < COST_BUILDING) {
			return false;
		}

		// If there is a collision, return
		collision = this->p->Collision->CheckBuildingCollision(x, y);
		if (collision) {
			return false;
		}
		
		// If the building is not in the city's build range, return
		if (! this->p->Build->inRange(false)) {
			return false;
		}

		// If the building is not on the map, return
		if ((x < 0) || (x > 510) || (y < 0) || (y > 510)) {
			return false;
		}
	}

	buildingPacket.type = type;
	buildingPacket.x = x;
	buildingPacket.y = y;
	buildingPacket.isAutoBuild = isAutoBuild;
	this->p->Winsock->SendData(cmBuild,(char *)&buildingPacket,sizeof(buildingPacket));

	return true;
}