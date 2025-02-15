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

int DirSortCompareTree (_DirEntry *A, _DirEntry *B)
  {
    int Result;
    char *sA, *sB;
    //
    sA = GetItemPath (A);
    sB = GetItemPath (B);
    Result = StrCompareCase (sA, sB, false);
    free (sA);
    free (sB);
    return Result;
  }

int TreeRoot;

int TreeDepth (char *Path)
  {
    return StrChCount (&Path [TreeRoot], PathDelimiter, 0);
  }

void ShowPageItemTree (void *Data, int Index, int xOffset)
  {
    _DirEntry *Item;
    int i;
    //
    // Find the Item in the list
    Item = Data;
    for (i = 0; i < Index; i++)
      Item = Item->Next;
    // Show the Tree
    ConsoleColourFG (Colours [ColBodyFGDir]);
    //i = TreeDepth (Item->Path);
    //while (i--)
    //  PutString ("/ ");
    PutCharN (' ', 1 + 2 * TreeDepth (Item->Path));
    PutString (Item->Name);
    // Show some stats
    ConsoleCursor (ConsoleSizeX - 2 - (8 + 14 + 1), ConsoleY);
    ConsoleColourFG (Colours [ColBodyFG]);
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
    return (int) StrPos_ (Item->Name, Target) >= 0;
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
    List = (_DirEntry *) SortList ((void **) List, (_SortListCompare *) DirSortCompareTree);
    ConsoleLine (1, Colours [ColTitleFG1], Colours [ColTitleBG]);
    ConsoleCursor (ConsoleSizeX - 2 - (8 + 14 + 1), ConsoleY);
    PutString ("   Count");
    PutString ("  TotalFileSize");
    ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
    PutStringHighlight ("|Esc|  |Enter|", Colours [ColHelpFG2]);
    Res = false;
    Sel = 0;
    while (true)
      {
        c = ShowGenericPage (2, 1, ShowPageItemTree, List, ListSize, Colours [ColBodyFG], Colours [ColBodyBG], Colours [ColBodyBGSel], &Sel, &ShowPageSeekTree);
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
    return Res;
  }
