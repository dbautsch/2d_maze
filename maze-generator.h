#ifndef MAZE_GENERATOR_H_INCLUDED
#define MAZE_GENERATOR_H_INCLUDED

#include <windows.h>
#include <vector>
#include <sstream>
#include <ctime>
#include <fstream>

#include "stack.h"

class MazeGenerator
{
	public:
		class Cell
		{
			public:
				Cell(bool bIsUp = true, bool bIsVisited = false)
				{

					this->bIsUp = bIsUp;
					this->bIsVisited = bIsVisited;
				}
				Cell(const Cell& c)
				{
					this->bIsUp = c.bIsUp;
					this->bIsVisited = c.bIsVisited;
				}
				~Cell() { }

				bool bIsUp;
				bool bIsVisited;

				Cell operator=(const Cell& cell)
				{
					return Cell(cell);
				}
		};
		class Pair
		{
			public:
				Pair(int iY = 0, int iX = 0)
				{
					this->iY = iY;
					this->iX = iX;

				}
				Pair(const Pair& p)
				{
					this->iX = p.iX;
					this->iY = p.iY;
				}
				~Pair() { }

				int iX, iY;

				Pair operator=(const Pair& p)
				{
					return Pair(p);
				}
		};

    	MazeGenerator();
        ~MazeGenerator();
        void SetSize(short size);
        short GetSize() { return size; }
        void GenerateMaze();
        void GetMazeData(std::vector< std::vector<bool> >& mazeData);
        void GetStartPosition(int& iY, int& iX);
        void GetEndPosition(int& iY, int& iX);
        void PreInitializeMaze(bool** pbMazeData, int iSize);
        void SetStartPosition(int iY, int iX);
        void SetEndPosition(int iY, int iX);

    private:
		std::vector< std::vector<Cell> > cellsVect;
		int iXStart, iYStart, iXEnd, iYEnd, size;

		void InitWalls();
		int FindUnvisitedNeighbors(int y, int x, std::vector<MazeGenerator::Pair>& unVisitedVect);
		inline int GetRandomFromRange(int iMin, int iMax);
		inline void BreakWallBetweenCells(int iY1, int iX1, int iY2, int iX2);


        bool GetExeDirectory(char* szExeDir)
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

        void toFile(std::stringstream& str)
        {
            std::ofstream f;
            f.open("c:\\log.txt");
            f << str.str();
            f.close();
        }
};

#endif
