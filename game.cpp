#include "game.h"

using namespace std;

Game::Game()
{
	//	konstruktor klasy silnika gry

	initwindow(1024, 710);

	saveSlots = NULL;
	mazeGenerator = NULL;
	settings.iPlayerLevel = PLAYER_LEVEL_BEGINNER;
}

Game::~Game()
{
	//	destruktor klasy silnika gry

	closegraph();


	delete [] saveSlots;
}

void Game::InitGame()
{
	/*
		Funkcja ta inicjalizuje okno gry. Tworzy glowne menu oraz zajmuje sie
        przetwarzaniem akcji wykonywanych przez uzytkownika w petli
    */

    if (GetCurrentBPP() < 32)   //  weryfikacja czy ekran pracuje w 32 bpp
    {
        MessageBox(0, "Ta gra wymaga g³êbi kolorów 32 bit na piksel.\nProgram teraz zakoñczy swoje dzia³anie.",
                   "B³¹d", MB_ICONERROR);
        return;
    }

    if (InitGameSaves() == false)   //  czy udalo sie zaincjalizowac system zapisu gry ?
    {
		MessageBox(0, "Nie uda³o siê za³adowac zapisanych stanów gry. Upewnij siê ¿e masz uprawnienia do zapisu / odczytu w folderze gry/Images.",
                    "B³¹d", MB_ICONERROR);
		return;
	}

	if (LoadGameSettings() == false)
	{
	    MessageBox(0, "Nie uda³o siê za³adowaæ ustawieñ gry. Upewnij siê ¿e masz uprawnienia do zapisu / odczytu w folderze z plikiem EXE.",
                   "B³¹d", MB_ICONERROR);
        return;
	}

    char szExeDir[1024];
    GetExeDirectory(szExeDir);

    if (LoadGameBitmaps() == false) //  ladujemy obrazki uzywane w grze
    {
        MessageBox(0, "Nie uda³o siê za³adowac wszystkich bitmap wymaganych do dzia³ania gry.",
                   "B³¹d", MB_ICONERROR);
        return;
    }

    unsigned long ulLastMsec = GetTickCount();
	int iSleepTime = 100;   //  interwal czasu, co ile wykonuje sie petla. Zmienia sie w trakcie gry w zaleznosci od wymagan dokladnosci operacji
	int iKey, iCurrentSlotSelected;     //  aktualnie wcisniety klawisz, aktualnie wybrany slot gry
    int iCurrentMenuLevel = MAIN_MENU;  //  okresla na jakim poziomie menu znajdujemy sie, defaultowo jest to menu glowne programu
    int iCurrentMenuOption = MENU_OPTION_BEGIN; //  okresla ktora opcje menu wybrano, domyslnie zaznaczona jest opcja "nowa gra"
	bool bContinueGame = true;  //  czy kontynuowac gre
    bool bReinitMenu = true;    //  czy menu glowne trzeba odrysowac ? zmniejsza efekt migania
    bRefreshStartTicks = false; //  czy gra byla pauzowana (zapis gry), jesli byla to trzeba zaktualizowac czas rozpoczecia gry
    bStopTitle = false;
    int ret;

    iKey = CheckKeyboardState();

	while (bContinueGame == true)
    {
        if (GetTickCount() - ulLastMsec > 200)
        {
            iKey = CheckKeyboardState();
            ulLastMsec = GetTickCount();
        }
        else iKey = - 1;

        switch (iCurrentMenuLevel)
        {
			case GAME_IN_PROGRESS:	//	aktualnie toczy sie gra
                if (bGameFinished == true)
                {
                    //  uzytkownik wszedl na pole oznaczone jako wyjscie, konczymy ta gre
                    iCurrentMenuLevel = GAME_FINISHED_PAGE;
                    continue;
                }
				if (iKey == VK_ESCAPE)
                {
                    //  wcisnieto ESC, wychodzimy z gry do glownego menu
                    bRefreshStartTicks = true;
                    ulMsecsAtSaveGame = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;
                    bStopTitle = true;
                    iKey = 0;
                    ret = MessageBox(0, "Czy na pewno chcesz przerwac grê ?", "Pytanie",
                                     MB_ICONQUESTION | MB_YESNO);
                    switch (ret)
                    {
                        case IDYES:
                            iKey = - 1;
                            bReinitMenu = true;
                            iCurrentMenuLevel = MAIN_MENU;
                            break;
                        case IDNO:
                            iKey = - 1;
                            RefreshGame();
                            break;
                    }
                    bStopTitle = false;
                    continue;
                }
                if (iKey == VK_DOWN || iKey == VK_UP || iKey == VK_LEFT || iKey == VK_RIGHT)
                {
                    //  gracz chce sie przemiescic
                    iPlayerMoveDirection = iKey;
                }
                if (iKey == VK_F1)
                {
                    //  w czasie gry ktos chce zajrzec w ekran pomocy
                    bRefreshStartTicks = true;
                    ulGameTimeMsecsIncrement += GetTickCount()  - ulTicksAtGameStart;
                    ulMsecsAtSaveGame = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;
                    bMazeNeedUpdate = true;
                    bHelpReturnToGame = true;
                    iCurrentMenuLevel = HELP_PAGE;
                    ShowHelpScreen();
                    continue;
                }
                if (iKey == VK_F5)
                {
                    //  gracz chce przejsc do ekranu z zapisami stanu gry
                    bRefreshStartTicks = true;
                    ulGameTimeMsecsIncrement += GetTickCount()  - ulTicksAtGameStart;
                    ulMsecsAtSaveGame = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;
                    iCurrentMenuLevel = SAVE_GAME_PAGE;
                    ShowGameSaveSlotsMenu(&iCurrentSlotSelected);
                    continue;
                }
                RefreshGame();  //  odswiezamy ekran toczacej sie gry
                iKey = 0;
				break;
         	case MAIN_MENU:			//	jestesmy w glownym menu gry (zaraz po uruchomieniu)
            	iSleepTime = 100;
            	iCurrentSlotSelected = 0;
                if (iKey == VK_ESCAPE)
                {
                    switch (MessageBox(0, "Czy chcesz zakoñczyc grê ?", "Pytanie", MB_ICONQUESTION | MB_YESNO))
                    {
                        case IDYES:
                            bContinueGame = false;	//	konczymy program, wcisnieto ESC
                            iKey = 0;
                            break;
                        case IDNO:
                            iKey = 0;
                            break;
                    }
                }
                //	jesli nie wcisnieto ESC, to pokazujemy glowne menu gry
                if (bReinitMenu == true)
                {
                    //  jesli trzeba wyswietlic na nowo menu, to robimy to
                    cleardevice();
	                ShowMainMenu(&iCurrentMenuOption, false, true);
	                bReinitMenu = false;
                }
				if (iKey == VK_UP || iKey == VK_DOWN)
                    //  gracz porusza sie w menu programu
	                ShowMainMenu(&iCurrentMenuOption, iKey != VK_DOWN && iKey == VK_UP, false);
                if (iKey == VK_F1)
                {
                    //  ktos chce zobaczyc okno pomocy
                    iCurrentMenuLevel = HELP_PAGE;
                    bHelpReturnToGame = false;
                    ShowHelpScreen();
                    continue;
                }
                if (iKey == VK_RETURN)
                {
                 	//	wcisnieto ENTER, okreslamy co wybral uzytkownik z menu
					if (iCurrentMenuOption == MENU_OPTION_QUIT)
                    {
                    	//	konczymy gre bo uzytkownik wybral w menu opcje wyjscia
                    	bContinueGame = false;
                        break;
                    }
                    else if (iCurrentMenuOption == MENU_OPTION_BEGIN)
                    {
                     	//	przechodzimy do tworzenia planszy
                     	bGameFinished = false;
                     	iSleepTime = 10;    //  petla bedzie sie teraz wykonywala 10 razy czesciej na rzecz planszy
                    	iCurrentMenuLevel = GAME_IN_PROGRESS;
                    	iGameTitleX = 200;
                    	ulTicksAtGameStart = GetTickCount();
                    	ulGameTimeMsecsIncrement = 0;
                        StartGame();
                    }
                    else if (iCurrentMenuOption == MENU_OPTION_HELP)
                    {
                        //  uzytkownik chce zobaczyc okno pomocy
                        iCurrentMenuLevel = HELP_PAGE;
                        bHelpReturnToGame = false;
                        ShowHelpScreen();
                    }
                    else if (iCurrentMenuOption == MENU_OPTION_LOAD_GAME)
                    {
                        //  ladowanie gry - ekran
                        bRefreshStartTicks = false;   //  wstrzymujemy obliczanie czasu gry
                        iCurrentSlotSelected = SAVE_SLOT_1;
                        ShowGameSlotsMenu(&iCurrentSlotSelected);
                        iCurrentMenuLevel = LOAD_GAME_PAGE;
                        continue;
                    }
                    else if (iCurrentMenuOption == MENU_OPTION_PLAYER_LVL)
                    {
                        //  plansza z ustawieniami poziomu trudnosci labiryntow
                        iCurrentMenuLevel = PLAYER_LEVEL_PAGE;
                        ShowPlayerLevelScreen();
                        continue;
                    }
                }
            	break;
            case HELP_PAGE:		//	jestesmy w sekcji pomocy
                if (iKey == VK_SPACE)
                {
                    //  nacisnieto spacje, wracamy do glownego menu
                    cleardevice();
                    if (bHelpReturnToGame == true)
                    {
                        //  pomoc byla wywolana z poziomu planszy - czyli przywracamy
                        iCurrentMenuLevel = GAME_IN_PROGRESS;
                        RefreshGame();
                        bHelpReturnToGame = false;
                        continue;
                    }
                    iCurrentMenuLevel = MAIN_MENU;
                    bReinitMenu = true;
                    continue;
                }
            	break;
            case LOAD_GAME_PAGE:	//	jestesmy w sekcji ladowania gry
                if (iKey == VK_ESCAPE)
                {
                    //  nacisnieto spacje, wracamy do glownego menu
                    iCurrentMenuLevel = MAIN_MENU;
                    bReinitMenu = true;
                    continue;
                }
                else if (iKey == VK_UP || iKey == VK_DOWN)
                {
                    //  uzytkownik porusza sie po slotach zapisanych gier
                    CheckGameSlotForActivation(&iCurrentSlotSelected, iKey != VK_DOWN && iKey == VK_UP, false);
                }
                else if (iKey == VK_RETURN)
                {
                    //  nacisnieto ENTER - uzytkownik wybral slot
                    if (GameSlotCanBeLoaded(iCurrentSlotSelected) == false)
                    {
                        //  slot nie moze zostac zaladowany poniewaz nie zawiera zapisanej gry (nieuzywany)
                        settextstyle(8, HORIZ_DIR, 3);
                        setcolor(RED);
                        outtextxy(100, 500, "Wybrany slot nie zawiera zapisanej gry.");
                    }
                    else
                    {
                        //  przywracamy gre ze slotu o podanym indeksie
                        bGameFinished = false;
                        bRefreshStartTicks = true;
                        iSleepTime = 10;
                        iGameTitleX = 200;
                        RestoreGameState(iCurrentSlotSelected);
                        iCurrentMenuLevel = GAME_IN_PROGRESS;
                        continue;
                    }
                }
            	break;
            case SAVE_GAME_PAGE:        //  ekran zapisywania aktualnie trwajacej gry
                if (iKey == VK_ESCAPE)
                {
                    //  gracz chce wrocic do gry, anulowal zapisywanie
                    bRefreshStartTicks = true;
                    iCurrentMenuLevel = GAME_IN_PROGRESS;
                    bMazeNeedUpdate = true;
                    bPlayerNeedUpdate = true;
                    cleardevice();
                    RefreshGame();
                    continue;
                }
                else if (iKey == VK_DOWN || iKey == VK_UP)
                {
                    //  gracz porusza sie po liscie slotow
                    CheckGameSlotForActivation(&iCurrentSlotSelected, iKey != VK_DOWN && iKey == VK_UP, false);
                }
                else if (iKey == VK_RETURN)
                {
                    //  gracz wybral gre do zapisu i nacisnal enter
                    SaveGameState(iCurrentSlotSelected);
                    //  zapisano gre, powracamy do planszy
                    bRefreshStartTicks = true;
                    iCurrentMenuLevel = GAME_IN_PROGRESS;
                    bMazeNeedUpdate = true;
                    bPlayerNeedUpdate = true;
                    cleardevice();
                    continue;
                }
                break;
            case GAME_FINISHED_PAGE:    //  plansza z informacjami na temat wlasnie zakonczonej gry
                iSleepTime = 100;
                if (iKey == VK_SPACE)
                {
                    iCurrentMenuLevel = MAIN_MENU;
                    bReinitMenu = true;
                    continue;
                }
                break;
            case PLAYER_LEVEL_PAGE:    //  plansza z ustawieniami poziomu trudnosci gry
                if (iKey == VK_SPACE)
                {
                    iCurrentMenuLevel = MAIN_MENU;
                    bReinitMenu = true;
                    continue;
                }
                else if (iKey == VK_UP || iKey == VK_DOWN)
                {
                    //  gracz porusza sie w menu programu
	                ShowPlayerLevelMenu(&settings.iPlayerLevel, iKey != VK_DOWN && iKey == VK_UP, false);
                }
                break;
        }

        Sleep(iSleepTime);
    }

    if (UpdateGameSaves() == false) //  zapisujemy fizycznie do pliku sloty gier
    {
        MessageBox(0, "Nie uda³o siê zapisac slotów stanu gry.", "B³¹d",
                   MB_ICONERROR);
    }

    if (SaveGameSettings() == false)
    {
        MessageBox(0, "Nie uda³o siê zapisac ustawieñ gry. Upewnij siê ¿e masz uprawnienia do zapisu / odczytu w folderze z plikiem EXE.",
                   "B³¹d", MB_ICONERROR);
    }
}

