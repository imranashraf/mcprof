#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

typedef int TYPE;

TYPE coeff = 2;
int nElem;

using namespace std;

class Coordinate
{
  public:
    int x, y;

    Coordinate() {}
    Coordinate(int px, int py)
      : x(px), y(py)
    {
    }
};

class pixel
{
    public:
        Coordinate pos;
        float value;

    bool operator < (const pixel& pix) const
    {
        return (value > pix.value);
    }
};

void initCoordinates(vector<Coordinate>& coordinates)
{
    for (unsigned int i=0; i<nElem; i++)
    {
        coordinates[i].x = i+1;
        coordinates[i].y = i+2;
    }
}

void initPixels(vector<pixel>& pixels)
{
    for (unsigned int i=0; i<nElem; i++)
    {
        pixels[i].pos.x = i+1;
        pixels[i].pos.y = i+2;
        pixels[i].value = i+3.0;
    }
}

void printPixels(vector<pixel>& pixels)
{
    for (unsigned int i=0; i<nElem; i++)
    {
        printf("(%d,%d) = %f\n",pixels[i].pos.x, pixels[i].pos.y, pixels[i].value);
    }
}

void sortPixels(vector<pixel>& pixels1)
{
    sort(pixels1.begin(), pixels1.end());
}

void printCoordinates(vector<Coordinate>& coordinates)
{
    for (unsigned int i=0; i<nElem; i++)
    {
        printf("(%d,%d)\n",coordinates[i].x, coordinates[i].y);
    }
}

void func(vector<Coordinate>& coordinates)
{
    printCoordinates(coordinates);

#if 1
    vector<Coordinate> coordinates2(nElem);
#else
    vector<Coordinate> coordinates2;
    coordinates2.resize(nElem);
#endif

#if 1
    coordinates2 = coordinates;
#else
    for (unsigned int i=0; i<nElem; i++)
    {
        coordinates2[i].x = coordinates[i].x;
        coordinates2[i].y = coordinates[i].y;
    }
#endif

    printCoordinates(coordinates2);
}

int main()
{
    nElem = 10;
    printf("Vector Operations Test.\n");

#if 1
    vector<Coordinate> coordinates1(nElem);
#else
    vector<Coordinate> coordinates1;
    coordinates1.resize(nElem);
#endif

    initCoordinates(coordinates1);
    printCoordinates(coordinates1);
    func(coordinates1);

    vector<pixel> pixels1(nElem);
    initPixels(pixels1);
    printPixels(pixels1);
    sortPixels(pixels1);
    printPixels(pixels1);

    printf("End Vector Operations\n");

    return 0;
}
