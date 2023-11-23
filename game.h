#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <bgi/winbgim.h>
#include <windows.h>
#include <string.h>
#include <string>
#include <cmath>
#include <vector>

#include "maze-generator.h"
#include "defines.h"

class Game
{
	public:
    	Game();
        ~Game();
        void InitGame();
        void SaveGame(int iSlot);
        bool LoadGame(int iSlot);

    private:
		struct SaveHeaderStruct
		{
			int iTimeFromStart;		    //	ile czasu uplynelo od rozpoczecia gry (w milisekundach)
			int ixPos;				    //	jaka byla pozycja X gracza przy zapisie gry
			int iyPos;				    //	pozycja Y gracza przy zapisie gry
			int iMazeSize;			    //	rozmiar labiryntu (jeden z bokow)
			bool bSlotIsUsed;		    //	czy ten slot jest uzywany i zawiera poprawne wartosci zmiennych ? Jesli nie to przyjmuje false
			char szSaveDateTime[32];    //  czas kiedy wykonano zapis do slotu
			int iXStartPos;             //  x pozycja startowa
			int iYStartPos;             //  y pozycja startowa
			int iXEndPos;               //  x pozycja koncowa
			int iYEndPos;               //  y pozycja koncowa
		};
		struct SaveSlotStruct
		{
			SaveHeaderStruct saveHeader;	//	naglowek zapisanego stanu gry
			bool** pbBoard;					//	tablica wskaznikow tablic typu bool, true = mur, false = brak muru
		};
		class BitmapClass
		{
		    public:
                BitmapClass()
                {
                    plPixels = NULL;
                }
                ~BitmapClass()
                {
                    delete [] plPixels;
                }
                int iWidth;
                int iHeight;
                long* plPixels;
		};
		struct SettingsStruct
		{
            int iPlayerLevel;
		};

        SettingsStruct settings;    //  struktura z ustawieniami programu
		SaveSlotStruct* saveSlots;	//	tablica wskaznikow z zapisanymi stanami gry
		MazeGenerator* mazeGenerator;
		int iBoardTop, iBoardLeft, iGameTitleX, iCurrentPlayerYPos, iCurrentPlayerXPos,
            iPlayerMoveDirection;
		unsigned long ulTicksAtGameStart, ulGameTimeMsecsIncrement, ulMsecsPlayerUpdated, ulMsecsAtSaveGame;
		std::vector< std::vector<bool> > mazeData;
        BitmapClass playerFrame[3];
        bool bPlayerNeedUpdate, bMazeNeedUpdate, bRefreshStartTicks, bGameFinished, bHelpReturnToGame,
             bStopTitle;

    	int CheckKeyboardState();
        void ShowMainMenu(int* pCurrentMenuState, bool bMoveUp, bool bShowOnly);
        void CheckMenuOptionForActivation(int iCurrentState, int iMenuOption);
		void StartGame(bool bPreInitMaze = false, int iStartSlot = 0);
		void RefreshGame();
		void RestoreGameState(int iSlotIndex);
		void SaveGameState(int iSlotIndex);
		bool InitGameSaves();
		bool UpdateGameSaves();
		bool LoadSlotContents(const char* szFileName, SaveSlotStruct* pSaveSlotStruct);
		bool SaveSlotContents(const char* szFileName, SaveSlotStruct* pSaveSlotStruct);
		bool GameSlotCanBeLoaded(int iIndex);
		void ShowGameSlotsMenu(int* pCurrentSlotPosition);
		void ShowGameSaveSlotsMenu(int* pCurrentSlotPosition);
		void CheckGameSlotForActivation(int* pCurrentSlotPosition, bool bMoveUp, bool bShowOnly);
		void DrawMaze();
		void DrawPlayer(int iY, int iX);
		void DrawCounter();
		void DrawGameTitle();
		void ShowHelpScreen();
		void ShowGameFinishedScreen();
		void ShowPlayerLevelScreen();
		void ShowPlayerLevelMenu(int* pCurrentPlayerLevel, bool bMoveUp, bool bShowOnly);
		bool LoadBmpFile(const char* szFileName, BitmapClass* bitmap);
		inline void DrawBitmap(int iY, int iX, BitmapClass* bitmap);
		bool LoadGameBitmaps();
		inline int GetRandomFromRange(int iMin, int iMax);
		bool GetExeDirectory(char* szExeDir);
		int GetCurrentBPP();
		std::string PrepareCurrentDateTimeString();
		void FreeOldMazeDataVect();
		bool LoadGameSettings();
		bool SaveGameSettings();
};

#endif
