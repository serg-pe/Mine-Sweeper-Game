#include <fstream>
#include "Game.h"

using namespace System::Windows::Forms;

Game::Game(int width, int height, int mines)
{
	//player = new Player();
	this->width = width;
	this->height = height;
	this->mines = mines;
	player.setMines(mines);
	flags = mines;
}

Game::~Game()
{
}

void Game::createField(Form ^f) {
	//GameCell **gField;   ���� ���, �������� ���)
	float xStart,
		yStart;
	yStart = (f->Height / 2) - (height / 2)*Cell::edge;
	xStart = (f->Width / 2) - (width / 2)*Cell::edge;

	field = new GameCell*[width];

	for (int i = 0; i < width; i++) {
		field[i] = new GameCell[height];
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			field[i][j].setXStart(xStart + i*Cell::edge);
			field[i][j].setYStart(yStart + j*Cell::edge);
			field[i][j].setXEnd(field[i][j].getXStart() + Cell::edge);
			field[i][j].setYEnd(field[i][j].getYStart() + Cell::edge);
			field[i][j].redrawCell(f);
		}
	}
	wasFirstClick = false;
	showMines(f);
}

void Game::spawnMines(int &curPosX, int &curPosY) {
	int x,
		y;
//		mines = player.getMines();

	while (mines != 0) {
		x = rand() % (width - 1) + 0;
		y = rand() % (height - 1) + 0;
		if (x != curPosX && y != curPosY && field[x][y].getState() != state::mined) {
			field[x][y].setState(state::mined);
			if (x - 1 >= 0) {
				field[x - 1][y - 1].addNearbyMines();
			}
			if (y - 1 >= 0) {
				field[x][y - 1].addNearbyMines();
			}
			if (x + 1 <= width && y - 1 >= 0) {
				field[x + 1][y - 1].addNearbyMines();
			}
			if (x + 1 <= width) {
				field[x + 1][y].addNearbyMines();
			}
			if (x + 1 <= width && y + 1 <= height) {
				field[x + 1][y + 1].addNearbyMines();
			}
			if (y + 1 <= height) {
				field[x][y + 1].addNearbyMines();
			}
			if (x - 1 >= 0 && y + 1 <= height) {
				field[x - 1][y + 1].addNearbyMines();
			}
			if (x - 1 >= 0) {
				field[x - 1][y].addNearbyMines();
			}
			mines--;
		}
	}
}

void Game::showMines(Form ^f) {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (field[i][j].getState() == state::mined && field[i][j].getExtraState() != extraState::flagged) {
				field[i][j].redrawCell(f);
			}
		}
	}
}

void Game::saveGame(bool &started) {
	std::fstream save;
	int time = player.getTime(),
		mines = player.getMines(),
		lifes = player.getMines();

	save.open("Save.sav", std::ios::out | std::ios::trunc | std::ios::binary);
	save.write(reinterpret_cast<char*> (&started), sizeof(bool));
	save.write(reinterpret_cast<char*> (&wasFirstClick), sizeof(bool));
	save.write(reinterpret_cast<char*> (&time), sizeof(int));
	save.write(reinterpret_cast<char*> (&lifes), sizeof(int));
	save.write(reinterpret_cast<char*> (&mines), sizeof(int));
	save.write(reinterpret_cast<char*> (&flags), sizeof(int));
	save.write(reinterpret_cast<char*> (&closedCells), sizeof(int));
	save.write(reinterpret_cast<char*> (&width), sizeof(int));
	save.write(reinterpret_cast<char*> (&height), sizeof(int));
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			save.write(reinterpret_cast<char*> (&field[i][j]), sizeof(GameCell));
		}
	}
	save.close();
}


