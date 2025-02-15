////////////////////////////////////////////////////////////////////////////
//
// FILING
//

int StatCopy;
int StatMove;
int StatDelete;

bool Executable (_DirEntry *Item)
  {
    #ifdef _Windows
    if ((int) StrPos_ (Item->Name, ".exe") >= 0)
      return true;
    if ((int) StrPos_ (Item->Name, ".bat") >= 0)
      return true;
    #else
    if (Item->Attrib & (S_IXUSR | S_IXGRP | S_IXOTH))
      return true;
    #endif
    return false;
  }

bool RenameOld (char *Filename)
  {
    bool OK;
    char *p;
    //
    OK = false;
    p = malloc (StrLength (Filename) + 8);
    StrCopy (StrCopy (p, Filename), ".old");
    if (rename (Filename, p) == 0)   // success
      OK = true;
    free (p);
    return OK;
  }

bool CopyFileNow (char *Source, char *Dest, bool Newer)
  {
    int fs, fd;
    time_t fsd;   // File Date
    bool SymLink;
    bool OK;
    byte *Buffer;
    int BufferSize;
    unsigned int x;
    int c;
    //
    OK = false;
    // Open files, read DateTimes
    fs = FileOpen (Source, foRead);
    if (fs >= 0)
      {
        fsd = FileDateTimeRead (fs, &SymLink);
        if (!SymLink)
          {
            fd = FileOpen (Dest, foWrite);
            if (fd >= 0)
              {
                ShowActionFile (CmdCopy, Source, Newer ? "** Destination Newer" : NULL);
                // Calculate Buffer Size
                BufferSize = 0x1000000; //??make bigger; //was SSIZE_MAX
                while (true)
                  {
                    Buffer = (byte *) malloc (BufferSize);
                    if (Buffer)
                      break;
                    BufferSize = BufferSize >> 1;
                  }
                // Copy Contents
                while (true)
                  {
                    PutChar ('.');
                    c = GetKey ();
                    if (c == esc)
                      break;
                    x = read (fs, Buffer, BufferSize);
                    if (x == 0)
                      {
                        OK = true;
                        break;
                      }
                    if (write (fd, Buffer, x) != x)
                      break;
                  }
                free (Buffer);
                close (fd);
              }
          }
        close (fs);
      }
    if (OK)
      if (!FileDateTimeWrite (Dest, fsd))
        OK = false;
    // Copy Permissions/Attributes
    if (OK)
      {
        struct stat fst;//####????
        stat (Source, &fst);   // Read Source attributes
        #ifndef _Windows
        //####chown (Dest, fst.st_uid,fst.st_gid);   // update to the same uid/gid
        #endif
        chmod (Dest, fst.st_mode);   // update the permissions
        StatCopy++;
      }
    return OK;
  }

longint FileSize (int file)
  {
    longint Res;
    //
    Res = lseek (file, 0, SEEK_END);
    lseek (file, 0, SEEK_SET);
    return Res;
  }

int FileDateTimeCompare (time_t dt1, time_t dt2)
  {
    int Res, x;
    //
    // Calculate relative time difference
    Res = dt1 - dt2;
    // ignore 1 second rounding differences
    x = Res % 3600;   // seconds in the hour
    if (x < 0)
      x += 3600;
    if (x <= 2)
      Res = Res - x;
    else if (x >= 3600 - 2)
      Res = Res + 3600 - x;
    // Ignore daylight saving & time zone type offsets
    if (Setup.CopySyncTollerateHours)
      if (Res % 3600 == 0)   // whole hour difference
        if (Abs (Res / 3600) <= 24)   // and less than 1 day
          Res = 0;
    return Res;
  }

char GetOptionDefault = 0;   // clear before potentially repeating operations
bool GetOptionRepeat = false;

char GetOptionZ (char *List)
  {
    char List_ [128], *l;
    char op;
    //
    if (GetOptionDefault == 0)
      GetOptionRepeat = false;
    if (GetOptionRepeat)
      op = GetOptionDefault;
    else
      {
        l = StrCopy (List_, List);
        if (GetOptionDefault)
          StrCopy (l, "/|Z|");
        op = GetOption (List_, Colours [ColQueryFG2]);
        if (op == 'Z')
          {
            op = GetOptionDefault;
            GetOptionRepeat = true;
          }
        else   // not 'Z'
          GetOptionDefault = op;
      }
    return op;
  }

