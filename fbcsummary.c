/////////////////////////////////////////////////////////////////////
//
// SUMMARY (Breakdown)

typedef struct SummaryItem
  {
    struct SummaryItem *Next;
    char *Key;
    int Count;
    longint Size;
    int Tagged;
  } _SummaryItem;

_SummaryItem *SummaryFind (_SummaryItem *Sum, char *Key)
  {
    while (Sum)
      {
        if (StrCompare (Sum->Key, Key) == 0)   // Found
          return (Sum);
        Sum = Sum->Next;
      }
    return NULL;
  }

_SummaryItem *SummaryMake (_DirEntry *Dir, int *Length, bool byDir, int RootPathLength)
  {
    _SummaryItem *Res, *s;
    char *k;
    //
    *Length = 0;
    Res = NULL;
    while (Dir)
      {
        k = (char *) StrNull;
        if (byDir)
          {
            if (RootPathLength < StrLength (Dir->Path))
              k = &Dir->Path [RootPathLength];
          }
        else
          k = StrGetFileExtension (Dir->Name);
        if (!Dir->Directory)
          {
            s = SummaryFind (Res, k);
            if (s == NULL)   // Not found in list, add new item
              {
                // Create new Summary Item
                s = malloc (sizeof (_SummaryItem));
                MemSet (s, 0, sizeof (_SummaryItem));
                //s->Next = NULL;
                //s->Count = 0;
                //s->Size = 0;
                StrAssign (&(s->Key), k);
                // Add to the list
                s->Next = Res;
                Res = s;
                (*Length)++;
              }
            // Update Stats
            s->Count++;
            s->Size += Dir->Size;
            if (Dir->Tagged)
              s->Tagged++;
          }
        Dir = Dir->Next;
      }
    return Res;
  }

typedef enum {smKey, smCount, smSize, smZZZZ} _SummarySortMode;

int SummaryCompareKey (_SummaryItem *A, _SummaryItem *B)
  {
    int Res;
    //
    Res = StrCompareCase (A->Key, B->Key, false);
    if (Res == 0)
      Res = StrCompare (A->Key, B->Key);
    return Res;
  }

int SummaryCompareCount (_SummaryItem *A, _SummaryItem *B)
  {
    if (A->Count == B->Count)
      return SummaryCompareKey (A, B);
    return A->Count - B->Count;
  }

int SummaryCompareSize (_SummaryItem *A, _SummaryItem *B)
  {
    if (A->Size == B->Size)
      return SummaryCompareKey (A, B);
    if (A->Size > B->Size)
      return +1;
    if (A->Size < B->Size)
      return -1;
    return 0;
  }

void SummarySort (_SummaryItem **Sum, _SummarySortMode SortMode)
  {
    switch (SortMode)
      {
        case smKey:
          *Sum = SortList ((void **) *Sum, (_SortListCompare *) &SummaryCompareKey);
          break;
        case smCount:
          *Sum = SortList ((void **) *Sum, (_SortListCompare *) &SummaryCompareCount);
          break;
        case smSize:
          *Sum = SortList ((void **) *Sum, (_SortListCompare *) &SummaryCompareSize);
          break;
      }
  }

void SummaryFree (_SummaryItem *Sum)
  {
    _SummaryItem *s;
    //
    while (Sum)
      {
        free (Sum->Key);
        s = Sum->Next;
        free (Sum);
        Sum = s;
      }
  }

/*
byte ColourSelTag (bool Sel, bool Tag)
  {
    if (Tag)
      {
        if (Sel)
          return ColourBodyBGSelTag;
        return ColourBodyBGTag;
      }
    return ColourBodyBG;
  }

void ColourLine (bool Tagged)
  {
    if (Tagged)
      if (ConsoleBG == ColourBodyBGSel)
        ConsoleLine (ConsoleY, ConsoleFG, ColourBodyBGSelTag);
      else
        ConsoleLine (ConsoleY, ConsoleFG, ColourBodyBGTag);
  }
*/