void Game::ShowHelpScreen()
{
    //  pokazuje ekran pomocy

    cleardevice();

    settextstyle(8, HORIZ_DIR, 4);
    setcolor(BLUE);
	outtextxy(350, 100, "Labirynt 2012 - Pomoc");

	setcolor(WHITE);
	settextstyle(8, HORIZ_DIR, 3);
	outtextxy(400, 200, "Zasady gry :");
	outtextxy(100, 230, "Poruszaj siê w labiryncie u¿ywaj¹c klawiszy strza³ek.");
	outtextxy(100, 260, "Staraj siê przejœc do koñca labiryntu w mo¿liwie");
	outtextxy(100, 290, "najkrótszym czasie.");
	outtextxy(100, 320, "Aby zapisac grê, naciœnij w trakcie jej trwania klawisz F5.");
	setcolor(GREEN);
	outtextxy(100, 400, "Naciciœnij spacjê aby kontynuowac.");
}

void Game::ShowGameFinishedScreen()
{
    //  pokazuje ekran z informacjami o tym ze gra zostala wygrana

    cleardevice();

    settextstyle(8, HORIZ_DIR, 4);
    outtextxy(350, 50, "Wygra³eœ planszê !");

    settextstyle(8, HORIZ_DIR, 3);
    outtextxy(300, 200, "Gratulacje, w³aœnie ukoñczy³eœ planszê.");
    outtextxy(300, 240, "Twój czas to : ");


    unsigned long ulDiff = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;
    int iMins = ulDiff / 60000;
    int iSecs = (ulDiff % 60000) / 1000;
    int iMsecs = (ulDiff % 60000) % 100;

    stringstream str;
    str << iMins << ":" << iSecs << ":" << iMsecs;
    // TODO Fix non const char in library
    char* buf = new char[str.str().length() + 1];
    strncpy(buf, str.str().c_str(), str.str().length());
    outtextxy(550, 240, buf);
    delete [] buf;

    outtextxy(300, 500, "Aby kontynuowac naciœnij spacjê.");
}