void Game::loadGame(Form ^f, bool &started) {
	std::fstream load;
	int time = player.getTime(),
		mines = player.getMines(),
		lifes = player.getMines();

	load.open("Save.sav", std::ios::in | std::ios::binary);
	load.read(reinterpret_cast<char*> (&started), sizeof(bool));
	load.read(reinterpret_cast<char*> (&wasFirstClick), sizeof(bool));
	load.read(reinterpret_cast<char*> (&time), sizeof(int));
	load.read(reinterpret_cast<char*> (&lifes), sizeof(int));
	load.read(reinterpret_cast<char*> (&mines), sizeof(int));
	load.read(reinterpret_cast<char*> (&flags), sizeof(int));
	load.read(reinterpret_cast<char*> (&closedCells), sizeof(int));
	load.read(reinterpret_cast<char*> (&width), sizeof(int));
	load.read(reinterpret_cast<char*> (&height), sizeof(int));

	field = new GameCell*[width];

	for (int i = 0; i < width; i++) {
		field[i] = new GameCell[height];
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			load.read(reinterpret_cast<char*> (&field[i][j]), sizeof(GameCell));
			field[i][j].redrawCell(f);
		}
	}
}

void Game::openCell(int &x, int &y, Form ^f, bool &started, int &mb, int &lifes, int &closedCells, bool &wasFirstClick) {
	float xStart = field[0][0].getXStart(),
		xEnd = field[width - 1][height - 1].getXEnd(),
		yStart = field[0][0].getYStart(),
		yEnd = field[width - 1][height - 1].getYEnd();
	int curPosX,
		curPosY;

		if (started == true) {
			if (x > xStart && x<xEnd && y>yStart && y < yEnd) {
				curPosX = (int)((x - xStart) / Cell::edge);
				curPosY = (int)((y - yStart) / Cell::edge);
			
				if (mb == GameCell::mb_open && field[curPosX][curPosY].getExtraState() != extraState::opened && field[curPosX][curPosY].getExtraState() != extraState::flagged && field[curPosX][curPosY].getExtraState() != extraState::undefined) {
					if (wasFirstClick == false) {
						spawnMines(curPosX, curPosY);
						wasFirstClick = true;
				}
				
				if (field[curPosX][curPosY].getState() == state::empty) {
					field[curPosX][curPosY].setExtraState(extraState::opened);
					field[curPosX][curPosY].redrawCell(f);
					closedCells--;
				}
				else {
					flags--;
					mines--;
					field[curPosX][curPosY].setExtraState(extraState::opened);
					field[curPosX][curPosY].redrawCell(f);
					if (lifes == 0) {
						started = false;
					}
					else {
						lifes--;
					}	
				}	
			}
				if (mb == GameCell::mb_flag && field[curPosX][curPosY].getExtraState() != extraState::opened && field[curPosX][curPosY].getExtraState() != extraState::undefined) {
				if (wasFirstClick == false) {
					spawnMines(curPosX, curPosY);
					wasFirstClick = true;
				}
				if (field[curPosX][curPosY].getExtraState() == extraState::flagged) {
					field[curPosX][curPosY].setExtraState(state::empty);
					field[curPosX][curPosY].redrawCell(f);
					flags++;
					if (field[curPosX][curPosY].getState() == state::mined) {
						mines++;
					}
				}
				else {
					if (flags > 0) {
						field[curPosX][curPosY].setExtraState(extraState::flagged);
						field[curPosX][curPosY].redrawCell(f);
						flags--;
						if (field[curPosX][curPosY].getState() == state::mined) {
							mines--;
						}
					}
				}
			}
				if (mb == GameCell::mb_undefined && field[curPosX][curPosY].getExtraState() != extraState::opened && field[curPosX][curPosY].getExtraState() != extraState::flagged) {
					if (wasFirstClick == false) {
						spawnMines(curPosX, curPosY);
						wasFirstClick = true;
				}
				if (field[curPosX][curPosY].getExtraState() == extraState::undefined) {
					field[curPosX][curPosY].setExtraState(extraState::unchecked);
					field[curPosX][curPosY].redrawCell(f);
				}
				else {
					field[curPosX][curPosY].setExtraState(extraState::undefined);
					field[curPosX][curPosY].redrawCell(f);
				}
			}
		}
	}
}
