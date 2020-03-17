class pattern_variables:
   def __init__(self, buff, buffInc, ptrA, ptrB, cntA, cntB, multH1, multH2, incA, incB, multInc, multPrior, multA, multB, multOffset, identiSegInc, flipFlopSegInc, checkVal):
      self.buff = buff
      self.buffInc = buffInc
      self.ptrA = ptrA # 'A' occurs more often than 'B'
      self.ptrB = ptrB
      self.cntA = cntA
      self.cntB = cntB
      self.incA = incA
      self.incB = incB
      self.multInc = multInc # if incA == cntA then multInc++
      self.multPrior = multPrior
      self.multH1 = multH1 # holds multA/B until they can be distinguished
      self.multH2 = multH2
      self.multA = multA # occurs more often than multB
      self.multB = multB
      self.multOffset = multOffset
      self.identiSegInc = identiSegInc # identical segment increment
      self.flipFlopSegInc = flipFlopSegInc
      self.checkVal = checkVal

def resetPattVars():
   pv = pattern_variables([], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2)
   return pv

def recalcBitPattVars(pv):
   bitCntA = pv.multA * pv.cntA + pv.cntB
   bitCntB = pv.multB * pv.cntA + pv.cntB
   pv.ptrA = pv.buffInc+1 - (bitCntA + bitCntB)
   pv.ptrB = pv.buffInc+1 - bitCntB
   pv.cntA = bitCntA
   pv.cntB = bitCntB
   return

def accOffset(pv): # account offset

   pv.multH1 = 0
   pv.multH2 = 0
   
   if pv.flipFlopSegInc >= 2:
      pv.multA = 1
      pv.multB = 2
      recalcBitPattVars(pv)
      pv.multOffset = pv.flipFlopSegInc - 1
      pv.multInc = 0
   elif pv.flipFlopSegInc == 1:
      if pv.multInc == pv.multOffset:
         pv.multOffset = 0
      else:
         pv.multOffset = 1
      pv.multInc = pv.identiSegInc + 1
      pv.multH1 = pv.multInc
   elif pv.flipFlopSegInc == 0:
      pv.multOffset = pv.identiSegInc + 1
      pv.multInc = 0
   
   pv.multA = 0
   pv.multB = 0
   pv.flipFlopSegInc = 0
   pv.identiSegInc = 0
   
   return

def recalcPattVars(pv):
   
   # account offsets that represent current largest possible
   if pv.multInc == pv.multOffset-1:
      if pv.multH1 == 0:
         pv.multH1 = pv.multOffset
      elif pv.multH2 == 0:
         pv.multH1 = pv.multOffset
         pv.multH2 = pv.multInc
         pv.multA = pv.multOffset
         pv.identiSegInc += 1
   
   # account multiplier holders
   if pv.multH1 == 0:
      pv.multH1 = pv.multInc
   elif pv.multH2 == 0:
      if pv.multInc != pv.multH1:
         pv.multH2 = pv.multInc
   
   # account both multA/B
   if pv.multA == 0:
      if pv.multInc == pv.multPrior:
         pv.multA = pv.multInc
   else:
      if pv.multInc != pv.multA:
         pv.multB = pv.multInc
         recalcBitPattVars(pv)
   
   if pv.multB:
      accOffset(pv)
   else:
      # keep track of identical segments
      if pv.multInc == pv.multPrior:
         pv.identiSegInc += 1
      
      # keep track of the flip-flop pattern size
      if pv.multInc == pv.multH2 and pv.multInc != pv.multPrior:
         pv.flipFlopSegInc += 1
   
   return

def growPattModel(bitVal, pv):
   
   # find if the pattern matches 'A' or 'B'
   if bitVal == pv.buff[pv.ptrA+pv.incA]:
      if pv.incA != -1:
         pv.incA += 1
   else:
      pv.incA = -1 # does not match
   
   if bitVal == pv.buff[pv.ptrB+pv.incB]:
      if pv.incB != -1:
         pv.incB += 1
   else:
      pv.incB = -1 # does not match
   
   if pv.incA == pv.cntA: # if the pattern matches 'A'
      pv.multInc += 1
      pv.incA = 0
      pv.incB = 0
   
   if pv.incB == pv.cntB: # if the pattern matches 'B'
      recalcPattVars(pv)
      pv.multPrior = pv.multInc
      pv.multInc = 0
      pv.incA = 0
      pv.incB = 0
   
   pv.buff.append(bitVal)
   pv.buffInc += 1
   
   return