void Game::ShowPlayerLevelScreen()
{
    //  pokazuje ekran z mozliwoscia ustawienia poziomu trudnosci generowanych
    //  labiryntow

    cleardevice();

    settextstyle(8, HORIZ_DIR, 4);
    outtextxy(300, 50, "Wybierz poziom trudnoœci gry :");

    ShowPlayerLevelMenu(&settings.iPlayerLevel, false, true);

    outtextxy(300, 650, "Powrót = SPACJA.");
}

void Game::ShowPlayerLevelMenu(int* pCurrentPlayerLevel, bool bMoveUp, bool bShowOnly)
{
    //  wyswietla menu z lista poziomow trudnosci gry do wyboru

    settextstyle(8, HORIZ_DIR, 3);

    if (bShowOnly == false) //  inicjalizujemy menu ?
    {
        if (*pCurrentPlayerLevel == PLAYER_LEVEL_BEGINNER && bMoveUp == true)
            *pCurrentPlayerLevel = PLAYER_LEVEL_MASTER;
        else if (*pCurrentPlayerLevel == PLAYER_LEVEL_BEGINNER && bMoveUp == false)
            *pCurrentPlayerLevel = PLAYER_LEVEL_GOOD;
        else if (*pCurrentPlayerLevel == PLAYER_LEVEL_MASTER && bMoveUp == true)
            *pCurrentPlayerLevel = PLAYER_LEVEL_GOOD;
        else if (*pCurrentPlayerLevel == PLAYER_LEVEL_MASTER && bMoveUp == false)
            *pCurrentPlayerLevel = PLAYER_LEVEL_BEGINNER;
        else if (bMoveUp == true)
            *pCurrentPlayerLevel -= 1;
        else if (bMoveUp == false)
            *pCurrentPlayerLevel += 1;
    }

    CheckMenuOptionForActivation(*pCurrentPlayerLevel, PLAYER_LEVEL_BEGINNER);
    outtextxy(440, 300, "Gracz pocz¹tkuj¹cy");
    CheckMenuOptionForActivation(*pCurrentPlayerLevel, PLAYER_LEVEL_GOOD);
    outtextxy(440, 340, "Gracz na obeznany");
    CheckMenuOptionForActivation(*pCurrentPlayerLevel, PLAYER_LEVEL_MASTER);
    outtextxy(440, 380, "Mistrz labiryntów");
}

