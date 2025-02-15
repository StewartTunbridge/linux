////////////////////////////////////////////////////////////////////////////
//
// DISPLAY DIRECTORY TREE
//

byte ReadDirCallbackTree (_DirEntry *Item, int Depth)
  {
    byte Res;
    //
    Res = 0;
    #ifndef _Windows
    if (!Item->SymLink)   // not SymLink
    #endif
      if (Item->Directory)
        Res = ReadDirInList;
      else
        Res = ReadDirInStats;
    return Res;
  }

bool SortReverse;

int DirSortCompareTree (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    char *sA, *sB;
    //
    sA = GetItemPath (A);
    sB = GetItemPath (B);
    Result = StrCmp (sA, sB);
    free (sA);
    free (sB);
    if (SortReverse)
      return -Result;
    return Result;
  }

int TreeRoot;
unsigned long *TreeBranches;

int TreeDepth (char *Path)
  {
    return StrChCount (&Path [TreeRoot], PathDelimiter, 0);
  }

void TreeBranchesBuild (_DirEntry *Dir, int ListSize)
  {
    int Index1, Index2, i;
    _DirEntry *d1, *d2;
    //
    TreeBranches = malloc (ListSize * sizeof (TreeBranches [0]));
    for (i = 0; i < ListSize; i++)
      TreeBranches [i] = 0;
    d1 = Dir;
    Index1 = 0;
    while (d1)
      {
        i = TreeDepth (d1->Path);
        if (i)
          {
            TreeBranches [ListSize - 1 - Index1] |= Bit [i + i - 1] | Bit [i + i - 2];
            Index2 = Index1 + 1;
            d2 = d1->Next;
            while (true)
              {
                if (d2 == NULL)
                  break;
                if (TreeDepth (d2->Path) < i)
                  break;
                TreeBranches [ListSize - 1 - Index2] |= Bit [i + i - 2];
                Index2++;
                d2 = d2->Next;
              }
          }
        Index1++;
        d1 = d1->Next;
      }
  }

void ShowPageItemTree (void *Data, int Index, int xOffset)
  {
    _DirEntry *Item;
    unsigned long b;
    int i;
    //
    // Find the Item in the list
    Item = Data;
    for (i = 0; i < Index; i++)
      Item = Item->Next;
    // Show the Tree
    PutChar (' ');
    b = TreeBranches [Index];
    //PutIntLengthBase (b, 12, 2);
    i = ConsoleFG;
    ConsoleColourFG (ColourBodyFGDir);
    while (b)
      {
        if (b & 0x1)
          PutChar ('|');
        else
          PutChar (' ');
        if (b & 0x2)
          PutChar ('_');
        else
          PutChar (' ');
        b >>= 2;
      }
    //PutCharN (' ', i + i);
    ConsoleColourFG (i);
    PutString (Item->Name);
    // Show some stats
    ConsoleCursor (ConsoleSizeX - 2 - (8 + 14 + 1), ConsoleY);
    //PutCharN (' ', 2);
    PutInt (Item->Count, 8);
    PutInt (Item->Size, (14 + 1) | IntToLengthCommas);
  }

bool ShowPageSeekTree (void *Data, int Index, char *Target)
  {
    _DirEntry *Item;
    int i;
    // Find the Item in the list
    Item = Data;
    for (i = 0; i < Index; i++)
      Item = Item->Next;
    return (int) StrPos (Item->Name, Target) >= 0;
  }

bool ShowTree (int Root)
  {
    _DirEntry *List, *l;
    int ListSize;
    int Sel;
    bool Res;
    int c;
    //
    TreeRoot = Root;
    List = NULL;
    ReadDir (&List, true, ReadDirCallbackTree);
    ListSize = GetDirLength (List);
    SortReverse = true;
    List = (_DirEntry *) SortList ((void **) List, (_SortListCompare *) DirSortCompareTree);
    TreeBranchesBuild (List, ListSize);
    SortReverse = false;
    List = (_DirEntry *) SortList ((void **) List, (_SortListCompare *) DirSortCompareTree);
    //TreeArrayBuild (List, ListSize);
    ConsoleLine (1, ColourTitleFG1, ColourTitleBG);
    ConsoleLine (ConsoleSizeY - 1, ColourHelpFG1, ColourHelpBG);
    PutStringHighlight ("|Esc|  |Enter|", ColourHelpFG1, ColourHelpFG2);
    Res = false;
    Sel = 0;
    while (true)
      {
        c = ShowGenericPage (2, 1, ShowPageItemTree, List, ListSize, ColourBodyFG, ColourBodyBG, ColourBodyBGSel, &Sel, &ShowPageSeekTree);
        if (c == KeyEnter)
          {
            l = FindDirEntry (List, Sel);
            if (l)
              if (chdir (l->Path) == 0)
                if (chdir (l->Name) == 0)
                  Res = true;
            break;
          }
        if (c == esc)
          break;
        if (c != GetKeyWaitResizeOccured)
          ConsoleBeep ();
      }
    FreeDir (List);
    free (TreeBranches);
    //free (TreeArray);
    return Res;
  }
