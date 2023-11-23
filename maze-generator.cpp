#include "maze-generator.h"

using namespace std;

MazeGenerator::MazeGenerator()
{
    //  konstruktor klasy generatora labiryntow
}

MazeGenerator::~MazeGenerator()
{
    //  destruktor
}

void MazeGenerator::SetSize(short size)
{
    //  ustawia rozmiar labiryntu

	this->size = size;
}

void MazeGenerator::GenerateMaze()
{
    //  rozpoczyna generowanie labiryntu

	Stack<MazeGenerator::Pair> cellXYStack;
	int iTotalCells = size * size;
	int iVisitedCells = 1;
	MazeGenerator::Pair currentCellXY(0, 0);
	vector<Pair> unvisitedXYVect;

	InitWalls();
	cellsVect[0][0].bIsUp = false;
	cellsVect[0][0].bIsVisited = true;

	iYStart = 0;
	iXStart = 0;

	srand((unsigned) time(NULL));

	while (iVisitedCells < iTotalCells)
	{
		unvisitedXYVect.clear();

		int iNeighbors = FindUnvisitedNeighbors(currentCellXY.iY, currentCellXY.iX, unvisitedXYVect);
		if (iNeighbors > 0)
		{
			//	znaleziono jeden lub wiecej nieodwiedzonych sasiadow
			int iRandom = GetRandomFromRange(0, iNeighbors - 1);
			cellsVect[currentCellXY.iY][currentCellXY.iX].bIsVisited = true;

			BreakWallBetweenCells(currentCellXY.iY, currentCellXY.iX,
								  unvisitedXYVect[iRandom].iY, unvisitedXYVect[iRandom].iX);
            cellsVect[currentCellXY.iY][currentCellXY.iX].bIsUp = false;


			cellXYStack.Push(currentCellXY);

			currentCellXY.iY = unvisitedXYVect[iRandom].iY;
			currentCellXY.iX = unvisitedXYVect[iRandom].iX;

			++iVisitedCells;
		}
		else
		{
			//	nie znaleziono zadnych nieodwiedzonych sasiadow
			if (cellXYStack.HasElements() == false)
                break;
			currentCellXY = cellXYStack.Pop();
			iYEnd = currentCellXY.iY;
			iXEnd = currentCellXY.iX;
			cellsVect[iYEnd][iXEnd].bIsUp = false;
		}
	}
}

void MazeGenerator::BreakWallBetweenCells(int iY1, int iX1, int iY2, int iX2)
{
	//	zadaniem funkcji jest znalezienie muru ktory stoi pomiedzy dwoma punktami
	//	oraz usuniecie go (ustawienie zmiennej bIsUp na false)

	int y, x;	//	tutaj znajda sie wspolrzedne muru ktory nalezy opuscic w dol i zrobic przejscie

	if (iY1 == iY2)
	{
		y = iY1;
		if (iX1 > iX2)
		{
			//	po lewej
			x = iX1 - 1;
		}
		else if (iX1 < iX2)
		{
			//	po prawej
			x = iX1 + 1;
		}
	}
	else if (iX1 == iX2)
	{
		x = iX1;
		if (iY1 > iY2)
		{
			//	na dole
			y = iY1 - 1;
		}
		else if (iY1 < iY2)
		{
			//	na gorze
			y = iY1 + 1;
		}
	}

	stringstream str;
	str << "y1 " << iY1 << ", x1 " << iX1 << ", y2 " << iY2 << ", x2 " << iX2;
	str << "\n calculated y = " << y << " x = " << x;
	toFile(str);

	cellsVect[y][x].bIsUp = false;
	cellsVect[y][x].bIsVisited = true;
}