void Game::ShowMainMenu(int* pCurrentMenuState, bool bMoveUp, bool bShowOnly)
{
	/*	wyswietla glowne menu gry w postaci listy

    	@ pCurrentMenuState - wskaznik na zmienna ktora zawiera aktualnie
        	zaznaczony element menu. Jest modyfikowany w tej funkcji

        @ bMoveUp - okresla czy uzytkownik porusza sie w menu do gory czy na dol

        @ bShowOnly - okresla czy pokazujemy tylko menu, czy poruszamy sie po nim
    */

    settextstyle(8, HORIZ_DIR, 6);
    setcolor(BLUE);
	outtextxy(350, 100, "Labirynt 2012");

    settextstyle(8, HORIZ_DIR, 3);

    if (bShowOnly == false) //  inicjalizujemy menu ?
    {
		if (*pCurrentMenuState == MENU_OPTION_BEGIN && bMoveUp == true)
			*pCurrentMenuState = MENU_OPTION_QUIT;
	    else if (*pCurrentMenuState == MENU_OPTION_BEGIN && bMoveUp == false)
	    	*pCurrentMenuState = MENU_OPTION_HELP;
	    else if (*pCurrentMenuState == MENU_OPTION_QUIT && bMoveUp == false)
	    	*pCurrentMenuState = MENU_OPTION_BEGIN;
	    else if (*pCurrentMenuState == MENU_OPTION_QUIT && bMoveUp == true)
	    	*pCurrentMenuState = MENU_OPTION_PLAYER_LVL;
	    else if (bMoveUp == true)
	    	*pCurrentMenuState -= 1;
	    else if (bMoveUp == false)
	    	*pCurrentMenuState += 1;
    }

    CheckMenuOptionForActivation(*pCurrentMenuState, MENU_OPTION_BEGIN);
    outtextxy(460, 300, "Nowa gra");
    CheckMenuOptionForActivation(*pCurrentMenuState, MENU_OPTION_HELP);
    outtextxy(460, 340, "Pomoc F1");
    CheckMenuOptionForActivation(*pCurrentMenuState, MENU_OPTION_LOAD_GAME);
    outtextxy(460, 380, "£aduj grê");
    CheckMenuOptionForActivation(*pCurrentMenuState, MENU_OPTION_PLAYER_LVL);
    outtextxy(460, 420, "Poziom trudnoœci");
    CheckMenuOptionForActivation(*pCurrentMenuState, MENU_OPTION_QUIT);
    outtextxy(460, 460, "Wyjœcie");
}

void Game::CheckMenuOptionForActivation(int iCurrentState, int iMenuOption)
{
	//	funkcja wykonuje sprawdzenie czy nalezy podswietlic tekst
    //	jesli argumenty sa rowne to podswietla, jesli rozne to ustawia
    //	kolor tekstu na bialy

	if (iCurrentState == iMenuOption)
    	setcolor(RED);
    else setcolor(WHITE);
}

int Game::CheckKeyboardState()
{
    //	zwraca rezultat w postaci virtual-code wcisnietego klawisza
    //	sprawdzane klawisze to : VK_UP VK_DOWN VK_LEFT VK_RIGHT
    //  VK_RETURN VK_ESCAPE VK_F1 VK_F5. Jesli zaden z wymienionych klawiszy
    //	nie zostal wcisniety, to funkcja zwraca - 1

    if (GetAsyncKeyState(VK_RETURN))
        return VK_RETURN;

    if (GetAsyncKeyState(VK_ESCAPE))
        return VK_ESCAPE;

    if (GetAsyncKeyState(VK_UP))
        return VK_UP;

    if (GetAsyncKeyState(VK_DOWN))
    	return VK_DOWN;

    if (GetAsyncKeyState(VK_LEFT))
        return VK_LEFT;

    if (GetAsyncKeyState(VK_RIGHT))
        return VK_RIGHT;

    if (GetAsyncKeyState(VK_F1))
    	return VK_F1;

    if (GetAsyncKeyState(VK_SPACE))
        return VK_SPACE;

    if (GetAsyncKeyState(VK_F5))
        return VK_F5;

    return -1;
}

void Game::StartGame(bool bPreInitMaze, int iStartSlot)
//  bPrevInitMaze = false @ default
//  iStartSlot = 0 @ default
{
	//	rozpoczyna nowa gre - tworzy plansze

    cleardevice();

    bPlayerNeedUpdate = true;
    iPlayerMoveDirection = 0;
    bMazeNeedUpdate = true;

    delete mazeGenerator;
    mazeGenerator = new MazeGenerator;
    FreeOldMazeDataVect();

    if (bPreInitMaze == true)
    {
        //  inicjalizujemy klase labiryntu danymi pobranymi ze slotu zapisanej gry
        mazeGenerator->PreInitializeMaze(saveSlots[iStartSlot].pbBoard, saveSlots[iStartSlot].saveHeader.iMazeSize);
        iCurrentPlayerYPos = saveSlots[iStartSlot].saveHeader.iyPos;
        iCurrentPlayerXPos = saveSlots[iStartSlot].saveHeader.ixPos;
        mazeGenerator->SetEndPosition(saveSlots[iStartSlot].saveHeader.iYEndPos,
                                      saveSlots[iStartSlot].saveHeader.iXEndPos);
    }
    else
    {
        //  labirynt jest generowany od podstaw, poniewaz gra jest rozpoczynana
        //  od nowa, nie wczytywana ze slotu
        //	okreslamy z jakim poziomem trudnosci ma zostac utworzony labirynt
        switch (settings.iPlayerLevel)
        {
            case PLAYER_LEVEL_BEGINNER:
                mazeGenerator->SetSize(7);
                break;
            case PLAYER_LEVEL_GOOD:
                mazeGenerator->SetSize(11);
                break;
            case PLAYER_LEVEL_MASTER:
                mazeGenerator->SetSize(19);
                break;
        }
        mazeGenerator->GenerateMaze();
        mazeGenerator->GetStartPosition(iCurrentPlayerYPos, iCurrentPlayerXPos);
    }

    mazeGenerator->GetMazeData(mazeData);

	DrawMaze();
	DrawGameTitle();
	bMazeNeedUpdate = false;

    DrawPlayer(iCurrentPlayerYPos, iCurrentPlayerXPos);
}

void Game::RefreshGame()
{
    //  jesli jest pozwolenie na update labiryntu, to wykonujemy

    if (bMazeNeedUpdate == true)    //  czy plansza wymaga odrysowania ? (np gra byla wstrzymywana)
    {
        DrawMaze();
        bMazeNeedUpdate = false;
    }

    DrawGameTitle();
    DrawCounter();

    //  jesli uzytkownik chce poruszyc postacia, zezwalamy na ruch
    if (iPlayerMoveDirection != 0)
        bPlayerNeedUpdate = true;

    //  niech postac gracza odswieza sie co 300 ms
    int iDiff = GetTickCount() - ulMsecsPlayerUpdated;
    if (iDiff > 300)
        bPlayerNeedUpdate = true;

    //  jesli jest zezwolenie na update postaci gracza, to wykonujemy je
    if (bPlayerNeedUpdate == true)
        DrawPlayer(iCurrentPlayerYPos, iCurrentPlayerXPos);
}

