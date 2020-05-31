#include <stdio.h>
#include <stdlib.h>
#include "rastToVec.h"
#include "checkLinePatt.h"

#define TRUE 1
#define FALSE 0

void drawEdgeV(int xA, int yA, int xB, int yB, int W, int H,
               char shade1, char shade2, char (*raster)[W])
{
   int x, y;
   
   for (y=yA; y < yB; y++)
   for (x=0; x < W; x++)
   if (x < (float)(xB - xA) * (y - yA) / (yB - yA) + xA)
   raster[y][x] = shade1;
   else
   raster[y][x] = shade2;
}

void getEdge(int W, int H, char (*raster)[W])
{
   static int t = 0, m = 0, b = 0;
   int xA, yA, xB, yB, xC, yC;
   
   xA = t; // top
   yA = H;
   
   xB = m % W; // middle
   yB = m / W;
   
   xC = b; // bottom
   yC = 0;
   
   drawEdgeV(xB, yB, xA, yA, W, yA-yB, 0, 1, raster);
   drawEdgeV(xC, yC, xB, yB, W, yB-yC, 0, 1, raster);
   
   t++;
   if (t == W)
   {
      b++;
      t = 0;
   }
   if (b == W)
   {
      m++;
      b = 0;
   }
   if (m == W*H)
   m = 0;
}

void printEdge(int W, int H, char (*raster)[W])
{
   int x, y;
   
   for (y=H-1; y >= 0; y--)
   {
      for (x=0; x < W; x++)
      printf("%i", raster[y][x]);
      
      printf("\n");
   }
   printf("\n");
}

int scanEdge(int W, int H, char (*raster)[W], int offset, int *loc)
{
   int i;
   
   for (i=1; i < W; i++)
   if (raster[offset][i-1] != raster[offset][i])
   {
      *loc = i;
      return 0;
   }
   
   return 1;
}

int traceEdge(int W, int H, char (*raster)[W],
              char *pB, char *cB, int offset, int loc, int *lineSz)
{
   int i;
   char *pattBuff;
   char *checkBuff;
   struct pattern_variables *pv;
   int bitsAllowed;
   int retCheckVal;
   int lineInc = 0;
   int retVal = 0;
   
   pattBuff = pB;
   checkBuff = cB;
   pv = iniPattVars(pattBuff);
   
   for (i=offset+1; i < H; i++)
   {
      *lineSz = lineInc;
      lineInc++;
      
      if ((loc < W) && (raster[i][loc-1] != raster[i][loc]))
      {
         // return if "not a line" and both functions agree
         if (retVal)
         {
            checkBuff[lineInc-1] = 0;
            retCheckVal = checkPattEndBit(checkBuff, lineInc, pv->checkVal, &bitsAllowed);
            
            if ((retCheckVal&IS_STRAIGHT) == FALSE)
            return 0;
         }
         
         retVal = checkLinePatt(0, pv);
         
         checkBuff[lineInc-1] = 0;
         retCheckVal = checkPattEndBit(checkBuff, lineInc, pv->checkVal, &bitsAllowed);
         //printf("%i) bitsAllowed %i | 0 given\n", i-1, bitsAllowed);
         
         // return if both functions disagree on the possible bit values
         if ((retCheckVal&BITS_MATCH) == FALSE)
         return 1;
         
         continue;
      }
      else if ((loc+1 < W) && (raster[i][loc] != raster[i][loc+1]))
      {
         // return if "not a line" and both functions agree
         if (retVal)
         {
            checkBuff[lineInc-1] = 1;
            retCheckVal = checkPattEndBit(checkBuff, lineInc, pv->checkVal, &bitsAllowed);
            
            if ((retCheckVal&IS_STRAIGHT) == FALSE)
            return 0;
         }
         
         retVal = checkLinePatt(1, pv);
         
         checkBuff[lineInc-1] = 1;
         retCheckVal = checkPattEndBit(checkBuff, lineInc, pv->checkVal, &bitsAllowed);
         //printf("%i) bitsAllowed %i | 1 given\n", i-1, bitsAllowed);
         
         // return if both functions disagree on the possible bit values
         if ((retCheckVal&BITS_MATCH) == FALSE)
         return 1;
         
         loc++;
         continue;
      }
      
      break;
   }
   
   return 0;
}

void printPattBuffs(char *pattBuff, char *checkBuff, int size)
{
   int i;
   
   printf("PB:");
   for (i=0; i < size; i++)
   printf("%i", (int)pattBuff[i]);
   printf("\n");
   
   printf("CB:");
   for (i=0; i < size; i++)
   printf("%i", (int)checkBuff[i]);
   printf("\n\n\n");
}

int main(int argc, char **argv)
{
   #define W 20
   #define H 50
   #define OFFSET 5
   int i, loc;
   char raster[H][W];
   char pB[H], cB[H];
   int lineSz;
   
   for (i=0; i < W*W * W*H; i++)
   {
      getEdge(W, H, raster);
      printEdge(W, H, raster); // comment out to speed up computation
      
      if (scanEdge(W, H, raster, OFFSET, &loc))
      continue;
      
      if (traceEdge(W, H, raster, pB, cB, OFFSET, loc, &lineSz))
      {
         printPattBuffs(pB, cB, lineSz-1);
         printf("mismatch at %i\n", i);
         return 0;
      }
      
      printPattBuffs(pB, cB, lineSz-1); // comment out to speed up computation
   }
   
   printf("-- Test Complete -- No Errors Found --\n");
   return 0;
}
