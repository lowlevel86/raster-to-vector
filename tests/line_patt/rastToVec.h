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