void Game::DrawMaze()
{
	//	rysuje na ekranie labirynt

    //  okreslamy w ktorym miejscu na osi X znajdzie sie poczatek labiryntu
    iBoardTop = 710 / 2 - ((mazeGenerator->GetSize() * WALL_SIZE) / 2) + WALL_SIZE;
    iBoardLeft = 1024 / 2 - (mazeGenerator->GetSize() * WALL_SIZE / 2) + WALL_SIZE;
	int iCurrentYPos = iBoardTop;
	int iCurrentXPos = iBoardLeft;
	int iXEnd, iYEnd;

	mazeGenerator->GetEndPosition(iYEnd, iXEnd);

	for (int x = 0; x < mazeGenerator->GetSize(); ++x)
	{
		for (int y = 0; y < mazeGenerator->GetSize(); ++y)
		{
		    if (y == iYEnd && x == iXEnd)
		    {
		        //  rysujemy punkt koncowy - wyjscie z labiryntu
		        setfillstyle(1, RED);
				bar(iCurrentXPos, iCurrentYPos, iCurrentXPos - WALL_SIZE, iCurrentYPos - WALL_SIZE);
		    }
			else if (mazeData[y][x] == true)
			{
				//	rysujemy sciane
				setfillstyle(1, 7);
				bar(iCurrentXPos, iCurrentYPos, iCurrentXPos - WALL_SIZE, iCurrentYPos - WALL_SIZE);
			}
			else
			{
				//	nie ma sciany
				setfillstyle(1, COLOR(0, 128, 128));
				bar(iCurrentXPos, iCurrentYPos, iCurrentXPos - WALL_SIZE, iCurrentYPos - WALL_SIZE);
			}
			iCurrentYPos += WALL_SIZE;
		}

		iCurrentXPos += WALL_SIZE;
		iCurrentYPos = iBoardTop;
	}
}

void Game::DrawPlayer(int iY, int iX)
{
    //  iY oraz iX -> wspolrzedne na powierzchni labiryntu - nie ekranu

    int iPlayerYPos =  (iY * WALL_SIZE) + iBoardTop - WALL_SIZE;
    int iPlayerXPos =  (iX * WALL_SIZE) + iBoardLeft - WALL_SIZE;
    int iPlayerOrigYPos = iPlayerYPos;
    int iPlayerOrigXPos = iPlayerXPos;
    bool bMaskOldPos = true;

    //  okreslamy czy i w ktora strone nalezy przemiescic postac gracza
    //  umieszczono tutaj rowniez warunki blokujace wejscie gracza na sciany (kolizje)
    switch (iPlayerMoveDirection)
    {
        case VK_DOWN:   //  w dol
            if (iY < mazeGenerator->GetSize() - 1 && mazeData[iY + 1][iX] == false)
            {
                iPlayerYPos += WALL_SIZE;
                iCurrentPlayerYPos++;
            }
            break;
        case VK_UP:     //  w gore
            if (iY > 0 && mazeData[iY - 1][iX] == false)
            {
                iPlayerYPos -= WALL_SIZE;
                iCurrentPlayerYPos--;
            }
            break;
        case VK_LEFT:   //  w lewo
            if (iX > 0 && mazeData[iY][iX - 1] == false)
            {
                iPlayerXPos -= WALL_SIZE;
                iCurrentPlayerXPos--;
            }
            break;
        case VK_RIGHT:  //  w prawo
            if (iX < mazeGenerator->GetSize() - 1 && mazeData[iY][iX + 1] == false)
            {
                iPlayerXPos += WALL_SIZE;
                iCurrentPlayerXPos++;
            }
            break;
        default:
            bMaskOldPos = false;
            break;
    }

    if (bMaskOldPos == true)
    {
        //  nalezy zaslonic poprzednia zawartosc klatki
        setfillstyle(1, COLOR(0, 128, 128));
        bar(iPlayerOrigXPos, iPlayerOrigYPos, iPlayerOrigXPos + WALL_SIZE, iPlayerOrigYPos + WALL_SIZE);
    }

    //  losujemy klatke animacji
    int iFrameIndex = GetRandomFromRange(0, 2);
    DrawBitmap(iPlayerYPos, iPlayerXPos, &playerFrame[iFrameIndex]);
    bPlayerNeedUpdate = false;
    ulMsecsPlayerUpdated = GetTickCount();
    iPlayerMoveDirection = 0;

    //  sprawdzamy czy gracz doszedl do koncowego pola labiryntu oznaczonego jako wyjscie
    int iXEnd, iYEnd;

    mazeGenerator->GetEndPosition(iYEnd, iXEnd);
    if (iY == iYEnd && iX == iXEnd)
    {
        //  gracz znajduje sie na pozycji wyjsciowej, konczymy gre
        bGameFinished = true;
        ShowGameFinishedScreen();
    }
}

void Game::DrawCounter()
{
    //  rysowanie licznika czasu gry

    settextstyle(8, HORIZ_DIR, 4);
    setcolor(CYAN);

    if (bRefreshStartTicks == true)
    {
        //  gra byla wstrzymywana, dodajemy czas gry przed pauza do calkowitego czasu gry
        //  i liczymy od nowa
        ulTicksAtGameStart = GetTickCount();
        bRefreshStartTicks = false;
    }

    unsigned long ulDiff = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;

    int iMins = ulDiff / 60000;
    int iSecs = (ulDiff % 60000) / 1000;
    int iMsecs = (ulDiff % 60000) % 100;

    stringstream str;
    str << iMins << ":" << iSecs << ":" << iMsecs;
    // TODO fix non const char in library
    char* buf = new char[str.str().length() + 1];
    strncpy(buf, str.str().c_str(), str.str().length());
    outtextxy(850, 660, buf);
    delete [] buf;
}

void Game::DrawGameTitle()
{
    //  rysuje animacje tytulowa

    if (bStopTitle == true)
        return;

    settextstyle(9, HORIZ_DIR, 4);
    setcolor(CYAN);
	outtextxy(iGameTitleX, 10, "Labirynt 2012");
	static int step = 2;
	static bool canChangeDirection = true;

    if (iGameTitleX > 500 && canChangeDirection == true)
    {
        //  napis musi zmienic swoj kierunek - w lewo
        step = -2;
        canChangeDirection = false;
    }
    else if (iGameTitleX < 50)
    {
        //  zmieniamy kierunek - w prawo
        step = 2;
        canChangeDirection = true;
    }

    iGameTitleX += step;

    settextstyle(8, HORIZ_DIR, 3);
    setcolor(CYAN);
    outtextxy(10, 670, "F1 = Pomoc / Pauza | F5 = Zapisz grê | ESC = Powrót do menu.");
}