_Action copyFile (char *Source, char *Dest)
  {
    _Action go;
    char op;
    int fs, fd;
    time_t fsd, fdd;   // File Dates
    bool SymLink;
    int i;
    //
    go = aError;
    // Open / check source
    fs = FileOpen (Source, foRead);
    fd = -1;
    if (fs >= 0)
      {
        fsd = FileDateTimeRead (fs, &SymLink);
        fdd = 0;
        if (SymLink)
          go = aSkip;
        else if (fsd >= 0)   // not dir entry error
          {
            fd = FileOpen (Dest, foRead);
            go = aYes;
            op = esc;
            if (fd >= 0)
              {
                fdd = FileDateTimeRead (fd, &SymLink);
                if (fdd >= 0)
                  {
                    if (FileSize (fs) == FileSize (fd))   // Same File Size
                      if (FileDateTimeCompare (fsd, fdd) == 0)
                        go = aSkip;
                    if (go == aYes)
                      if (fdd > fsd)   // Overwriting newer file
                        {
                          ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
                          i = StrPosLastCh (Dest, PathDelimiter) + 1;
                          PutString ("Destination ");
                          PutString (&Dest [i]);
                          PutNewLine ();
                          PutString ("Is newer by ");
                          PutTimeDelta (fdd - fsd);
                          op = GetOptionZ (" |O|verwrite/|R|everse/|S|kip/r|E|name/|A|bort");
                          //if (op == 'O') || (op == KeyEnter)
                          //  go = aYes;
                          //else
                          if (op == 'R')
                            {
                              //go = aYes;
                              close (fs);
                              fs = -1;
                              SwapBytes (&Source, &Dest, sizeof (Source));
                              fdd = fsd;
                            }
                          else if (op == 'S')
                            go = aSkip;
                          else if ((op == 'A') || (op == esc))
                            go = aAbort;
                        }
                  }
                close (fd);
                fd = -1;
                if (op == 'E')
                  {
                    //go = aYes;
                    if (!RenameOld (Dest))
                      go = aError;
                  }
              }
          }
        if (fs >= 0)
          close (fs);
      }
    if (go == aYes)
      if (!StrPosMulti (Source, Setup.CopyIgnore, spmNoCase))
        if (!CopyFileNow (Source, Dest, fdd > fsd))
          go = aError;
    return go;
  }

_Action deleteFile (char *Source)
  {
    bool Res;
    //
    Res = aError;
    ShowActionFile (CmdDelete, Source, NULL);
#ifdef _Windows
    if (DeleteFile (Source) != 0)
#else
    if (remove (Source) == 0)
#endif
      {
        Res = aYes;
        StatDelete++;
      }
    return Res;
  }

_Action deleteFolder (char *Source)
  {
    ShowActionFile (CmdDelete, Source, NULL);
    if (rmdir (Source) == 0)
      return aYes;
    else
      return aError;
  }

_Action moveFile (char *Source, char *Dest)
  {
    //int fs, fd;
    _Action go;
    char op;
    bool Exists;
    int i;
    //
    go = aYes;
    Exists = false;
    //fd = open (Dest, O_WRONLY);
    //if (fd >= 0)   // Destination exists
    if (access (Dest, F_OK) == 0)   // Destination exists
      {
        //close (fd);
        Exists = true;
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutString ("Destination ");
        i = StrPosLastCh (Dest, PathDelimiter) + 1;
        PutString (&Dest [i]);
        PutNewLine ();
        PutString ("File exists: ");
        op = GetOptionZ (" |O|verwrite/|S|kip/r|E|name/|A|bort");
        if (op == 'O')
          go = deleteFile (Dest);
        else if (op == 'S')
          go = aSkip;
        else if (op == 'E')
          {
            //go = aYes;
            if (!RenameOld (Dest))
              go = aError;
          }
        else // (op == 'A') || (op == esc)
          go = aAbort;
      }
    if (go == aYes)
      {
        ShowActionFile (CmdMove, Source, Exists ? "** Destination Existed" : NULL); //####????
        if (rename (Source, Dest) == 0)   // success
          StatMove++;
        else   // fail
          {
            go = copyFile (Source, Dest);
            if (go == aYes)
              go = deleteFile (Source);
          }
      }
    return go;
  }

