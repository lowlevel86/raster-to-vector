/*
   Copyright 2020 Crailin Mogged

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include "rastToVec.h"


struct pattern_variables *iniPattVars(char *buff)
{
   static struct pattern_variables pv;

   pv.buff = buff;
   pv.buffInc = 0;
   pv.ptrA = 0;
   pv.ptrB = 0;
   pv.cntA = 0;
   pv.cntB = 0;
   pv.incA = 0;
   pv.incB = 0;
   pv.multInc = 0;
   pv.multPrior = 0;
   pv.multH1 = 0;
   pv.multH2 = 0;
   pv.multA = 0;
   pv.multB = 0;
   pv.multOffset = 0;
   pv.identiSegInc = 0;
   pv.flipFlopSegInc = 0;
   pv.checkVal = 2; // 2 == any bit value; -1 == none
   
   return &pv;
}


void recalcBitPattVars(struct pattern_variables *pv)
{
   int bitCntA, bitCntB;
   
   bitCntA = pv->multA * pv->cntA + pv->cntB;
   bitCntB = pv->multB * pv->cntA + pv->cntB;
   pv->ptrA = pv->buffInc+1 - (bitCntA + bitCntB);
   pv->ptrB = pv->buffInc+1 - bitCntB;
   pv->cntA = bitCntA;
   pv->cntB = bitCntB;
}


void accOffset(struct pattern_variables *pv) // account offset
{
   if (pv->flipFlopSegInc == 0)
   {
      pv->multOffset = pv->identiSegInc + 1;
      pv->multInc = 0;
      pv->multH1 = 0;
      pv->multH2 = 0;
      pv->flipFlopSegInc = 0;
   }
   else if (pv->flipFlopSegInc == 1)
   {
      if (pv->multOffset)
      pv->multOffset = 0;
      else
      pv->multOffset = 1;
      
      pv->multInc = pv->identiSegInc + 1;
      pv->multH1 = pv->multInc;
      pv->multH2 = 0;
      pv->flipFlopSegInc = 0;
   }
   else if (pv->flipFlopSegInc == 2)
   {
      if (pv->multOffset)
      pv->multOffset = 1;
      else
      pv->multOffset = 0;
      
      pv->multInc = 2;
      pv->multH1 = 1;
      pv->multH2 = 2;
      pv->flipFlopSegInc = 1;
   }
   else if (pv->flipFlopSegInc > 2)
   {
      pv->multA = 1;
      pv->multB = 2;
      recalcBitPattVars(pv);
      pv->multOffset = pv->flipFlopSegInc - 1;
      pv->multInc = 0;
      pv->multH1 = 0;
      pv->multH2 = 0;
      pv->flipFlopSegInc = 0;
   }
   
   pv->multA = 0;
   pv->multB = 0;
   pv->identiSegInc = 0;
}


void recalcPattVars(struct pattern_variables *pv)
{
   // account offsets that represent current largest possible
   if (pv->multInc == pv->multOffset-1)
   {
      if (pv->multH1 == 0)
      {
         pv->multH1 = pv->multOffset;
         pv->multOffset = 0;
      }
      else if (pv->multH2 == 0)
      {
         pv->multH1 = pv->multOffset;
         pv->multH2 = pv->multInc;
         pv->multA = pv->multOffset;
         pv->identiSegInc += 1;
      }
   }
   
   // account multiplier holders
   if (pv->multH1 == 0)
   pv->multH1 = pv->multInc;
   else if (pv->multH2 == 0)
   {
      if (pv->multInc != pv->multH1)
      pv->multH2 = pv->multInc;
   }
   
   // account both multA/B
   if (pv->multA == 0)
   {
      if (pv->multInc == pv->multPrior)
      pv->multA = pv->multInc;
   }
   else
   {
      if (pv->multInc != pv->multA)
      {
         pv->multB = pv->multInc;
         recalcBitPattVars(pv);
      }
   }
   
   if (pv->multB)
   accOffset(pv);
   else
   {
      // keep track of identical segments
      if (pv->multInc == pv->multPrior)
      pv->identiSegInc += 1;
      
      // keep track of the flip-flop pattern size
      if ((pv->multInc == pv->multH2) && (pv->multInc != pv->multPrior))
      pv->flipFlopSegInc += 1;
   }
}


void growPattModel(int bitVal, struct pattern_variables *pv)
{
   // find if the pattern matches 'A' or 'B'
   if (bitVal == pv->buff[pv->ptrA+pv->incA])
   {
      if (pv->incA != -1)
      pv->incA += 1;
   }
   else
   pv->incA = -1; // does not match
   
   if (bitVal == pv->buff[pv->ptrB+pv->incB])
   {
      if (pv->incB != -1)
      pv->incB += 1;
   }
   else
   pv->incB = -1; // does not match
   
   if (pv->incA == pv->cntA) // if the pattern matches 'A'
   {
      pv->multInc += 1;
      pv->incA = 0;
      pv->incB = 0;
   }
   
   if (pv->incB == pv->cntB) // if the pattern matches 'B'
   {
      recalcPattVars(pv);
      pv->multPrior = pv->multInc;
      pv->multInc = 0;
      pv->incA = 0;
      pv->incB = 0;
   }
   
   pv->buff[pv->buffInc] = bitVal;
   pv->buffInc += 1;
}


void resolveDuoBitVal(struct pattern_variables *pv)
{
   if ((pv->incA != -1) && (pv->incB != -1))
   {
      if (pv->buff[pv->ptrA+pv->incA] == pv->buff[pv->ptrB+pv->incB])
      pv->checkVal = pv->buff[pv->ptrA+pv->incA];
      else
      pv->checkVal = 2;
   }
   else if (pv->incA != -1)
   pv->checkVal = pv->buff[pv->ptrA+pv->incA];
   else if (pv->incB != -1)
   pv->checkVal = pv->buff[pv->ptrB+pv->incB];
}

void duoPattCheck(struct pattern_variables *pv, int smMult, int lgMult)
{
   if (pv->multInc == smMult)
   resolveDuoBitVal(pv);
   else if (pv->multInc == lgMult)
   pv->checkVal = pv->buff[pv->ptrB+pv->incB];
   else if (pv->multInc < smMult)
   pv->checkVal = pv->buff[pv->ptrA+pv->incA];
}

void trioPattCheck(struct pattern_variables *pv, int smMult, int medMult, int lgMult)
{
   if (pv->multInc == smMult)
   resolveDuoBitVal(pv);
   else if (pv->multInc == medMult)
   resolveDuoBitVal(pv);
   else if (pv->multInc == lgMult)
   pv->checkVal = pv->buff[pv->ptrB+pv->incB];
   else if (pv->multInc < smMult)
   pv->checkVal = pv->buff[pv->ptrA+pv->incA];
}

void flipFlopPattCheck(struct pattern_variables *pv, int mult1, int mult2)
{
   if (pv->multA == mult1)
   {
      if (pv->multInc == mult2)
      pv->checkVal = pv->buff[pv->ptrB+pv->incB];
      else if (pv->multInc < mult2)
      pv->checkVal = pv->buff[pv->ptrA+pv->incA];
   }
   else if (pv->multA == mult2)
   {
      if (pv->multInc == mult1)
      pv->checkVal = pv->buff[pv->ptrB+pv->incB];
      else if (pv->multInc < mult1)
      pv->checkVal = pv->buff[pv->ptrA+pv->incA];
   }
}

int checkBit(int bitVal, struct pattern_variables *pv)
{
   pv->checkVal = -1;

   if ((pv->incA == -1) && (pv->incB == -1))
   return 1;
   
   if ((pv->flipFlopSegInc >= 2) && (pv->multA)) // end of flip-flop pattern
   {
      flipFlopPattCheck(pv, pv->multH1, pv->multH2);
   }
   else if (pv->multH2) // check for 2 possible patterns
   {
      if (pv->multH1 == pv->multH2-1) // multH1 < multH2
      duoPattCheck(pv, pv->multH1, pv->multH2);
      else if (pv->multH2 == pv->multH1-1) // multH2 < multH1
      duoPattCheck(pv, pv->multH2, pv->multH1);
   }
   else if ((pv->multH1) && (pv->multH2 == 0)) // check for 3 possible patterns
   {
      trioPattCheck(pv, pv->multH1-1, pv->multH1, pv->multH1+1);
   }
   else if (pv->multH1 == 0) // pattern size is unknown
   {
      if (pv->multInc >= pv->multOffset-1)
      resolveDuoBitVal(pv);
      else if (pv->multInc < pv->multOffset-1)
      pv->checkVal = pv->buff[pv->ptrA+pv->incA];
   }
   
   if (bitVal == pv->checkVal)
   return 0;
   
   if (pv->checkVal == 2)
   return 0;
   
   return 1;
}


void iniGrowPattModel(int bitVal, struct pattern_variables *pv)
{
   // find the bit value that occurs more often
   if (pv->cntA == 0)
   {
      if ((pv->buffInc) && (bitVal == pv->buff[pv->buffInc-1]))
      {
         pv->ptrA = pv->buffInc-1;
         pv->cntA = 1;
         pv->flipFlopSegInc = pv->buffInc >> 1;
      }
   }
   else if (pv->cntB == 0) // find the other bit
   {
      if (bitVal != pv->buff[pv->ptrA])
      {
         pv->ptrB = pv->buffInc;
         pv->cntB = 1;
         
         // find offset
         if (pv->flipFlopSegInc == 0) // 00...1
         {
            pv->multOffset = pv->buffInc;
         }
         else if (pv->flipFlopSegInc == 1) // 0100...1
         {
            if (pv->buff[0] == pv->buff[pv->ptrA]) // 0100...1 else 100...1
            pv->multOffset = 1;
            
            pv->multPrior = pv->ptrB - pv->ptrA;
            pv->multH1 = pv->multPrior;
            pv->flipFlopSegInc = 0;
         }
         else if (pv->flipFlopSegInc == 2) // if flip-flop Pattern 0101001
         {
            if (pv->buff[0] == pv->buff[pv->ptrA]) // 0101001 else 101001
            pv->multOffset = 1;
            
            pv->multPrior = 2;
            pv->multH1 = 1;
            pv->multH2 = 2;
            pv->flipFlopSegInc = 1;
         }
         else if (pv->flipFlopSegInc > 2) // if flip-flop Pattern 010101...001
         {
            pv->ptrA -= 2; pv->ptrB -= 2;
            pv->cntA = 2; pv->cntB = 3;
            pv->multOffset = pv->flipFlopSegInc - 1;
            pv->flipFlopSegInc = 0;
         }
      }
   }

   pv->buff[pv->buffInc] = bitVal;
   pv->buffInc += 1;
}

int iniCheckBit(int bitVal, struct pattern_variables *pv)
{
   pv->checkVal = 2;
   
   // find the possible bit values for flip-flop patterns
   if (pv->flipFlopSegInc >= 2)
   {
      if (pv->buff[pv->buffInc-2] == pv->buff[pv->ptrA])
      pv->checkVal = !pv->buff[pv->ptrA];
      
      if (pv->buff[pv->buffInc-3] == pv->buff[pv->ptrA])
      pv->checkVal = -1;
   }
   
   if (bitVal == pv->checkVal)
   return 0;
   
   if (pv->checkVal == 2)
   return 0;
   
   return 1;
}


int checkLinePatt(int bitVal, struct pattern_variables *pv)
{
   if (pv->cntB)
   {
      // check if the given pattern bit value is allowed
      if (checkBit(bitVal, pv))
      return 1;
      
      // grow the pattern model that will be used to tell if the line is straight
      growPattModel(bitVal, pv);
   }
   else
   {
      if (iniCheckBit(bitVal, pv))
      return 1;
      
      iniGrowPattModel(bitVal, pv);
   }
   
   return 0;
}
