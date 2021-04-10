#include <iostream>
#include <unistd.h>
#include <cmath>

const int ScreenWidth = 211, ScreenHeight = 53, FPS = 10;

class Map
{
	char *list = NULL;
	int rowSize, colSize;
	int _pos;
public:
	Map(int size1, int size2): rowSize(size1), colSize(size2), _pos(0)
	{
		list = new char[size1 * size2];
	}
	Map(const Map &m)
	{
		rowSize = m.rowSize;
		colSize = m.colSize;
		_pos = m._pos;
		list = new char[rowSize * colSize];
		for (int i = 0; i < colSize; i++)
			for (int j = 0; j < rowSize; j++)
				list[i * rowSize + j] = m.list[i * rowSize + j];
	}
	Map& operator+=(const char *str)
	{
		for (int i = 0; i < rowSize; i++)
			list[_pos++] = str[i];
		return *this;
	}
	char& get(float row, float col)
	{
		return list[(int)row * rowSize + (int)col];
	}
	~Map()
	{
		delete[] list;
	}
};

class Scene
{
	Map map;
	int MapWidth, MapHeight;
	char *screen;
	int ScreenWidth, ScreenHeight;
	float playerX, playerY, playerA;
	float FOV = 3.14159 / 3.0; // field of view
	float Depth = 16.0;
	float Speed = 0.4;
public:
	Scene(int screenWidth, int screenHeight, int mapWidth, int mapHeight, Map m): map(m)
	{
		ScreenWidth  = screenWidth;
		ScreenHeight = screenHeight;
		MapWidth  = mapWidth;
		MapHeight = mapHeight;
		screen = new char[ScreenWidth * ScreenHeight + 1];
		screen[ScreenWidth * ScreenHeight] = '\0';
		playerX = playerY = 1.5;
		playerA = 0.0; // Angle
	}
	void draw() // Ray casting
	{
		for (int x = 0; x < ScreenWidth; x++)
		{
			float rayA = (playerA - FOV / 2.0) + ((float) x / ScreenWidth * FOV);
			float step = 0.1;
			float distToWall = 0.0;
			bool isHitWall = false;
			bool isEdge = false;
			while (!isHitWall && distToWall < Depth)
			{
				distToWall += step;
				int rayX = (int)(playerX + cos(rayA) * distToWall);
				int rayY = (int)(playerY + sin(rayA) * distToWall);
				if (rayX < 0 || rayX > MapWidth || rayY < 0 || rayX > MapHeight)
				{
					isHitWall = true;
					distToWall = Depth;
				}
				else if (map.get(rayY, rayX) == '=')
				{
					isHitWall = true;
					float d1, d2;
					float vx1,vy1, vx2, vy2;
					d1 = d2 = -1;
					for (int tx = 0; tx < 2; tx++)
						for (int ty = 0; ty < 2; ty++)
						{
							float vx = (float) rayX - playerX + tx;
							float vy = (float) rayY - playerY + ty;
							if (d1 < 0 || (sqrt(vx*vx + vy*vy) < d1 && d2 > 0 && d1 > d2))
							{
								d1 = sqrt(vx*vx + vy*vy);
								vx1 = vx;
								vy1 = vy;
							}
							else if (d2 < 0 || sqrt(vx*vx + vy*vy) < d2)
							{
								d2 = sqrt(vx*vx + vy*vy);
								vx2 = vx;
								vy2 = vy;
							}
						}
					float maxAngle = 0.005;
					if (acos(cos(rayA) * vx1 / d1 + sin(rayA) * vy1 / d1) < maxAngle || acos(cos(rayA) * vx2 / d2 + sin(rayA) * vy2 / d2) < maxAngle)
						isEdge = true;
				}
			}
			int y1 = (ScreenHeight / 2.0) - ScreenHeight / (distToWall);
			int y2 = ScreenHeight - y1;
			char wall = ' ';
			if (isEdge)
				wall = '|';
			else if (distToWall <= Depth / 4.0)
				wall = '0';
			else if (distToWall <= Depth / 3.0)
				wall = '1';
			else if (distToWall <= Depth / 2.0)
				wall = '2';
			else if (distToWall < Depth / 1.0)
				wall = '3';

			for (int y = 0; y < ScreenHeight; y++)
			{
				if (y <= y1)
					screen[y * ScreenWidth + x] = ' ';
				else if (y > y1 && y < y2)
					screen[y * ScreenWidth + x] = wall;
				else
				{
					float d = ((float) y - ScreenHeight / 2.0) / ((float) ScreenHeight / 2.0);
					if (d > 0.75)
						screen[y * ScreenWidth + x] = '#';
					else if (d > 0.5)
						screen[y * ScreenWidth + x] = 'x';
					else if (d > 0.25)
						screen[y * ScreenWidth + x] = '-';
					else if (d > 0.1)
						screen[y * ScreenWidth + x] = '.';
					else
						screen[y * ScreenWidth + x] = ' ';
				}
			}
		}
		insertMap(0, 0);
	}
	void print()
	{
		system("clear");
		std::cout << "X = " << playerX << " Y = " << playerY << " Angle = " << playerA << std::endl;
		for (int i = 0; i < ScreenHeight; i++)
		{
			for (int j = 0; j < ScreenWidth; j++)
			{
				char c = screen[i * ScreenWidth + j];
				if (c == '0')
					std::cout << "█";
				else if (c == '1')
					std::cout << "▓";
				else if (c == '2')
					std::cout << "▒";
				else if (c == '3')
					std::cout << "░";
				else
					std::cout << c;
			}
			std::cout << std::endl;
		}
		std::cout << "> Enter some command (w, s, a, d, q - quit): ";
		flush(std::cout);
	}
	void updPos(char cmd)
	{
		if (cmd == 'w')
		{
			playerX += cos(playerA) * Speed;
			playerY += sin(playerA) * Speed;
			if (map.get(playerY, playerX) == '=')
			{
				playerX -= cos(playerA) * Speed;
				playerY -= sin(playerA) * Speed;
			}
		}
		else if (cmd == 's')
		{
			playerX -= cos(playerA) * Speed;
			playerY -= sin(playerA) * Speed;
			if (map.get(playerY, playerX) == '=')
			{
				playerX += cos(playerA) * Speed;
				playerY += sin(playerA) * Speed;
			}
		}
		else if (cmd == 'a')
			playerA -= 0.1;
		else if (cmd == 'd')
			playerA += 0.1;
	}
	void insertMap(int x, int y)
	{
		for (int i = y; i < MapHeight + y; i++)
			for (int j = x; j < MapWidth + x; j++)
				screen[i * ScreenWidth + j] = map.get(i - y, j - x);
		screen[(int)((int)(playerY) * ScreenWidth + (int)(playerX))] = 'X';
	}
};

int main()
{
	char cmd = ' ';
	const int MapWidth  = 16;
	const int MapHeight = 16;
	Map map(MapWidth, MapHeight);
	map += "================";
	map += "=..............=";
	map += "===========....=";
	map += "=..............=";
	map += "=..............=";
	map += "=..............=";
	map += "=......==......=";
	map += "=......==......=";
	map += "=..............=";
	map += "=..............=";
	map += "=..............=";
	map += "=.....=..=======";
	map += "=.....=........=";
	map += "=.....=........=";
	map += "=.....=........=";
	map += "================";

	Scene S(ScreenWidth, ScreenHeight, MapWidth, MapHeight, map);
	while (cmd != 'q')
	{
		S.draw();
		S.print();
		usleep((1.0 / 10) * 1e6);
		std::cin >> cmd;
		S.updPos(cmd);
	}
	return 0;
}
