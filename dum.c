#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

struct Point {
   int x;
   int y;
};

typedef struct entry {
  Point *point;			/* entry=step*/
  struct entry *next, *prev;	/* next and previous entry */
} Entry;

struct Point *foo(int x, int y);
void foo2();

main()
{
    struct Point *point = foo(8888,8888);
    struct Point * points[UINT_MAX]; 
    int i; 
    // for (i=0; i<20; i++)
    // {
    // 	points[i] = foo(i,i*i);
    // 	printf("point = (%d, %d)\n", points[i]->x, points[i]->y);
    // }
    // foo2(foo(2,4));
    // printf("\n");
    // foo2(foo(3,5));
    // printf("\n");
    // foo2(foo(4,6));

    // printf("point = (%d, %d)\n", point->x, point->y);
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

void foo2( struct Point * point)
{
	static struct Point * trial[10];
	static int i = 0; 
	int track;
	trial[i] = point; 
	i++;
	track = i; 
	int j=0;
	for (j=0; j<track; j++){
		printf("point = (%d, %d)\n", trial[j]->x, trial[j]->y);
	}
}