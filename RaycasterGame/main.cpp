#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace olc;
#define M_PI 3.14159265359f

// Override base class with your custom functionality
class Raycaster : public olc::PixelGameEngine
{
public:
	Raycaster()
	{
		sAppName = "Raycaster";
	}

private:
	float rad(float deg)
	{
		return M_PI * deg / 180.0f;
	}

	float deg(float rad)
	{
		return 180.0f * rad / M_PI;
	}

	int sgn(float n)
	{
		return (n < 0 ? -1 : 1);
	}

	void DrawGrid(Pixel color)
	{
		for (float x = 0; x <= float(mapW)+0.01f; x += mapTileSize)
			DrawLine((int)x+mapPos.x, mapPos.y, (int)x+mapPos.x, mapH+mapPos.y, color);

		for (float y = 0; y <= float(mapH)+0.01f; y += mapTileSize)
			DrawLine(mapPos.x, (int)y+mapPos.y, mapW + mapPos.x, (int)y + mapPos.y, color);
	}

	void DrawWalls(Pixel color)
	{
		float x = 0.0f;
		for (int ix = 0; ix < Wtiles; x += mapTileSize, ix++)
		{
			float y = 0.0f;
			for (int iy = 0; iy < Htiles; y += mapTileSize, iy++)
			{
				if (grid[iy][ix])
				{
					vi2d pos = { (int)x, (int)y };
					FillRect(mapPos + pos, vi2d(mapTileSize+1, mapTileSize+1), color);
				}
			}
		}
	}

	void handleMapInput()
	{
		if (GetMouse(0).bHeld)
		{
			vf2d mapMousePos = { float(GetMouseX() - mapPos.x), float(GetMouseY() - mapPos.y) };
			mapMousePos /= mapTileSize;

			if (mapMousePos.x > 0 && mapMousePos.x < Wtiles && mapMousePos.y > 0 && mapMousePos.y < Htiles)
			{
				if (GetKey(Key::CTRL).bHeld)
				{
					if (!grid[(int)mapMousePos.y][(int)mapMousePos.x])
						start = mapMousePos;
				}
				else if (GetMouse(0).bPressed)
				{
					grid[(int)mapMousePos.y][(int)mapMousePos.x] = !grid[(int)mapMousePos.y][(int)mapMousePos.x];
				}
				else if (GetKey(Key::SHIFT).bHeld)
				{
					grid[(int)mapMousePos.y][(int)mapMousePos.x] = (!GetKey(Key::Z).bHeld);
				}
			}
		}
		if (GetMouse(1).bHeld)
		{
			vf2d mapMousePos = { float(GetMouseX() - mapPos.x), float(GetMouseY() - mapPos.y) };
			mapMousePos /= mapTileSize;

			if (mapMousePos.x > 0 && mapMousePos.x < Wtiles && mapMousePos.y > 0 && mapMousePos.y < Htiles)
			{
				if (mapMousePos.x != start.x)
				{
					alpha = atan2f(mapMousePos.y - start.y, mapMousePos.x - start.x);
				}
				else
					alpha = M_PI / 2 * sgn(mapMousePos.y - start.y);
			}
		}
	}

private:
	std::vector<std::vector<bool>> grid;

	bool showMap;
	int Wtiles;
	int Htiles;
	int mapW;
	int mapH;
	vi2d mapPos;
	float k;
	float mapTileSize;

	vf2d start;
	float alpha;
	float fov;
	
public:
	bool OnUserCreate() override
	{
		showMap = true;
		k = 0.35;
		Wtiles = 11;
		Htiles = 11;
		mapTileSize = k * (ScreenWidth() / ScreenHeight() > Wtiles / Htiles ? (float)ScreenHeight() / (float)Htiles : (float)ScreenWidth() / (float)Wtiles);
		mapW = mapTileSize * Wtiles;
		mapH = mapTileSize * Htiles;
		mapPos = { ScreenWidth() - mapW, 0 };

		start = { 5.5f, 6.0f };
		alpha = rad(45.0f);

		grid.resize(Htiles);
		for (int i = 0; i < Htiles; i++)
		{
			grid[i].resize(Wtiles);
		}
		for (int i = 0; i < Htiles; i++)
			for (int j = 0; j < Wtiles; j++)
				if (i == 0 || i == Htiles - 1 || j == 0 || j == Wtiles - 1)
					grid[i][j] = true;

		std::cout << mapTileSize << "\n";

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(WHITE);

		//3d stuff
		
		if (GetKey(Key::M).bPressed)
			showMap = !showMap;
		if (showMap)
		{
			handleMapInput();

			DrawWalls(BLUE);
			DrawGrid(BLACK);
			vi2d globalPlayerPos = mapPos + start * mapTileSize;
			DrawLine(globalPlayerPos, globalPlayerPos + vi2d(40.0 * cosf(alpha), 40.0f * sinf(alpha)), BLACK);
			FillCircle(globalPlayerPos, 2, RED);
		}

		return true;
	}
};

int main()
{
	Raycaster game;
	if (game.Construct(600, 300, 2, 2))
		game.Start();
	return 0;
}