void ShowPageItemSummary (void *Data, int Index, int xOffset)
  {
    _SummaryItem *Sum;
    bool Sel;
    byte FG;
    //
    Sum = (_SummaryItem *) Data;
    while (Index--)
      Sum = Sum->Next;
    Sel = (ConsoleBG == Colours [ColBodyBGSel]);
    FG = Sum->Tagged == Sum->Count ? Colours [ColBodyFG] : Colours [ColBodyFGDir];
    if (Sum->Tagged)
      if (Sel)
        ConsoleLine (ConsoleY, FG, Colours [ColBodyBGSelTag]);
      else
        ConsoleLine (ConsoleY, FG, Colours [ColBodyBGTag]);
    PutInt (Sum->Count, 8 | IntToLengthCommas);
    PutInt (Sum->Size, (16 + 1) | IntToLengthCommas);
    PutChar (' ');
    /*
    if (Sum->Tagged > 0 && Sum->Tagged < Sum->Count)   // Some tagged
      if (Sel)
        ConsoleColourBG (ColourBodyBGSel);
      else
        ConsoleColourBG (ColourBodyBG);
    */
    PutString (Sum->Key);
  }

bool ShowPageSeekSummary (void *Data, int Index, char *Target)
  {
    _SummaryItem *Item;
    int i;
    // Find the Item in the list
    Item = Data;
    for (i = 0; i < Index; i++)
      Item = Item->Next;
    return (int) StrPos_ (Item->Key, Target) >= 0;
  }

char *SubStr (char *Str, int Len)
  {
    int l;
    //
    l = StrLength (Str);
    if (Len > l)
      Len = l;
    return &Str [Len];
  }

void SummaryTagMatchingKey (_DirEntry *Dir, _SummaryItem *SI, bool MatchPath, bool DoTagging)
  {
    _DirEntry *d;
    bool Tag;
    bool Match;
    int i;
    //
    StrAssign (&DirItem, NULL);
    Tag = SI->Count != SI->Tagged;
    d = Dir;
    i = 0;
    while (d)
      {
        if (MatchPath)
          Match = StrCompare (SubStr (d->Path, HomeDirLen), SI->Key) == 0;
        else
          Match = StrCompare (StrGetFileExtension (d->Name), SI->Key) == 0 && !d->Directory;
        if (Match)
          {
            if (DoTagging)
              d->Tagged = Tag;
            else
              {
                DirItemIndex = i;
                break;
              }
            //if (!DirItem)
            //  StrAssign (&DirItem, d->Name);
          }
        d = d->Next;
        i++;
      }
    if (DoTagging)
      if (Tag)
        SI->Tagged = SI->Count;
      else
        SI->Tagged = 0;
  }

void ShowSummary (_DirEntry *Dir, bool byDir, int CommonPathLength)
  {
    int Sel;
    _SummaryItem *Sum, *s;
    int Length;
    _SummarySortMode SortMode;
    int c;
    int i;
    //
    SortMode = smKey;
    Sum = SummaryMake (Dir, &Length, byDir, CommonPathLength);
    Sel = 0;
    if (Length)
      while (true)
        {
          ConsoleLine (1, Colours [ColTitleFG1], Colours [ColTitleBG]);
          ShowTitle ("   Count", SortMode == smCount);
          ShowTitle ("            Size", SortMode == smSize);
          if (byDir)
            ShowTitle ("Directory", SortMode == smKey);
          else
            ShowTitle ("Extension", SortMode == smKey);
          ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
          PutStringHighlight ("|^S|ort  |Enter|=go  |SPACE|=tag  |Esc|", Colours [ColHelpFG2]);
          SummarySort (&Sum, SortMode);
          c = ShowGenericPage (2, 1, ShowPageItemSummary, Sum, Length, Colours [ColBodyFG], Colours [ColBodyBG], Colours [ColBodyBGSel], &Sel, ShowPageSeekSummary);
          s = Sum;
          for (i = 0; i < Sel; i++)
            s = s->Next;
          if (c == esc)
            break;
          if (c == KeyEnter)
            {
              SummaryTagMatchingKey (Dir, s, byDir, false);
              break;
            }
          if (c == ' ')
            SummaryTagMatchingKey (Dir, s, byDir, true);
          else if (c == Cntrl ('S'))
            SortMode = (SortMode + 1) % smZZZZ;
          else if (c != GetKeyWaitResizeOccured)
            ConsoleBeep ();
        }
    SummaryFree (Sum);
  }
