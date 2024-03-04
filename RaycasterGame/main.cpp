#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace olc;
#define M_PI 3.14159265359f
#define ATAN_HALF 0.463647f

constexpr float ANGLE_CHANGE_SPEED = 3.0f;
constexpr float SPEED = 4.0f;
constexpr float MAP_ZOOM = 0.1f;

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
						pPos = mapMousePos;
				}
				else if (GetMouse(0).bPressed)
				{
					if(!((int)mapMousePos.x == (int)pPos.x && (int)mapMousePos.y == (int)pPos.y))
						grid[(int)mapMousePos.y][(int)mapMousePos.x] = !grid[(int)mapMousePos.y][(int)mapMousePos.x];
				}
				else if (GetKey(Key::SHIFT).bHeld)
				{
					if (!((int)mapMousePos.x == (int)pPos.x && (int)mapMousePos.y == (int)pPos.y))
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
				if (mapMousePos.x != pPos.x)
				{
					pAngle = atan2f(mapMousePos.y - pPos.y, mapMousePos.x - pPos.x);
				}
				else
					pAngle = M_PI / 2 * sgn(mapMousePos.y - pPos.y);
			}
		}
	}

	void handle3DInput(float fElapsedTime)
	{
		if (GetKey(Key::LEFT).bHeld)
		{
			pAngle -= ANGLE_CHANGE_SPEED * fElapsedTime;
		}
		if (GetKey(Key::RIGHT).bHeld)
		{
			pAngle += ANGLE_CHANGE_SPEED * fElapsedTime;
		}
		if (GetKey(Key::UP).bHeld)
		{
			if (!grid[int(pPos.y)][int(pPos.x + SPEED * fElapsedTime * cosf(pAngle))])
				pPos.x += SPEED * fElapsedTime * cosf(pAngle);
			if (!grid[int(pPos.y + SPEED * fElapsedTime * sinf(pAngle))][int(pPos.x)])
				pPos.y += SPEED * fElapsedTime * sinf(pAngle);
		}
		if (GetKey(Key::DOWN).bHeld)
		{
			if (!grid[int(pPos.y)][int(pPos.x - SPEED * fElapsedTime * cosf(pAngle))])
				pPos.x -= SPEED * fElapsedTime * cosf(pAngle);
			if (!grid[int(pPos.y - SPEED * fElapsedTime * sinf(pAngle))][int(pPos.x)])
				pPos.y -= SPEED * fElapsedTime * sinf(pAngle);
		}
	}

	float rayLength(vf2d start, float alpha)
	{
		vi2d current = vi2d(start);
		float Sx, Sy;
		float Lx, Ly;

		Sx = 1 / cosf(alpha);
		Sy = 1 / sinf(alpha);

		int Xi = sgn(Sx);
		int Yi = sgn(Sy);

		Sx = abs(Sx);
		Sy = abs(Sy);

		float dx = start.x - floorf(start.x);
		float dy = start.y - floorf(start.y);
		if (Xi == -1)
		{
			dx = 1 - dx;
		}
		if (Yi == -1)
		{
			dy = 1 - dy;
		}

		Lx = Sx * (1 - dx);
		Ly = Sy * (1 - dy);

		bool Xlast = true;

		for (int i = 0; i < 10000; i++)
		{
			if (current.x < 0 || current.y < 0 || current.x >= Wtiles || current.y >= Htiles || grid[current.y][current.x])
				break;
			
			if (Lx < Ly)
			{
				current.x += Xi;
				Lx += Sx;
				Xlast = true;
			}
			else
			{
				current.y += Yi;
				Ly += Sy;
				Xlast = false;
			}
		}

		float Length = (Xlast ? Lx - Sx : Ly - Sy);
		return Length;
	}

	float interp(float n0, float n1, float t)
	{
		return n0 + (n1 - n0) * t;
	}

private:
	std::vector<std::vector<bool>> grid;
	int Width, Height;

	bool showMap;
	int Wtiles;
	int Htiles;
	int mapW;
	int mapH;
	vi2d mapPos;
	float k;
	float mapTileSize;

	vf2d pPos;
	float pAngle;
	float fov;
	