int MazeGenerator::FindUnvisitedNeighbors(int y, int x, vector<MazeGenerator::Pair>& unVisitedVect)
{
	/*
		wyszukuje wszystkich sasiadow ktorzy sa nienaruszeni (nieodwiedzeni)
		w zaleznosci od tego w jakim miejscu lezy komorka na siatce, bedzie miala
		rozna maksymalna ilosc sasiadow.
		Jesli lezy na srodku, to bedzie miala 8 sasiadow.
	*/

	int iRet = 0;
	vector<MazeGenerator::Pair> neighborsVect;

	//	do rozpatrzenia jest 9 przypadkow

	int iMaxIndex = size - 1;

    //	1 przypadek - jest 4 sasiadow
	if (y > 0 && y < iMaxIndex && x > 0 && x < iMaxIndex)
	{
		neighborsVect.resize(4);
		neighborsVect[0].iY = y - 2;	neighborsVect[0].iX = x;
		neighborsVect[1].iY = y;		neighborsVect[1].iX = x - 2;
		neighborsVect[2].iY = y + 2;	neighborsVect[2].iX = x;
		neighborsVect[3].iY = y;		neighborsVect[3].iX = x + 2;
	}
	//	2 przypadek - gorny lewy rog - jest 2 sasiadow
	else if (y == 0 && x == 0)
	{
		neighborsVect.resize(2);
		neighborsVect[0].iY = y + 2;	neighborsVect[0].iX = x;
		neighborsVect[1].iY = y;		neighborsVect[1].iX = x + 2;
	}
	//	3 przypadek - dolny lewy rog
	else if (y == iMaxIndex && x == 0)
	{
		neighborsVect.resize(2);
		neighborsVect[0].iY = y - 2;	neighborsVect[0].iX = x;
		neighborsVect[1].iY = y;		neighborsVect[1].iX = x + 2;
	}
	//	4 - gorny prawy rog
	else if (y == 0 && x == iMaxIndex)
	{
		neighborsVect.resize(2);
		neighborsVect[0].iY = y;		neighborsVect[0].iX = x - 2;
		neighborsVect[1].iY = y + 2;	neighborsVect[1].iX = x;
	}
	//	5 - dolny prawy rog
	else if (y == iMaxIndex && x == iMaxIndex)
	{
		neighborsVect.resize(2);
		neighborsVect[0].iY = y;		neighborsVect[0].iX = x - 2;
		neighborsVect[1].iY = y - 2;	neighborsVect[1].iX = x;
	}

	//	6 - gora pomiedzy bokami - jest 3 sasiadow
	else if (y == 0 && x > 0 && x < iMaxIndex)
	{
		neighborsVect.resize(3);
		neighborsVect[0].iY = y;		neighborsVect[0].iX = x - 2;
		neighborsVect[1].iY = y + 2;	neighborsVect[1].iX = x;
		neighborsVect[2].iY = y;		neighborsVect[2].iX = x + 2;
	}
	//	7 - dol pomiedzy bokami
	else if (y == iMaxIndex && x > 0 && x < iMaxIndex)
	{
		neighborsVect.resize(3);
		neighborsVect[0].iY = y;		neighborsVect[0].iX = x - 2;
		neighborsVect[1].iY = y - 2;	neighborsVect[1].iX = x;
		neighborsVect[2].iY = y;		neighborsVect[2].iX = x + 2;
	}
	//	8 - lewa strona pomiedzy bokami
	else if (y > 0 && y < iMaxIndex && x == 0)
	{
		neighborsVect.resize(3);
		neighborsVect[0].iY = y + 2;	neighborsVect[0].iX = x;
		neighborsVect[1].iY = y;		neighborsVect[1].iX = x + 2;
		neighborsVect[2].iY = y - 2;	neighborsVect[2].iX = x;
	}
	//	9 - prawa strona pomiedzy bokami
	else if (y > 0 && y < iMaxIndex && x == iMaxIndex)
	{
		neighborsVect.resize(3);
		neighborsVect[0].iY = y + 2;	neighborsVect[0].iX = x;
		neighborsVect[1].iY = y;		neighborsVect[1].iX = x - 2;
		neighborsVect[2].iY = y - 2;	neighborsVect[2].iX = x;
	}

	//	teraz w petli obliczamy ile z sasiadow bylo juz odwiedzonych

	for (unsigned i = 0; i < neighborsVect.size(); ++i)
	{
		//	jesli komorka jeszcze byla nieodwiedzana, inkrementujemy ilosc
		//	oraz odkladamy informacje o wspolrzednych komorki do wektora sasiadow nieodwiedzonych
		if (cellsVect[neighborsVect[i].iY][neighborsVect[i].iX].bIsVisited == false)
		{
			Pair p(neighborsVect[i].iY, neighborsVect[i].iX);
			unVisitedVect.push_back(p);
			iRet++;
		}
	}

	return iRet;
}

void MazeGenerator::InitWalls()
{
	//	inicjalizujemy sciany labiryntu - wszystkie sa "podniesione"
	//	czyli tworzona jest forma na wzor kratownicy, ktora jest obrabiana
	//	w funkcji GenerateMaze()

	cellsVect.resize(size);

	for (int y = 0; y < size; ++y)
	{
		cellsVect[y].resize(size);
		for (int x = 0; x < size; ++x)
		{
            cellsVect[y][x].bIsUp = true;
		}
	}
}

int MazeGenerator::GetRandomFromRange(int iMin, int iMax)
{
	//	losuje liczbe z podanego przedzialu

	return rand () % (iMax - iMin + 1) + iMin;
}

void MazeGenerator::GetMazeData(std::vector< std::vector<bool> >& mazeData)
{
	//	zapisuje dane labiryntu do zmiennej mazeData
	mazeData.clear();
	mazeData.resize(size);

	for (int y = 0; y < size; ++y)
	{
		mazeData[y].resize(size);
		for (int x = 0; x < size; ++x)
		{
			mazeData[y][x] = cellsVect[y][x].bIsUp;
		}
	}
}

void MazeGenerator::GetStartPosition(int& iY, int& iX)
{
    iY = iYStart;
    iX = iXStart;
}

void MazeGenerator::GetEndPosition(int& iY, int& iX)
{
    iY = iYEnd;
    iX = iXEnd;
}

void MazeGenerator::PreInitializeMaze(bool** pbMazeData, int iSize)
{
    cellsVect.resize(iSize);
    this->size = iSize;

    for (int y = 0; y < iSize; ++y)
    {
        cellsVect[y].resize(iSize);
        for (int x = 0; x < iSize; x++)
        {
            cellsVect[y][x].bIsUp = pbMazeData[y][x];
        }
    }
}

void MazeGenerator::SetStartPosition(int iY, int iX)
{
    this->iXStart = iX;
    this->iYStart = iY;
}

void MazeGenerator::SetEndPosition(int iY, int iX)
{
    this->iXEnd = iX;
    this->iYEnd = iY;
}