bool Game::InitGameSaves()
{
	//	inicjuje system zapisu gry - laduje wszystkie sloty

	saveSlots = new SaveSlotStruct[4];

	if (LoadSlotContents("Savegame\\slot1.sav", &saveSlots[0]) == false)
		return false;

    if (LoadSlotContents("Savegame\\slot2.sav", &saveSlots[1]) == false)
		return false;

    if (LoadSlotContents("Savegame\\slot3.sav", &saveSlots[2]) == false)
		return false;

	if (LoadSlotContents("Savegame\\slot4.sav", &saveSlots[3]) == false)
		return false;

	return true;
}

bool Game::UpdateGameSaves()
{
    //  wykonuje zapis wszystkich slotow do plikow

    if (SaveSlotContents("Savegame\\slot1.sav", &saveSlots[0]) == false)
		return false;

    if (SaveSlotContents("Savegame\\slot2.sav", &saveSlots[1]) == false)
		return false;

    if (SaveSlotContents("Savegame\\slot3.sav", &saveSlots[2]) == false)
		return false;

    if (SaveSlotContents("Savegame\\slot4.sav", &saveSlots[3]) == false)
		return false;

    return true;
}

bool Game::LoadSlotContents(const char* szFileName, SaveSlotStruct* pSaveSlotStruct)
{
	//	laduje zawartosc podanego pliku do pamieci i wypelnia strukture pSaveSlotStruct

	char szExeDir[1024];
	GetExeDirectory(szExeDir);
	strcat(szExeDir, "\\");
	strcat(szExeDir, szFileName);

	HANDLE hFile = CreateFile(szExeDir,
							  GENERIC_READ,
							  0,
							  0,
							  OPEN_ALWAYS,
							  0,
							  0);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	//	sprawdzamy rozmiar pliku
	if (GetFileSize(hFile, NULL) == 0)
	{
		//	plik ma zerowy rozmiar
		CloseHandle(hFile);
		pSaveSlotStruct->saveHeader.bSlotIsUsed = false;
		return true;
	}

	DWORD dwRead;
	//	ladujemy zawartosc naglowka pliku do pamieci
	ReadFile(hFile, &pSaveSlotStruct->saveHeader, sizeof(pSaveSlotStruct->saveHeader),
			 &dwRead, 0);

	//	mamy zaladowany naglowek pliku, wiec znany jest rozmiar labiryntu
	//	ladujemy tablice labiryntu do pamieci

	//	najpierw tworzona jest macierz o rozmiarze iMazeSize x iMazeSize
	pSaveSlotStruct->pbBoard = new bool*[pSaveSlotStruct->saveHeader.iMazeSize];
	for (int i = 0; i < pSaveSlotStruct->saveHeader.iMazeSize; ++i)
	{
		pSaveSlotStruct->pbBoard[i] = new bool[pSaveSlotStruct->saveHeader.iMazeSize];
	}

    for (int x = 0; x < pSaveSlotStruct->saveHeader.iMazeSize; ++x)
    {
        ReadFile(hFile, pSaveSlotStruct->pbBoard[x], sizeof(bool) * pSaveSlotStruct->saveHeader.iMazeSize, &dwRead, 0);
    }

	CloseHandle(hFile);
	return true;
}

bool Game::SaveSlotContents(const char* szFileName, SaveSlotStruct* pSaveSlotStruct)
{
	//	wykonuje zapis wskazanego pliku w argumencie z pamieci na dysk twardy

	char szExeDir[1024];
	GetExeDirectory(szExeDir);
	strcat(szExeDir, "\\");
	strcat(szExeDir, szFileName);

	HANDLE hFile = CreateFile(szExeDir,
							  GENERIC_WRITE,
							  0,
							  0,
							  CREATE_ALWAYS,
							  0,
							  0);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwWrite;
	//	zapisujemy zawartosc naglowka slotu, do pliku na HDD
	WriteFile(hFile, &pSaveSlotStruct->saveHeader, sizeof(pSaveSlotStruct->saveHeader),
			 &dwWrite, 0);

	//	naglowek zostal zapisany, teraz laduja dane labiryntu

    for (int x = 0; x < pSaveSlotStruct->saveHeader.iMazeSize; ++x)
    {
        WriteFile(hFile, pSaveSlotStruct->pbBoard[x], sizeof(bool) * pSaveSlotStruct->saveHeader.iMazeSize, &dwWrite, 0);
    }

	CloseHandle(hFile);
	return true;
}

bool Game::GameSlotCanBeLoaded(int iIndex)
{
    //  funkcja uzywana do okreslenia czy slot o podanym indeksie moze zostac zaladowany
    //  jesli wczesniej podany slot nie byl uzyty do zapisu - to nie moze byc wczytany

    return saveSlots[iIndex].saveHeader.bSlotIsUsed == true;
}

void Game::RestoreGameState(int iSlotIndex)
{
    /*
        Funkcja wykonuje przywrocenie gry do stanu z przed zapisu. Jako argument
        przyjmuje indeks slotu ktory ladujemy.
    */

    ulGameTimeMsecsIncrement = saveSlots[iSlotIndex].saveHeader.iTimeFromStart;

    StartGame(true, iSlotIndex);
}

