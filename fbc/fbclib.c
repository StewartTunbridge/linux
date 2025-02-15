////////////////////////////////////////////////////////////////////////////
//
// SUPPORT
//

// Find any resource file on the system
// Caller must free result
char *FindFileResource (char *Filename)
  {
    char *Path, *p;
    char *Res;
    //
    Res = NULL;
    Path = malloc (MaxPath);
    if (FileExists (Filename))   // Is Filename here?
      StrAssign (&Res, Filename);
    else
      {
        p = Path;   // Is it in the Starting Directory?
        StrToStr (&p, DirStart);
        CharToStr (&p, PathDelimiter);
        StrToStr (&p, Filename);
        *p = 0;
        if (FileExists (Path))
          StrAssign (&Res, Path);
        else
          {
            #ifndef _Windows
            p = Path;   // Is it in the Config folder?
            StrPathConfig (&p, Filename, (char *) AppName);
            if (FileExists (Path))
              StrAssign (&Res, Path);
            else
            #endif // _Windows
              {
                p = Path;   // Is it in the Home folder?
                StrPathHome (&p, Filename);
                if (FileExists (Path))
                  StrAssign (&Res, Path);
                else
                  Res = FindFileInPath (Filename);   // Is it on the $PATH?
              }
          }
      }
    free (Path);
    return Res;
  }

bool In (int Val, int Range1, int Range2)
  {
    if (Range1 <= Range2)
      return (Val >= Range1) && (Val <= Range2);
    return (Val >= Range2) && (Val <= Range1);
  }

bool StrSame_ (char *S1, char *S2)
  {
    return StrCompareCase (S1, S2, false) == 0;
  }

void ConsoleLine_ (int Line, int FG, int BG)
  {
    ConsoleCursor (0, Line);
    ConsoleColourFG (FG);
    ConsoleColourBG (BG);
    ConsoleClearEOL ();
    //while (ConsoleX < ConsoleSizeX)
    //  PutChar (' ');
    //ConsoleCursor (0, Line);
  }

void WaitKey (void)
  {
    PutString ("<Press a key>");
    GetKeyWait (false);
  }

byte EditString_ (char *St, int Size)
  {
    return EditString (St, Size, ConsoleSizeX - ConsoleX);
  }

byte EditString__ (char *St, int Size)
  {
    ConsoleColourFG (Colours [ColQueryFG2]);
    return EditString_ (St, Size);
  }

void PutNumName (int Num, char *NameSingular, char *NamePlural)
  {
    char St [255], *s;
    //
    s = St;
    IntStrToStrFirst = true;
    IntStrToStr (&s, Num, NameSingular, NamePlural);
    *s = 0;
    PutString (St);
    /*
    PutInt (Num, IntToLengthCommas);
    PutChar (' ');
    if (Num == 1)
      PutString (NameSingular);
    else
      PutString (NamePlural);
    */
  }

/*
void IntStrToStr (char **St, int Num, char *NameSingular, char *NamePlural)
  {
    IntToStrBase (St, Num, IntToLengthCommas, 10);
    *(*St++) = ' ';
    PutChar (' ');
    if (Num == 1)
      StrToStr (St, NameSingular);
    else
      StrToStr  (St, NamePlural);
  }

void ShowHelpPage (const char **HelpPage, int Size)
  {
    int c;
    //
    ShowPageHelpFGMask = Colours [ColHelpFG1] ^ Colours [ColHelpFG2];
    while (true)
      {
        c = ShowGenericPage (0, 0, ShowPageItemHelp, HelpPage, Size, Colours [ColHelpFG1], Colours [ColHelpBG], Colours [ColHelpBG] ^ ColBright, NULL, NULL);
        if ((c == esc) || (c == KeyEnter))
          break;
        if (c != GetKeyWaitResizeOccured)
          ConsoleBeep ();
      }
  }
*/

void ShowHelpPageFile_ (char *Filename)
  {
    char *Path;
    int c;
    //
    Path = FindFileResource (Filename);
    if (Path)
      {
        ShowPageHelpFGMask = Colours [ColHelpFG1] ^ Colours [ColHelpFG2];
        while (true)
          {
            c = ShowHelpPageFile (Path, 0, 0, Colours [ColHelpFG1], Colours [ColHelpBG]);
            if ((c == esc) || (c == KeyEnter))
              break;
            if (c != GetKeyWaitResizeOccured)
              ConsoleBeep ();
          }
        free (Path);
      }
    else
      StrCopy (Message, "No Help file");
  }

