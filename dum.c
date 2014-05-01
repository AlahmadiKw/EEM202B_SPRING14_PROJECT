#include <stdio.h>
#include <string.h>
#include <limits.h>

struct Point {
   int x;
   int y;
};

struct Point *foo(int x, int y);

main()
{
    struct Point *point = foo(8888,8888);
    struct Point * points[UINT_MAX]; 
    int i; 
    for (i=0; i<20; i++)
    {
    	points[i] = foo(i,i*i);
    	printf("point = (%d, %d)\n", points[i]->x, points[i]->y);
    }

    printf("point = (%d, %d)\n", point->x, point->y);
}

struct Point *foo(int x, int y)
{
	struct Point *point;
    if ((point = malloc (sizeof(*point))) == NULL) {
        printf ("\n\n*** ERROR: fail allocating memory...\n\n");
        perror ("\n\n*** ERROR: fail allocating memory...\n\n");
    }
	point->x = x; 
	point->y = y;
	return point;
}