void Game::SaveGameState(int iSlotIndex)
{
    /*
        Zapisuje aktualny stan gry do slotu o podanym indeksie.
    */

    //  w pierwszej kolejnosci nalezy sprawdzic, czy slot byl wczesniej uzywany
    //  poniewaz jesli byl, to oznacza ze trzeba najpierw zwolnic pamiec mu przydzielona
    if (saveSlots[iSlotIndex].saveHeader.bSlotIsUsed == true)
    {
        //  slot byl uzywany, w petli zwalniamy pamiec
        for (int x = 0; x < saveSlots[iSlotIndex].saveHeader.iMazeSize; ++x)
        {
            delete [] saveSlots[iSlotIndex].pbBoard[x];
        }

        delete [] saveSlots[iSlotIndex].pbBoard;
    }

    int iXStart, iYStart, iXEnd, iYEnd;

    mazeGenerator->GetStartPosition(iYStart, iXStart);
    mazeGenerator->GetEndPosition(iYEnd, iXEnd);

    //  wypelniamy dane naglowka slotu
    saveSlots[iSlotIndex].saveHeader.iTimeFromStart = GetTickCount() - ulTicksAtGameStart + ulGameTimeMsecsIncrement;
    saveSlots[iSlotIndex].saveHeader.ixPos = iCurrentPlayerXPos;
    saveSlots[iSlotIndex].saveHeader.iyPos = iCurrentPlayerYPos;
    saveSlots[iSlotIndex].saveHeader.iMazeSize = mazeGenerator->GetSize();
    saveSlots[iSlotIndex].saveHeader.bSlotIsUsed = true;
    saveSlots[iSlotIndex].saveHeader.iXStartPos = iXStart;
    saveSlots[iSlotIndex].saveHeader.iYStartPos = iYStart;
    saveSlots[iSlotIndex].saveHeader.iXEndPos = iXEnd;
    saveSlots[iSlotIndex].saveHeader.iYEndPos = iYEnd;
    strncpy(saveSlots[iSlotIndex].saveHeader.szSaveDateTime, PrepareCurrentDateTimeString().c_str(),
            sizeof(saveSlots[iSlotIndex].saveHeader.szSaveDateTime));

    //  przydzielamy pamiec dla labiryntu
    saveSlots[iSlotIndex].pbBoard = new bool*[mazeGenerator->GetSize()];

    //  wypelniamy dane labiryntu w slocie
    for (int x = 0; x < mazeGenerator->GetSize(); ++x)
	{
	    saveSlots[iSlotIndex].pbBoard[x] = new bool[mazeGenerator->GetSize()];

		for (int y = 0; y < mazeGenerator->GetSize(); ++y)
		{
			saveSlots[iSlotIndex].pbBoard[x][y] = mazeData[y][x];
		}
	}

	//  slot zostal zapisany
}

bool Game::GetExeDirectory(char* szExeDir)
{
    //  Pobiera nazwe katalogu w ktorym znajduje sie plik wykonywalny gry

	char szExeName [MAX_PATH];

	GetModuleFileName(NULL, szExeName, MAX_PATH);
	char* end = strrchr(szExeName, '\\');
	if (end == NULL)
		return false;

	strncpy(szExeDir, szExeName, strlen(szExeName) - strlen(end));
	szExeDir[strlen(szExeName) - strlen(end)] = '\0';

	return true;
}