/*
void ShowPageItemHelp (void *Data, int Index, int xOffset)
  {
    char *HelpLine;
    char c;
    int x;
    byte Col;
    const int Column = 14;
    //
    HelpLine = ((char **) Data) [Index];
    Col = ColourHelpFG1;
    if (*HelpLine == '_')
      {
        Col |= ColUnderline;
        HelpLine++;
      }
    x = 0;
    while (*HelpLine)
      {
        c = *HelpLine++;
        if (c == '|')   // Hot key on/off
          Col = Col ^ ColourHelpFG1 ^ ColourHelpFG2;   // Switch between 2 help colours
        else if (c == tab)
          do
            {
              if (x >= xOffset)
                PutCharCol (' ', Col);
              x++;
            }
          while (x % Column);
        else
          {
            if (x >= xOffset)
              PutCharCol (c, Col);
            x++;
          }
      }
  }

byte ShowPage (int Head, int Foot, _ShowPageItem *SPI, void *Data, int Size, int ColFG, int ColBG)
  {
    int xOffset, yOffset;
    int y;   // vertical position within the data window
    int c;
    //
    GetKeyMacro = NULL;
    //ConsoleTab = Column;
    xOffset = yOffset = 0;
    y = 0;
    while (true)
      {
        // Draw page
        for (y = Head; y < ConsoleSizeY - Foot; y++)
          {
            ConsoleLine (y, ColFG, ColBG);
            if (y - Head + yOffset < Size)
              SPI (Data, y - Head + yOffset, xOffset);
          }
        DrawScrollBar (Head, ConsoleSizeY - Foot - 1,
                       yOffset, yOffset + (ConsoleSizeY - Head - Foot - 1), Size,
                       ColFG, ColBG);
        // process user input
        //ConsoleCursor (0, y1);
        c = GetKeyWait (true);
        if (c >= 0)
          switch (c)
            {
              case esc:
                return c;
              case KeyHome:
                yOffset = 0;
                break;
              case KeyEnd:
                yOffset = Size;
                break;
              case KeyUp:
                yOffset--;
                break;
              case KeyDown:
                yOffset++;
                break;
              case KeyPageUp:
                yOffset -= ConsoleSizeY;
                break;
              case KeyPageDown:
                yOffset += ConsoleSizeY;
                break;
              case KeyLeft:
                if (xOffset > 0)
                  xOffset -= 8; //Column / 2;
                break;
              case KeyRight:
                xOffset += 8; //Column / 2;
                break;
              default:
                return c;
            }
        while (yOffset + (ConsoleSizeY - Head - Foot) > Size)
          yOffset--;// = Size - ConsoleSizeY;
        if (yOffset < 0)
          yOffset = 0;
      }
  }

void ShowHelpPage (const char **HelpPage, int Size)
  {
    while (true)
      {
        if (ShowPage (0, 0, ShowPageItemHelp, HelpPage, Size, ColourHelpFG1, ColourHelpBG) == esc)
          break;
        ConsoleBeep ();
      }
  }
*/

void ShowTitle (char *Titl, bool Sorted)
  {
    if (Sorted)
      ConsoleColourFG (Colours [ColTitleFG2]);
    PutString (Titl);
    if (Sorted)
      ConsoleColourFG (Colours [ColTitleFG1]);
    PutChar (' ');
  }

typedef enum {tBase, tPresent, tPast} _Tense;

const char *VerbsCopy [] = {"Copy ", "Copying ", " Copied "};
const char *VerbsMove [] = {"Move ", "Moving ", " Moved "};
const char *VerbsDelete [] = {"Delete ", "Deleting ", " Deleted "};

const char *CommandName (char Command, _Tense Tense)
  {
    if (Tense > tPast)
      return NULL;
    switch (Command)
      {
        case CmdCopy:   return VerbsCopy [Tense];
        case CmdMove:   return VerbsMove [Tense];
        case CmdDelete: return VerbsDelete [Tense];
      }
    return NULL;
  }

void PutCommandName (byte Command, _Tense Tense)
  {
    PutString (CommandName (Command, Tense));
  }