public:
	bool OnUserCreate() override
	{
		showMap = true;
		k = 0.5f;
		Wtiles = 33;
		Htiles = 33;
		fov = rad(90.0f);
		mapTileSize = k * (ScreenWidth() / ScreenHeight() > Wtiles / Htiles ? (float)ScreenHeight() / (float)Htiles : (float)ScreenWidth() / (float)Wtiles);
		mapW = mapTileSize * Wtiles;
		mapH = mapTileSize * Htiles;
		mapPos = { ScreenWidth() - mapW, 0 };
		Width = ScreenWidth();
		Height = ScreenHeight();

		pPos = { 5.5f, 6.0f };
		pAngle = rad(45.0f);

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

		handle3DInput(fElapsedTime);

		if (showMap && GetKey(Key::EQUALS).bHeld && GetKey(Key::SHIFT).bHeld)
		{
			if (k + MAP_ZOOM * fElapsedTime < 1.0f)
			k += MAP_ZOOM * fElapsedTime;
			mapTileSize = k * (ScreenWidth() / ScreenHeight() > Wtiles / Htiles ? (float)ScreenHeight() / (float)Htiles : (float)ScreenWidth() / (float)Wtiles);
			mapW = mapTileSize * Wtiles;
			mapH = mapTileSize * Htiles;
			mapPos = { ScreenWidth() - mapW, 0 };
		}
		if (showMap && GetKey(Key::MINUS).bHeld && !GetKey(Key::SHIFT).bHeld)
		{
			if(k - MAP_ZOOM * fElapsedTime > 0.0f)
				k -= MAP_ZOOM * fElapsedTime;
			mapTileSize = k * (ScreenWidth() / ScreenHeight() > Wtiles / Htiles ? (float)ScreenHeight() / (float)Htiles : (float)ScreenWidth() / (float)Wtiles);
			mapW = mapTileSize * Wtiles;
			mapH = mapTileSize * Htiles;
			mapPos = { ScreenWidth() - mapW, 0 };
		}

		for (int x = 0; x < Width; x++)
		{
			float rayAngle = pAngle + (fov / 2) * atanf(float(x) / float(Width) - 1.0f / 2.0f) / ATAN_HALF;
			if (GetKey(Key::L).bHeld && x % 20 == 0)
			{
				std::cout << x << " " << deg(rayAngle) << "\n";
			}
			float dist = rayLength(pPos, rayAngle);
			dist *= cosf(pAngle - rayAngle);

			int wallHeight = std::min(int(1 / dist * (float)Height), Height);
			float lightingFactor = 0.8f * std::min(5.0f / (dist+4.0f), 1.0f) + 0.2;
			float transparencyFactor = std::min(std::max((dist - 7.0f) / 8.0f, 0.0f), 1.0f);
			DrawLine(x, (Height - wallHeight) / 2, x, (Height + wallHeight) / 2, Pixel(interp(255 * lightingFactor,255, transparencyFactor), interp(25 * lightingFactor, 255, transparencyFactor), interp(25 * lightingFactor, 255, transparencyFactor)));
			//FillRect(100, 100, 100, 100, Pixel(100, 240, 100, 20));
		}

		if (GetKey(Key::M).bPressed)
			showMap = !showMap;
		if (showMap)
		{
			handleMapInput();

			FillRect(mapPos, vi2d(mapW, mapH), WHITE);
			DrawWalls(BLUE);
			DrawGrid(BLACK);
			vi2d globalPlayerPos = mapPos + pPos * mapTileSize;
			DrawLine(globalPlayerPos, globalPlayerPos + vi2d(40.0 * cosf(pAngle-fov/2), 40.0f * sinf(pAngle-fov/2)), BLACK);
			DrawLine(globalPlayerPos, globalPlayerPos + vi2d(40.0 * cosf(pAngle), 40.0f * sinf(pAngle)), BLACK);
			DrawLine(globalPlayerPos, globalPlayerPos + vi2d(40.0 * cosf(pAngle+fov/2), 40.0f * sinf(pAngle+fov/2)), BLACK);
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