bool Game::LoadBmpFile(const char* szFileName, BitmapClass* bitmap)
{
    /*
        Funkcja laduje plik bitmapy o nazwie przekazanej jako argument, do pamieci.
        Wypelnia klase bitmap informacjami o obrazku oraz danymi pixeli.
        Zwraca rezultat dzialania true / false
    */

    //  otwieramy bitmape
    HBITMAP hBmp = (HBITMAP) LoadImage(0, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBmp == NULL)
    {
        MessageBox(0, "LoadImage() - b³¹d.", "B³¹d", 0);
        return false;
    }

    BITMAPINFO bmpInfo;
    memset(&bmpInfo, 0, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    //  ladujemy informacje o bitmapie najpierw
    int ret = GetDIBits(CreateCompatibleDC(NULL), hBmp, 0, 0, NULL, &bmpInfo, 0);
    if (ret == 0)
    {
        MessageBox(0, "GetDIBits() - b³¹d pobierania informacji o pliku BMP.", "B³¹d", 0);
        CloseHandle(hBmp);
        return false;
    }

    //  ladujemy pixele bitmapy
    bitmap->plPixels = new long[bmpInfo.bmiHeader.biHeight * bmpInfo.bmiHeader.biWidth];
    ret = GetDIBits(CreateCompatibleDC(NULL), hBmp, 0, bmpInfo.bmiHeader.biHeight, bitmap->plPixels,
                    &bmpInfo, DIB_RGB_COLORS);
    if (ret == 0)
    {
        MessageBox(0, "GetDIBits() - b³¹d pobierania pixeli.", "B³¹d", 0);
        CloseHandle(hBmp);
        return false;
    }

    //  wypelniamy pola klasy BitmapClass informacjami o obrazku
    bitmap->iHeight = bmpInfo.bmiHeader.biHeight;
    bitmap->iWidth = bmpInfo.bmiHeader.biWidth;

    CloseHandle(hBmp);
    return true;
}

void Game::DrawBitmap(int iY, int iX, BitmapClass* bitmap)
{
    //  Rysuje bitmape przekazana jako argument we wskazanych koordynatach ekranu

    int iXorig = iX;
    iY += bitmap->iHeight - 1;

    //  obrazek rysujemy od ostatniej linii, poniewaz pixele sa ulozone na odwrot w tablicy
    for(int iLineStart = 0; iLineStart < bitmap->iHeight * bitmap->iWidth; iLineStart += bitmap->iWidth)
    {
        for (int xIndex = 0; xIndex < bitmap->iWidth; ++xIndex)
        {
            //  umieszczamy pojedynczy pixel obrazka na ekranie
            putpixel(iX, iY, COLOR((bitmap->plPixels[xIndex + iLineStart] >> 16) & 0xFF,
                                   (bitmap->plPixels[xIndex + iLineStart] >> 8) &  0xFF,
                                    bitmap->plPixels[xIndex + iLineStart] & 0xFF));
            iX++;
        }
        iY--;   //  dekrementujemy pozycje pixela, poniewaz rysujemy od dolu w gore
        iX = iXorig;
    }
}

bool Game::LoadGameBitmaps()
{
    //  zadaniem funkcji jest zaladowanie wszystkich bitmap ktore sa uzywane w grze
    //  do pamieci. Funkcja zwraca rezultat swojego dzialani w postaci true / false

    char szExeDir[MAX_PATH];
    GetExeDirectory(szExeDir);

    //  ladujemy klatki animacji obiektu sterowanego przez gracza
    if (LoadBmpFile((string(szExeDir) + string("\\Images\\player_frame_1.bmp")).c_str(), &playerFrame[0]) == false)
        return false;

    if (LoadBmpFile((string(szExeDir) + string("\\Images\\player_frame_2.bmp")).c_str(), &playerFrame[1]) == false)
        return false;

    if (LoadBmpFile((string(szExeDir) + string("\\Images\\player_frame_3.bmp")).c_str(), &playerFrame[2]) == false)
        return false;

    return true;
}

int Game::GetRandomFromRange(int iMin, int iMax)
{
	//	losuje liczbe z podanego przedzialu

	return rand () % (iMax - iMin + 1) + iMin;
}

void Game::ShowGameSlotsMenu(int* pCurrentSlotPosition)
{
    //  pokazuje ekran z lista gier do wczytania

    cleardevice();

    settextstyle(8, HORIZ_DIR, 4);
    setcolor(CYAN);
    outtextxy(200, 100, "Wczytaj grê");

    //  wyswietlamy liste slotow, zaznaczony jest nr. 1
    CheckGameSlotForActivation(pCurrentSlotPosition, false, true);

    settextstyle(8, HORIZ_DIR, 3);
    setcolor(BLUE);
    outtextxy(200, 600, "ENTER = Za³aduj wybrany slot.");
    outtextxy(200, 640, "ESC = Powrót do g³ównego menu.");
}

void Game::ShowGameSaveSlotsMenu(int* pCurrentSlotPosition)
{
    //  pokazuje ekran z lista slotow do zapisu

    cleardevice();

    settextstyle(8, HORIZ_DIR, 4);
    setcolor(CYAN);
    outtextxy(200, 100, "Zapisz grê");

    //  wyswietlamy liste slotow, zaznaczony jest nr. 1
    CheckGameSlotForActivation(pCurrentSlotPosition, false, true);

    settextstyle(8, HORIZ_DIR, 3);
    setcolor(BLUE);
    outtextxy(200, 600, "ENTER = Zapisz grê pod wybranym slotem.");
    outtextxy(200, 640, "ESC = Powrót do gry.");
}

void Game::CheckGameSlotForActivation(int* pCurrentSlotPosition, bool bMoveUp, bool bShowOnly)
{
    //  wykonuje sprawdzenie, czy nalezy zmienic kolor elementu menu (aktywowac go)

    settextstyle(8, HORIZ_DIR, 4);

    if (bShowOnly == false) //  czy tylko inicjalizujemy menu ?
    {
        if (*pCurrentSlotPosition == SAVE_SLOT_1 && bMoveUp == true)
            *pCurrentSlotPosition = SAVE_SLOT_4;
        else if (*pCurrentSlotPosition == SAVE_SLOT_1 && bMoveUp == false)
            *pCurrentSlotPosition = SAVE_SLOT_2;
        else if (*pCurrentSlotPosition == SAVE_SLOT_4 && bMoveUp == false)
            *pCurrentSlotPosition = SAVE_SLOT_1;
        else if (*pCurrentSlotPosition == SAVE_SLOT_4 && bMoveUp == true)
            *pCurrentSlotPosition = SAVE_SLOT_3;
        else if (bMoveUp == true)
	    	*pCurrentSlotPosition -= 1;
	    else if (bMoveUp == false)
	    	*pCurrentSlotPosition += 1;
    }

    //  zaslaniamy napis "wybrany slot nie zawiera gry" (nawet jesli byl nie pokazany)
    setfillstyle(1, BLACK);
    bar(100, 500, 700, 550);

    CheckMenuOptionForActivation(*pCurrentSlotPosition, SAVE_SLOT_1);
    outtextxy(360, 300, "Slot 1");
    if (saveSlots[0].saveHeader.bSlotIsUsed == true)
    {
        outtextxy(550, 300, saveSlots[0].saveHeader.szSaveDateTime);
    }
    CheckMenuOptionForActivation(*pCurrentSlotPosition, SAVE_SLOT_2);
    outtextxy(360, 340, "Slot 2");
    if (saveSlots[1].saveHeader.bSlotIsUsed == true)
    {
        outtextxy(550, 340, saveSlots[1].saveHeader.szSaveDateTime);
    }
    CheckMenuOptionForActivation(*pCurrentSlotPosition, SAVE_SLOT_3);
    outtextxy(360, 380, "Slot 3");
    if (saveSlots[2].saveHeader.bSlotIsUsed == true)
    {
        outtextxy(550, 380, saveSlots[2].saveHeader.szSaveDateTime);
    }
    CheckMenuOptionForActivation(*pCurrentSlotPosition, SAVE_SLOT_4);
    outtextxy(360, 420, "Slot 4");
    if (saveSlots[3].saveHeader.bSlotIsUsed == true)
    {
        outtextxy(550, 420, saveSlots[3].saveHeader.szSaveDateTime);
    }
}

int Game::GetCurrentBPP()
{
    //  zwraca aktualna glebie kolorow, w bitach na piksel

    return GetDeviceCaps(CreateCompatibleDC(NULL), BITSPIXEL);
}

string Game::PrepareCurrentDateTimeString()
{
    //  funkcja przygotowuje string zawierajacy aktualna date
    //  oraz godzine w formie np. 23-12-12 13:23

    stringstream str;
    SYSTEMTIME sysTime;

    GetLocalTime(&sysTime);

    str << sysTime.wDay << "-" << sysTime.wMonth << "-" << sysTime.wYear << " ";
    str << sysTime.wHour << ":" << sysTime.wMinute;

    return str.str();
}

void Game::FreeOldMazeDataVect()
{
    for (unsigned x = 0; x < mazeData.size(); ++x)
    {
        mazeData[x].clear();
    }

    mazeData.clear();
}

bool Game::LoadGameSettings()
{
    //  laduje ustawienia gry z pliku, takie jak poziom trudnosci

    char szFName[MAX_PATH];
    GetExeDirectory(szFName);
    strcat(szFName, "\\config.bin");

    HANDLE hFile = CreateFile(szFName,
                              GENERIC_READ,
                              0,
                              0,
                              OPEN_ALWAYS,
                              0,
                              0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;   //  nie udalo sie otworzyc pliku..

    if (GetFileSize(hFile, NULL) != sizeof(SettingsStruct))
    {
        //  plik ma zly rozmiar, ustawiamy def. wartosci
        memset(&settings, 0, sizeof(SettingsStruct));
        settings.iPlayerLevel = PLAYER_LEVEL_GOOD;
    }
    else
    {
        DWORD dwRead;
        ReadFile(hFile, &settings, sizeof(SettingsStruct), &dwRead, 0);
    }

    CloseHandle(hFile);
    return true;
}

bool Game::SaveGameSettings()
{
    //  zapisuje ustawienia gry do pliku

    char szFName[MAX_PATH];
    GetExeDirectory(szFName);
    strcat(szFName, "\\config.bin");

    HANDLE hFile = CreateFile(szFName,
                              GENERIC_WRITE,
                              0,
                              0,
                              CREATE_ALWAYS,
                              0,
                              0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;   //  nie udalo sie otworzyc pliku..

    DWORD dwWrite;
    WriteFile(hFile, &settings, sizeof(SettingsStruct), &dwWrite, 0);

    CloseHandle(hFile);
    return true;
}