void StrTimeDelta (char **St, int Time)   // Convert seconds to days, hours etc
  {
    const int TimeDay = 60 * 60 * 24;
    const int TimeHour = 60 * 60;
    const int TimeMin = 60;
    //
    if (Time >= TimeDay)   // >= 100 hours, show days
      {
        IntToStr (St, Time / TimeDay);
        StrToStr (St, " day");
        if (Time >= 2 * TimeDay)
          CharToStr (St, 's');
        CharToStr (St, ' ');
        Time = Time % TimeDay;
      }
    IntToStrFill (St, Time / TimeHour, 2 | IntToLengthZeros);
    Time = Time % TimeHour;
    CharToStr (St, ':');
    IntToStrFill (St, Time / TimeMin, 2 | IntToLengthZeros);
    Time = Time % TimeMin;
    CharToStr (St, ':');
    IntToStrFill (St, Time, 2 | IntToLengthZeros);
  }

void PutTimeDelta (int Time)
  {
    char St [24], *x;
    //
    x = St;
    StrTimeDelta (&x, Time);
    *x = 0;
    PutString (St);
  }

void ConsolePrompt (int ColourFG, int ColourBG)
  {
    ConsoleLine (ConsoleSizeY - 1, ColourFG, ColourBG);
    ConsoleLine (ConsoleSizeY - 2, ColourFG, ColourBG);
    HelpShow = true;
  }

void ShowActionFile (char Command, char *Filename, const char *Annotation)
  {
    int i;
    //
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutCommandName (Command, true);
    i = StrPosLastCh (Filename, PathDelimiter) + 1;
    PutString (&Filename [i]);
    PutNewLine ();
    if (Log)
      {
        LogWrite ((char *) CommandName (Command, true));
        LogWrite (Filename);
        if (Annotation)   // Suplimentary message
          {
            LogWrite (" ");
            LogWrite (Annotation);
          }
        LogWrite ("\n");
      }
  }

int PathsIndexNext (void)
  {
    return (PathsIndex + 1) % SIZEARRAY (Paths);
  }

/* not used 4.4
void PathsNormalize (void)   // Move Paths [PathsIndex] to top
  {
    char *p0;
    //
    if (PathsIndex)   // we have moved to Paths [PathsIndex]
      {  // so insert Paths [PathsIndex] at Paths [0]
        p0 = Paths [PathsIndex];
        while (PathsIndex > 0)
          {
            Paths [PathsIndex] = Paths [PathsIndex - 1];
            PathsIndex--;
          }
        Paths [0] = p0;
      }
  }

void PutPathTitle (int Index)
  {
    PutChar ('[');
    PutInt (Index + 1, 0);
    PutString ("] ");
    PutString (Paths [Index]);
  }
*/

void SelectDirectory (_DirEntry *Item)
  {
    if (Item)
      {
        chdir (Item->Path);
        free (Paths [PathsIndex]);
        Paths [PathsIndex] = GetCurrentWorkingDirectory ();
      }
  }

typedef enum {aYes, aSkip, aAbort, aError} _Action;

char *strerror (int __errnum);

void CheckAction (_DirEntry *Item, _Action *Action)
  {
    char St [128], *s;
    //
    if (*Action == aError)
      {
        ConsoleBeep ();
        // build message string
        s = St;
        StrToStr (&s, "** ERROR (");
        IntToStr (&s, errno);
        CharToStr (&s, '-');
        StrToStr (&s, strerror (errno));
        CharToStr (&s, ')');
        if (Item)
          {
            StrToStr (&s, " with ");
            StrToStr (&s, Item->Name);
          }
        *s = 0;
        if (Log)
          LogWrite_ (St);
        ConsolePrompt (Colours [ColErrorFG1], Colours [ColErrorBG]);
        PutString (St);
        PutNewLine ();
        switch (GetOption ("|C|ontinue/|A|bort", Colours [ColErrorFG2]))
          {
            case 'C': *Action = aYes;   break;
            case 'A':
            case esc: *Action = aAbort; break;
          }
        errno = 0;
      }
  }

_DirEntry *FindDirEntry (_DirEntry *Dir, int n)
  {
    while (true)
      {
        if (Dir == NULL)
          break;
        if (n == 0)
          break;
        Dir = Dir->Next;
        n--;
      }
    return Dir;
  }

/*
typedef struct ListGeneric
  {
    ListGeneric *Next;
    int Stuff;
  } _ListGeneric

void ListSearch (_ListGeneric **ListPointer, int **ListIndex, int Index)
  {
    if
  }
*/
