#include <fstream>
#include <cstdio>
#include <iostream>
#include <list>

struct myEvent {
	float time;
	int thing;
};

using namespace std;

list<myEvent> myList;

constexpr auto dimx = 256u, dimy = 256u;

float tMatrix[3][3] = { { 1,0,0 }, { 0,1,0 }, { 0,0,1 } };

unsigned char red[4] = { 240u,0u,0u,255u };
unsigned char green[4] = { 0u, 240u,0u,255u };
unsigned char blue[4] = { 0u, 0u,240u,255u };
unsigned char white[4] = { 255u, 255u,255u,255u };
unsigned char gray[4] = { 255u, 255u,255u,255u };
unsigned char purple[4] = { 255u, 0u,255u,255u };

void transform(unsigned int vec[3]) {


}

void setTranslate(int x, int y)
{
}

void setRotate(float z)
{

}

unsigned char imageBuff[dimx][dimy][4];

unsigned int vertexPositions[] = {
	0, 0,
	0, 32,
	32, 0,
	32, 32,
	64, 0,
	64, 32,
	96, 0,
	96, 32,
	128, 0,
	128, 32
};

void drawPixel(int x, int y, const unsigned char* pixColor)
{
	if (pixColor[3] == 255u) {
		imageBuff[x][y][0] = pixColor[0];
		imageBuff[x][y][1] = pixColor[1];
		imageBuff[x][y][2] = pixColor[2];
	}
	else {
	}
}

void drawline(int x0, int y0, int x1, int y1, const unsigned char* newColor)
{

}

void generateLines()
{

	drawline(1, 32, 62, 32, white);
	drawline(1, 31, 62, 31, gray);
	drawline(32, 1, 32, 62, white);
	drawline(31, 1, 31, 62, gray);

	drawline(1, 1, 62, 62, white);

	drawline(1, 2, 61, 62, gray);
	//drawline( 2, 1, 62, 61, gray);
	drawline(62, 61, 2, 1, gray); // same, flipped (high to low)


	drawline(62, 1, 1, 62, white);

	//drawline(1, 61, 61, 1, gray);
	drawline(61, 1, 1, 61, gray);
	//drawline(62, 2, 2, 62, gray); 
	drawline(2, 62, 62, 2, gray); // same, flipped

	drawline(1, 32, 63, 1, green);
	drawline(1, 32, 63, 63, blue);

	drawline(1, 32, 16, 1, red);
	drawline(1, 32, 16, 63, green);

	drawline(63, 32, 32, 1, blue);
	drawline(63, 32, 32, 63, red);

	drawline(32, 32, 16, 1, green);
	drawline(32, 32, 16, 63, blue);
	drawline(32, 32, 16, 63, green);

	drawline(63, 63, 63, 63, purple); //  test drawing a point

	for (int i = 0; i < 10; i++)
		drawline(rand() % 64, rand() % 64, rand() % 64, rand() % 64, purple);
}

void drawTriangle(unsigned int vert[])
{

	unsigned char white[4] = { 255u, 255u,255u,255u };
}
void drawTriangles(unsigned int vert[], int numTriangles)
{
	unsigned char white[4] = { 255u, 255u,255u,255u };

}
void drawTriangleStrip(unsigned int vert[], int numTriangles)
{
	unsigned char white[4] = { 255u, 255u,255u,255u };
}



void bezierCurve(int x[], int y[])
{

}

int myX[] = { 10, 32, 48, 64 };
int myY[] = { 48, 10, 10, 64 };

int myTexture() 
{
	memset(imageBuff, 0, sizeof(imageBuff));

	bezierCurve(myX, myY);

	generateLines();

	setTranslate(128, 64);

	drawTriangles(vertexPositions, 3);

	setTranslate(128, 96);

	drawTriangleStrip(vertexPositions, 6);

	setRotate(3.1415 / 6.0);

	drawTriangleStrip(vertexPositions, 6);
	setRotate(3.1415926 / 3.0);

	drawTriangleStrip(vertexPositions, 6);

	for (int i = 0; i < dimx; i++)
		for (int j = 0; j < dimy; j++)
		{
			if (((i / 16) % 2) == 0)
			{
				if (((j / 16) % 2) == 0)
					imageBuff[i][j][0] = imageBuff[i][j][1] = imageBuff[i][j][2] = 255;
			}
			else {
				if (((j / 16) % 2) == 1)
					imageBuff[i][j][0] = imageBuff[i][j][1] = imageBuff[i][j][2] = 255;
			}
		}

	return 0;
}