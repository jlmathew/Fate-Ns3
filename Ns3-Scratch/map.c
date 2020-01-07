#include <stdio.h>

int main()
{
   int x,y;
   for (y=0; y<5; y++)
   {
    for(x=0; x<5; x++) {
      if (x !=4)	    
        printf("%d %d \n", x+y*5, x+1+y*5);
      if (y != 4)
        printf("%d %d \n", x+y*5, x+5+y*5);
    }
   }

}
