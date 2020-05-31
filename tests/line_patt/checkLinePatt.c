#include <stdio.h>
#include <stdlib.h>
#include "checkLinePatt.h"

#define TRUE 1
#define FALSE 0

int isStraightLinePatt(char *patt, int pattSz)
{
   int i, j;
   int offset;
   int pattern_offset_fits;
   char *new_patt;
   int xP, yP;
   int xPrior;
   int isStraight;
   int x;
   
   // format pattern
   offset = pattSz;
   
   for (j=1; j < pattSz; j++) // find offset
   {
      pattern_offset_fits = TRUE;
      
      for (i=j; i < pattSz; i++)
      if (patt[i] != patt[i-j])
      {
         pattern_offset_fits = FALSE;
         break;
      }
      
      if (pattern_offset_fits)
      {
         offset = j;
         break;
      }
   }
   
   
   new_patt = (char *)malloc(offset);
   
   for (i=0; i < offset; i++) // make loopable
   new_patt[i] = patt[i];
   
   
   // get the x and y coordinates
   xP = 0;
   yP = offset;
   for (i=0; i < offset; i++)
   if (new_patt[i] == 1)
   xP += 1;
   
   // determine if the pattern can build a straight line
   for (j=1; j < yP+1; j++)
   {
      xPrior = (int)((double)xP * (j-1) / yP);
      isStraight = TRUE;
      
      for (i=0; i < yP; i++)
      {
         x = (int)((double)xP * (j+i) / yP);
         
         if (x - xPrior != new_patt[i])
         {
            isStraight = FALSE;
            break;
         }
         
         xPrior = x;
      }
      
      if (isStraight)
      {
         free(new_patt);
         return 1;
      }
   }
   
   free(new_patt);
   return 0;
}

int checkPattEndBit(char *patt, int pattSz, int endBit, int *bitsAllowed)
{
   int saveBit;
   int ret = 0;
   
   *bitsAllowed = NONE;
   
   saveBit = patt[pattSz-1];
   
   patt[pattSz-1] = 1; // Try 1
   
   if (isStraightLinePatt(patt, pattSz))
   *bitsAllowed = 1;
   
   patt[pattSz-1] = 0; // Try 0
   
   if (isStraightLinePatt(patt, pattSz))
   {
      if (*bitsAllowed == 1)
      *bitsAllowed = ANY; // either bit can make a straight line
      else
      *bitsAllowed = 0;
   }
   
   patt[pattSz-1] = saveBit; // return state
   
   if (endBit == *bitsAllowed)
   ret = BITS_MATCH;
   
   if (*bitsAllowed != NONE)
   ret += IS_STRAIGHT;
   
   return ret;
}