def iniGrowPattModel(bitVal, pv):

   pv.checkVal = 2

   # find the bit value that occurs more often
   if pv.cntA == 0:
      if pv.buffInc and bitVal == pv.buff[pv.buffInc-1]:
         pv.ptrA = pv.buffInc-1
         pv.cntA = 1
   else:
      # find the other bit
      if pv.cntB == 0:
         if bitVal != pv.buff[pv.ptrA]:
            pv.ptrB = pv.buffInc
            pv.cntB = 1
         
            # find offset
            if pv.buffInc >= 5 and pv.buff[pv.buffInc-2] != pv.buff[pv.buffInc-3]:
               # if flip-flop Pattern
               pv.ptrA -= 2; pv.ptrB -= 2
               pv.cntA = 2; pv.cntB = 3
               pv.multOffset = (pv.buffInc-3) >> 1
               pv.checkVal = 0 if pv.buff[pv.ptrA] else 1
            else:
               if pv.buff[0] == pv.buff[1]:
                  pv.multOffset = pv.buffInc
               else:
                  if pv.buff[0] == pv.buff[pv.ptrA]:
                     pv.multOffset = 1
                  pv.multH1 = pv.ptrB - pv.ptrA
                  pv.multPrior = pv.multH1
   
   pv.buff.append(bitVal)
   pv.buffInc += 1
   
   return

def resolveDuoBitVal(pv):
   if pv.incA != -1 and pv.incB != -1:
      if pv.buff[pv.ptrA+pv.incA] == pv.buff[pv.ptrB+pv.incB]:
         pv.checkVal = pv.buff[pv.ptrA+pv.incA]
      else:
         pv.checkVal = 2
   elif pv.incA != -1:
      pv.checkVal = pv.buff[pv.ptrA+pv.incA]
   elif pv.incB != -1:
      pv.checkVal = pv.buff[pv.ptrB+pv.incB]
   return

def duoPattCheck(pv, smMult, lgMult):
   if pv.multInc == smMult:
      resolveDuoBitVal(pv)
   elif pv.multInc == lgMult:
      pv.checkVal = pv.buff[pv.ptrB+pv.incB]
   elif pv.multInc < smMult:
      pv.checkVal = pv.buff[pv.ptrA+pv.incA]
   return

def trioPattCheck(pv, smMult, medMult, lgMult):
   if pv.multInc == smMult:
      resolveDuoBitVal(pv)
   elif pv.multInc == medMult:
      resolveDuoBitVal(pv)
   elif pv.multInc == lgMult:
      pv.checkVal = pv.buff[pv.ptrB+pv.incB]
   elif pv.multInc < smMult:
      pv.checkVal = pv.buff[pv.ptrA+pv.incA]
   return

def flipFlopPattCheck(pv, mult1, mult2):
   if pv.multA == mult1:
      if pv.multInc == mult2:
         pv.checkVal = pv.buff[pv.ptrB+pv.incB]
      elif pv.multInc < mult2:
         pv.checkVal = pv.buff[pv.ptrA+pv.incA]
   elif pv.multA == mult2:
      if pv.multInc == mult1:
         pv.checkVal = pv.buff[pv.ptrB+pv.incB]
      elif pv.multInc < mult1:
         pv.checkVal = pv.buff[pv.ptrA+pv.incA]
   return

def checkBit(bitVal, pv):

   pv.checkVal = -1

   if pv.incA == -1 and pv.incB == -1:
      return 1
   
   if pv.multH2: # check for 2 possible patterns
      if pv.flipFlopSegInc >= 2 and pv.multA: # end of flip-flop pattern
         flipFlopPattCheck(pv, pv.multH1, pv.multH2)
      elif pv.multH1 == pv.multH2-1: # multH1 < multH2
         duoPattCheck(pv, pv.multH1, pv.multH2)
      elif pv.multH2 == pv.multH1-1: # multH2 < multH1
         duoPattCheck(pv, pv.multH2, pv.multH1)
   elif pv.multH1 and pv.multH2 == 0: # check for 3 possible patterns
      trioPattCheck(pv, pv.multH1-1, pv.multH1, pv.multH1+1)
   elif pv.multH1 == 0: # pattern size is unknown
      if pv.multInc >= pv.multOffset-1:
         resolveDuoBitVal(pv)
      elif pv.multInc < pv.multOffset-1:
         pv.checkVal = pv.buff[pv.ptrA+pv.incA]

   if bitVal == pv.checkVal:
      return 0
   
   if pv.checkVal == 2:
      return 0
   
   return 1

def checkLinePatt(bitVal, pv):
   
   if pv.cntB:
      # check if the given pattern bit value is allowed
      if checkBit(bitVal, pv):
         return 1
      
      # grow the pattern model that will be used to tell if the line is straight
      growPattModel(bitVal, pv)
   else:
      iniGrowPattModel(bitVal, pv)
   
   return 0