// Searches File for Containing. Returns file index or "-1" if not found
unsigned int SearchFile (char *Filename, char *Containing)
  {
    unsigned int Res;
    int file;
    unsigned int size;
    char *buff;
    //
    // Read file into buff and find 'Containing'
    Res = -1;
    file = FileOpen (Filename, foRead);
    if (file >= 0)
      {
        size = lseek (file, 0, SEEK_END);
        lseek (file, 0, SEEK_SET);
        /*//####
        char s [100], *s_;
        s_ = s;
        StrToStr (&s_, "SearchFile ");
        StrToStr (&s_, Filename);
        StrToStr (&s_, ": Size=");
        IntToStr (&s_, size);
        *s_ = 0;
        LogWrite_ (s);
        *////####
        buff = malloc (size + 1);
        if (buff)
          {
            if (read (file, buff, size) == size)
              {
                buff [size] = 0;
                Res = StrPos (buff, Containing, Setup.FindModeContents);
              }
            free (buff);
          }
        close (file);
      }
    return Res;
  }


//////////////////////////////////////////////////////////////////////////////////
//
// ReadDirSearch (_DirEntry **Tail, char *Target, _ReadDirMode Mode, char *Containing, longint MaxSize)
//
// Read Directory into linked list of _DirEntry **Tail, with enhancements
//   Target: optional mask requiring match for inclusion. Uses StrPos(), (not used if rdmAnalyse)
//   Containing: optional text string needed inside file for inclusion. as above
//   Mode: rdmSingle: No recursion, dir sizes as stat() reports
//         rdmRecurse: Infinite sub Directory read
//         rdmAnalyse: Read sub Directories setting Size to total sub contents

typedef enum {rdmSingle, rdmRecurse, rdmAnalyse} _ReadDirMode;

_ReadDirMode ReadDirMode;
char   *ReadDirTarget;
char   *ReadDirTargetContents;
longint ReadDirMaxSize;

byte ReadDirCallback (_DirEntry *Item, int Depth)
  {
    byte Res;
    //
    Res = ReadDirInList | ReadDirInStats;
    if (ReadDirTarget && StrPos (Item->Name, ReadDirTarget, Setup.FindModeName) == -1)   // Not this item
      Res = 0;
    else
      if (!Item->Directory)   // not a Directory
        if (ReadDirMaxSize && Item->Size > ReadDirMaxSize)   // Not this Size
          Res = 0;
        else
          if (ReadDirTargetContents && ReadDirTargetContents [0])   // Check for Content
              {
                if (SearchFile (Item->Name, ReadDirTargetContents) == -1)   // No matching content
                  Res = 0;
                /*///####
                char s [100], *s_;
                s_ = s;
                StrToStr (&s_, "ReadDirCallback ");
                StrToStr (&s_, Item->Name);
                StrToStr (&s_, " @ ");
                IntToStr (&s_, (longint) (int) Item->Position);
                *s_ = 0;
                LogWrite_ (s);
                *////####
              }
    #ifndef _Windows
    if (Item->SymLink)   // not SymLink
      Res &= ~ReadDirInStats;
    #endif
    if (ReadDirMode == rdmAnalyse)
      if (Depth > 1)
        Res &= ~ReadDirInList;
    return Res;
  }

void ReadDirSearch (_DirEntry **List, char *Target, _ReadDirMode Mode, char *Containing, longint MaxSize)
  {
    ReadDirMode = Mode;
    ReadDirTarget = Target;
    ReadDirTargetContents = Containing;
    ReadDirMaxSize = MaxSize;
    ReadDir (List, (Mode != rdmSingle), ReadDirCallback);
  }
