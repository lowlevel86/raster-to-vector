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

struct pattern_variables
{
   char *buff;
   int buffInc;
   int ptrA; // 'A' occurs more often than 'B'
   int ptrB;
   int cntA;
   int cntB;
   int incA;
   int incB;
   int multInc; // if incA == cntA then multInc++
   int multPrior;
   int multH1; // holds multA/B until they can be distinguished
   int multH2;
   int multA; // occurs more often than multB
   int multB;
   int multOffset;
   int identiSegInc; // identical segment increment
   int flipFlopSegInc;
   int checkVal;
};

// iniPattVars() requires a pointer to a buffer with
// the size of the window width for vertical edges
// or window height for horizontal edges
struct pattern_variables *iniPattVars(char *);

int checkLinePatt(int, struct pattern_variables *);
