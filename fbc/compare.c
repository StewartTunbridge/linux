////////////////////////////////////////////////////////////////////////////
//
// COMPARE TEXT
// ============
//
// 03 Mar 2020 Implement


// Looks for Target in Str
// Returns Str index and the length of the Match
unsigned int StrPosN (char *Str, char *Target, unsigned int Threshold, unsigned int *MatchLen)
  {
    int a, b;
    //
    a = 0;
    b = 0;
    while (true)
      {
        if (Target [b] == 0)   // end of Target
          break;
        if (Str [a + b] == 0)   // end of Str
          break;
        if (Str [a + b] == Target [b])   // matching so far
          b++;
        else
          if (b >= Threshold)   // not a completye match but close enough
            break;
          else   // not good enough, keep looking
            {
              a++;
              b = 0;
            }
      }
    *MatchLen = b;
    return a;
  }


// Compare 2 blocks of text
// Result is the global pointer and integer below.
// It includes common text and marked differences
// Caller must free

char *CompareResult;
unsigned int CompareResultLength;
int CompareResultDifferences;

void StrToResult (char **Str, unsigned int Len, int Mode)
  {
    unsigned int n;
    static int ModeOld = 0;
    //
    if (Len)
      {
        if (Mode != ModeOld)
          {
            if (ModeOld)
              {
                CharToStrRepeat (&CompareResult, '#', 4);
                CharToStrRepeat (&CompareResult, '>', ModeOld);
                CompareResultLength += 4 + ModeOld;
              }
            else
              CompareResultDifferences++;
            ModeOld = Mode;
            if (Mode)
              {
                CharToStrRepeat (&CompareResult, '<', Mode);
                CharToStrRepeat (&CompareResult, '#', 4);
                CompareResultLength += 4 + Mode;
              }
          }
        n = StrToStrN (&CompareResult, *Str, Len);
        (*Str) += n;
        CompareResultLength += n;
      }
  }

void CompareTextBlocks (char *a, char *b, int Threshold)
  {
    char *Result;
    char *a_, *b_;   // Walking Pointers
    unsigned int aindex, amatch;
    unsigned int bindex, bmatch;
    //
    a_ = a;
    b_ = b;
    CompareResult = Result = NULL;
    CompareResultLength = 0;
    CompareResultDifferences = 0;
    while (true)
      {
        if ((*a_ == 0) || (*b_ == 0))   // end of the road
          {
            if (*a_)
              StrToResult(&a_, -1, 1);
            if (*b_)
              StrToResult(&b_, -1, 2);
            StrToResult (&a_, -1, 0);
            a_ = a;
            b_ = b;
            if (Result == NULL)
              CompareResult = Result = malloc (CompareResultLength + 1);
            else
              break;
            CompareResultLength = 0;
            CompareResultDifferences = 0;
          }
        bindex = StrPosN (b_, a_, Threshold, &bmatch);
        aindex = StrPosN (a_, b_, Threshold, &amatch);
        if ((amatch < Threshold) && (bmatch < Threshold))   // unique in both
          StrToResult (&a_, 1, 1);
        else if (amatch > bmatch)   // a has unique part for biggest common
          {
            StrToResult (&a_, aindex, 1);
            StrToResult (&a_, amatch, 0);
            b_ += amatch;
          }
        else   // b has unique text
          {
            StrToResult (&b_, bindex, 2);
            StrToResult (&b_, bmatch, 0);
            a_ += bmatch;
          }
      }
    *CompareResult = 0;   // Terminate Res
    CompareResult = Result;   // Publish start of result
  }
