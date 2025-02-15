////////////////////////////////////////////////////////////////////////////
//
// EDIT TEXT FILE
// ==============
//
// 24 Apr 2018 Implement Clipboard
//             Replace BlockCopy/Move/Delete with Cut/Copy/Paste
//             Add Block Select with Shift Up/Down
//             ^L => Line EraseEOL/Delete
//             ^G => Goto Line Number (was ^L)
// 25 Apr 2018 Keep restore points
// 29 Apr 2018 Implement Undo
// 13 May 2018 Shift <-/-> Indent/Outdent Line/Block
// 15 May 2018 ^Del=Delete word
//             Shift-Del=Delete Line
//             Alt-Del=Delete EOL
//             ^D=Duplicate line
// 15 Jul 2018 Add AltEnter: Insert line after current line
//  8 Aug 2018 Allow Macros on any key/Alt-Key/^Key
// 22 Mar 2019 Syntax Highlight: Add Pascal
// 22 Sep 2019 Syntax Highlight: Add italics for comments & strings
// 04 Dec 2019 Respond immediately to Terminal Size Change
// 13 May 2020 Delete Word: if not on a word, delete the spaces
// 27 Aug 2020 Add EditBufferOverwrite (unsigned int pos, char *Chars, int size)
//             Support Undo for above hence ChangeCase command (^K)
//             Setup: Overwrite mode, Outdent Clears
//             Block Hex <-> Binary
// 24 Dec 2020 ... Copy: No block => Word/bracketted, ^W=Go Mathing Bracket
//  5 Jan 2021 Show matching Target everywhere, tidy title
//  3 Feb 2021 Cut: Noblock => Word/bracketted
//  9 Apr 2021 Fix Copy to EOL. Show "Macro Deleted". Tidy Title
//
// TODO
//
//##^C^C => ?copy bigger than a word
// Edit
// / changed text => no star
// / IsBracket
//   / use StrPos
//   / ^W
//   /   stop on missmatch
//   /   add quote mode to ignore brackets in quotes
//   / ^E
//   /  use char offset and Move?()
//   ?Bug: Press 'esc' while cursor on italic text
// fbc:find
//   / use string iso offset
// Add Aliases for syntax highlight (auto updated from EditSetup.Syntax)

////////////////////////////////////////////////////////////////////////////
//
// SETUP: Editor Settings

typedef struct
  {
    bool Overwrite;
    int TabSize;
    bool AutoIndent;
    bool OutdentErases;
    char Syntax [16];
  } _EditSetup;

_EditSetup EditSetup = {false, 8, true, false, ""};//, spmNoCase};

const char *OverwriteNames [] = {"Insert", "Overwrite", NULL};


////////////////////////////////////////////////////////////////////////////
//
// Editor Clipboard

char *EditorClipboard = NULL;
int EditorClipboardSize = 0;
bool EditorClipboardPartLine = false;


////////////////////////////////////////////////////////////////////////////
//
// Editor Status

typedef enum {fLF, fCRLF, fCR, fXXXX, fBinary} _LineFormat;

typedef struct
  {
    char *Filename;
    char *Buffer;   // Text as single block. LF separated. Zero terminated
    unsigned int BufferSize;   // Size of Buffer
    unsigned int BufferPos;   // Start of current editing line. Index into Buffer
    unsigned int BufferBookmark;
    bool Hex;   // True if Buffer is to be converted FROM Hex/ASCII to Binary before saving
    _LineFormat LineFormat;   // Convert Buffer to (or back to) this format before saving

    int PageX;   // Editing Positon x on page (0 = left)
    int PageY;   // Editing Positon y on page (0 = top)
    int PageX0;   // X Offset

    bool Changed;   // Buffer has changes so save (at least ask)

    unsigned int BlockA, BlockB;
  } _EditState;

_EditState EditState;


////////////////////////////////////////////////////////////////////////////
//
// Editor Working variables

char EditBlockFilename [100];

bool FileWritten;   // true if file (s) has changes. Returned from EditFile ()

bool DrawLine = false;   // Redraw current line
bool DrawHelp = false;   // Redraw help line (bottom of page)
bool DrawTitle;   // Redraw title bar (top page)
int  DrawPageFrom = 0;   // Redraw page from here down: 0=from top, -1=no redraw

unsigned int PosTop, PosBot;

char *EditTargets [16];


////////////////////////////////////////////////////////////////////////////
//
// Editor Restore previous position

typedef struct
  {
    char *Filename;
    unsigned int Pos;
    unsigned int Bookmark;
  } _EditRestore;

_EditRestore EditRestore [16] = {0};

int EditRestoreFind (char *Filename)
  {
    int i;
    //
    i = 0;
    while (i < SIZEARRAY (EditRestore))
      {
        if (EditRestore [i].Filename)
          if (StrCompare (Filename, EditRestore [i].Filename) == 0)   // match
            return i;
        i++;
      }
    return -1;
  }

void EditRestoreRotate (int Index)
  {
    _EditRestore Temp;
    //
    Temp = EditRestore [Index];
    while (Index > 0)
      {
        EditRestore [Index] = EditRestore [Index - 1];
        Index--;
      }
    EditRestore [0] = Temp;
  }

void EditRestoreSet (char *Filename, unsigned int Position)
  {
    int i;
    //
    i = EditRestoreFind (Filename);
    if (i < 0)   // not found
      {
        EditRestoreRotate (SIZEARRAY (EditRestore) - 1);
        StrAssign (&EditRestore [0].Filename, Filename);
      }
    else
      EditRestoreRotate (i);
    EditRestore [0].Pos = Position;
    EditRestore [0].Bookmark = EditState.BufferBookmark;
  }

void EditRestoreFree (void)
  {
    int i;
    //
    for (i = 0; i < SIZEARRAY (EditRestore); i++)
      if (EditRestore [i].Filename)
        free (EditRestore [i].Filename);
  }

bool EditIsEOL (char Ch)
  {
    if (Ch == 0 || Ch == lf)
      return true;
    return false;
  }

char *StrEOL [] = {"\n", "\r\n", "\r", "\n"};

_LineFormat DetermineLineFormat (char *Buffer, int Size)
  {
    unsigned int i, nodd, ncr, nlf;
    _LineFormat Res;
    //
    Res = fLF;
    if (Size)
      {
        nodd = ncr = nlf = 0;
        for (i = 0; i < Size; i++)
          if (Buffer [i] == cr)
            ncr++;
          else if (Buffer [i] == lf)
            nlf++;
          else if (Buffer [i] != tab)
            if (((byte) Buffer [i] < ' ') || ((byte) Buffer [i] >= 0x80))
              nodd++;
        if (Size < 4 * nodd)   // < 75% printable ASCII => HEX
          Res = fBinary;
        else
          {
            if (ncr > 0)
              if (10 * Abs (ncr - nlf) / (ncr + nlf) == 0)   // ncr ~= nlf  => cr/lf
                Res = fCRLF;
              else if (5 * nlf / ncr == 0)   // nlf < ncr / 5  => cr only
                Res = fCR;
          }
      }
    return Res;
  }

void ChangeEOL (char **Lines, _LineFormat lf1, _LineFormat lf2, unsigned int *Size)
  {
    if (lf1 != lf2)
      StrReplaceAll (Lines, StrEOL [lf1], StrEOL [lf2], Size, spmStrict);
  }


////////////////////////////////////////////////////////////////////////////
//
// Editor Undo

typedef struct _UndoData
  {
    struct _UndoData *Prev;
    unsigned int Pos;
    int Inserted;
    int Deleted;
    int Overwritten;
    char *DeletedData;
    //
    unsigned int BufferPos;
    int PageX;
  } _UndoData;

_UndoData* UndoStack = NULL;
bool UndoAddBlock = false;   // Block UndoAdd

void UndoAdd (unsigned int Pos, int Inserted, int Deleted, int Overwritten, char *DeletedData)
  {
    _UndoData *Undo;
    //
    if (UndoAddBlock)
      return;
    EditState.Changed = true;
    DrawTitle = true;
    Undo = (_UndoData *) malloc (sizeof (_UndoData));
    Undo->Prev = UndoStack;
    Undo->BufferPos = EditState.BufferPos;
    Undo->PageX = EditState.PageX;
    Undo->Pos = Pos;
    Undo->Inserted = Inserted;
    Undo->Deleted = Deleted;
    Undo->Overwritten = Overwritten;
    Undo->DeletedData = NULL;
    if (Deleted || Overwritten)   // Make a copy of deleted/overwritten data
      {
        Undo->DeletedData = malloc (Deleted + Overwritten);
        MemMove (Undo->DeletedData, DeletedData, Deleted + Overwritten);
      }
    UndoStack = Undo;
  }

void EditBufferInsert (unsigned int pos, char *Chars, int size);
void EditBufferDelete (unsigned int pos, int size);
void EditBufferOverwrite (unsigned int pos, char *Chars, int size);

bool UndoRemove (void)
  {
    _UndoData *Undo;
    //
    // Anything to do?
    if (UndoStack == NULL)
      return false;
    EditState.Changed = true;
    DrawTitle = true;
    UndoAddBlock = true;
    // Go to location and redraw
    DrawPageFrom = 0;
    // Undoing an Insert?
    if (UndoStack->Inserted)
      EditBufferDelete (UndoStack->Pos, UndoStack->Inserted);
    // Undoing a Delete?
    if (UndoStack->Deleted)
      {
        EditBufferInsert (UndoStack->Pos, UndoStack->DeletedData, UndoStack->Deleted);
        free (UndoStack->DeletedData);
        UndoStack->DeletedData = NULL;
      }
    // Undoing an Overwrite?
    if (UndoStack->Overwritten)
      {
        EditBufferOverwrite  (UndoStack->Pos, UndoStack->DeletedData, UndoStack->Overwritten);
        free (UndoStack->DeletedData);
        UndoStack->DeletedData = NULL;
      }
    EditState.BufferPos = UndoStack->BufferPos;
    EditState.PageX = UndoStack->PageX;
    // Free this stack item and go to next one n stack
    Undo = UndoStack;
    UndoStack = UndoStack->Prev;
    free (Undo);
    UndoAddBlock = false;
    return true;
  }

void UndoFree (void)
  {
    _UndoData *Undo;
    //
    while (true)
      {
        if (UndoStack == NULL)
          break;
        if (UndoStack->DeletedData)
          free (UndoStack->DeletedData);
        Undo = UndoStack;
        UndoStack = UndoStack->Prev;
        free (Undo);
      }
  }


////////////////////////////////////////////////////////////////////////////
//
// Editor Macros

#define MacrosEnabled

#ifdef MacrosEnabled

#define MacroNum   0x200   // Macros can be any key / combo key / Function key / ^Fn Sh-Fn
#define MacroSize  100

byte Macro [MacroSize];
byte *Macros [MacroNum] = {NULL};
int MacroPlay = 0;   // Macro being played (for replay in block)
int MacroRecord = 0;   // Macro being recorded (for Title Bar)
bool MacroBlock = false;
//bool MacrosChanged = false;

#endif


////////////////////////////////////////////////////////////////////////////
//
// Editor Library

const char MessageErrorRead [] = "|*** ERROR:| Can not read file";
const char MessageErrorWrite [] = "|*** ERROR:| Can not write file";
const char MessageErrorHex [] = "|*** ERROR:| Invalid Hex or \"ASCII\" data";

void ConsoleBeep_ (void)
  {
    ConsoleBeep ();
    #ifdef MacrosEnabled
    GetKeyMacro = NULL;   // Stop Macro
    #endif
  }

void EditConsolePrompt (void)
  {
    ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
  }

void SyntaxLoad (char *Name);

bool EditLoadFile (bool Hex)
  {
    int file;
    unsigned int size;
    unsigned int i;
    char *hex;
    bool OK;
    //
    EditState.Changed = false;
    DrawTitle = true;
    if (EditState.Buffer)
      free (EditState.Buffer);
    EditState.BufferSize = 0;
    OK = false;
    file = FileOpen (EditState.Filename, foRead);
    if (file >= 0)
      {
        EditConsolePrompt ();
        PutString ("Loading ");
        PutString (EditState.Filename);
        PutString (" ...");
        GetKey ();
        size = lseek (file, 0, SEEK_END);
        if (size + 1 == 0)   // max uint => stream
          size = 0x1000000;   // so choose a sensible size
        EditState.BufferSize = size;
        lseek (file, 0, SEEK_SET);
        EditState.Buffer = (char *) malloc (EditState.BufferSize + 1);
        if (EditState.Buffer)
          {
            size = read (file, EditState.Buffer, EditState.BufferSize);
            if (size != EditState.BufferSize)
              StrCopy (Message, (char *) MessageErrorRead);
            //else
              {
                if (size != EditState.BufferSize)
                  EditState.BufferSize = size;
                EditState.Buffer [EditState.BufferSize] = 0;   // terminate last line
                OK = true;
                // Test if text file (and how lines are terminated)
                if (Hex)
                  EditState.LineFormat = fBinary;
                else
                  EditState.LineFormat = DetermineLineFormat (EditState.Buffer, EditState.BufferSize);
                EditState.Hex = EditState.LineFormat == fBinary;
                if (EditState.LineFormat == fBinary)
                  {
                    SyntaxLoad ("html");
                    hex = DataToHex ((byte *) EditState.Buffer, EditState.BufferSize, &i, ConsoleSizeX - 1);
                    free (EditState.Buffer);
                    EditState.Buffer = hex;
                    EditState.BufferSize = i;   // don't include the 0 terminator
                    EditState.Hex = true;
                    if (hex == NULL)
                      OK = false;
                  }
                else
                  {
                    // Change to LF format (and remove stray cr)
                    ChangeEOL (&EditState.Buffer, EditState.LineFormat, fLF, &EditState.BufferSize);
                    StrReplaceAll (&EditState.Buffer, "\r", NULL, &EditState.BufferSize, spmStrict);
                  }
              }
          }
        close (file);
      }
    else
      StrCopy (Message, (char *) MessageErrorRead);
    if (!OK)
      ConsoleBeep_ ();
    return OK;
  }

void EditBufferPosMove (unsigned int Dest);
unsigned int EditBufferPosCalc (unsigned int pos, int Lines, int *y);

bool EditWriteToFile (char *Filename, char *Data, unsigned int Size)   // return true if written successfully
  {
    int file;
    bool Result;
    char *Backupname;
    //
    if (Filename == NULL)
      return false;
    if (Filename [0] == 0)
      return false;
    // Make Backup
    if (FileExists (Filename))   // file exists
      {
        Backupname = NULL;
        StrAssign (&Backupname, Filename);
        Backupname [StrLength (Backupname) - 1] = '~';
        if (StrCompare (Filename, Backupname))   // not editing the backup
          {
            if (FileExists (Backupname))   // file exists
              remove (Backupname);
            rename (Filename, Backupname);
          }
        StrAssign (&Backupname, NULL);
      }
    // Write new file
    Result = false;
    file = FileOpen (Filename, foWrite);
    if (file >= 0)
      {
        if (write (file, Data, Size) == Size)
          {
            Result = true;
            if (Log)
              {
                LogWrite  ("Written ");
                Backupname = GetCurrentWorkingDirectory ();
                LogWrite (Backupname);
                Backupname [0] = PathDelimiter;
                Backupname [1] = 0;
                LogWrite (Backupname);
                LogWrite_ (Filename);
                StrAssign (&Backupname, NULL);
              }
          }
        close (file);
      }
    if (!Result)
      StrCopy (Message, (char *) MessageErrorWrite);
    return Result;
  }

bool EditSaveFile (char *Filename)
  {
    bool ok;
    unsigned int i;
    char *x;
    byte *raw;
    //
    ok = true;
    // Convert Hex to raw if EditHex
    if (EditState.Hex)
      {
        x = EditState.Buffer;
        i = StrGetHexAscii (&x, NULL, 0);
        if (*x)   // junk after last hex
          i = -1;
        if (i == -1)
          {
            ok = false;
            StrCopy (Message, (char *) MessageErrorHex);
            EditBufferPosMove (EditBufferPosCalc (x - EditState.Buffer + 1, -1, NULL));
            EditState.PageX = x - EditState.Buffer - EditState.BufferPos;
          }
        else
          {
            raw = (byte *) malloc (i);
            x = EditState.Buffer;
            StrGetHexAscii (&x, raw, i);
            // Write block to file
            if (EditWriteToFile (Filename, (char *) raw, i))
              {
                EditState.Changed = false;
                DrawTitle = true;
              }
            else
              ok = false;
            free (raw);
          }
      }
    else
      {
        // Write to file
        ChangeEOL (&EditState.Buffer, fCRLF, fLF, &EditState.BufferSize);   // correct any residual CRLF (from another editor)
        ChangeEOL (&EditState.Buffer, fLF, EditState.LineFormat, &EditState.BufferSize);   // convert to required/original format
        EditState.BlockA = EditState.BlockB = 0;
        EditState.BufferSize = StrLength (EditState.Buffer);
        if (EditWriteToFile (Filename, EditState.Buffer, EditState.BufferSize))
          {
            EditState.Changed = false;
            DrawTitle = true;
          }
        else
          ok = false;
        ChangeEOL (&EditState.Buffer, EditState.LineFormat, fLF, &EditState.BufferSize);   // convert to required/original format
      }
    if (ok)
      {
        FileWritten = true;
        StrCopy (Message, "File Saved");
      }
    return ok;
  }

unsigned int EditBufferPosCalc (unsigned int pos, int Lines, int *y)
  {
    unsigned int posPrev;
    //
    posPrev = pos;
    while (Lines > 0)
      {
        if (pos >= EditState.BufferSize)   // no end of line \n
          {
            pos = posPrev;
            break;
          }
        if (EditIsEOL (EditState.Buffer [pos]))   // end of line found
          {
            Lines--;
            if (y)
              (*y)++;
            pos ++; //= EditEOLSize ();
            posPrev = pos;
          }
        else
          pos++;
      }
    while (Lines < 0)
      {
        if (pos)
          pos--;
        if (pos == 0 || EditIsEOL (EditState.Buffer [pos - 1]))
          {
            Lines++;
            if (y)
              (*y)--;
          }
      }
    return pos;
  }

void EditBufferGoStartLine (unsigned int *pos)
  {
    *pos = EditBufferPosCalc (*pos + 1, -1, NULL);
  }

void EditBufferPosMove (unsigned int Dest)
  {
    unsigned int Pos;
    //
    while (true)
      {
        Pos = EditState.BufferPos;
        EditState.BufferPos = EditBufferPosCalc (EditState.BufferPos, +1, &EditState.PageY);
        if (Pos == EditState.BufferPos)
          break;
        if (EditState.BufferPos >= Dest)
          break;
      }
    while (Dest < EditState.BufferPos)
      EditState.BufferPos = EditBufferPosCalc (EditState.BufferPos, -1, &EditState.PageY);
    /*if (EditState.BufferPos != Dest)  //####DEBUG
      {
        ConsoleLine (ConsoleSizeY - 1, ColWhite, ColRed);
        PutString ("ERROR: EditBufferPosMove: EditBufferPos = ");
        PutInt (EditState.BufferPos, 0);
        PutString (". Dest = ");
        PutInt (Dest, 0);
        GetKeyWait (true);
        DrawHelp = true;
      }*/
    EditState.PageX = Dest - EditState.BufferPos;
  }

unsigned int EditBufferGetCurrentLine (void)
  {
    unsigned int Pos, Pos_;
    unsigned int Line;
    //
    Pos = 0;
    Line = 0;
    while (true)
      {
        if (Pos >= EditState.BufferPos)
          break;
        Pos_ = Pos;
        Pos = EditBufferPosCalc (Pos, +1, NULL);
        if (Pos == Pos_)
          break;
        Line++;
      }
    return Line;
  }

int EditLineLength (unsigned int pos)
  {
    int l;
    //
    l = 0;
    while (true)
      {
        if (EditIsEOL (EditState.Buffer [pos + l]))
          break;
        l++;
      }
    return l;
  }

void EditPointerMove (int Lines)
  {
    EditState.BufferPos = EditBufferPosCalc (EditState.BufferPos, Lines, &EditState.PageY);
  }

void EditAdjustBlockAndSwap (unsigned int pos, int adjustment)
  {
    // Adjust Block
    if (EditState.BlockA != EditState.BlockB)
      {
        if (pos < EditState.BlockA)
          EditState.BlockA += adjustment;
        if (pos < EditState.BlockB)
          EditState.BlockB += adjustment;
      }
    // Adjust Editor Positions
    if (pos < EditState.BufferPos)
      EditState.BufferPos += adjustment;
    if (pos < EditState.BufferBookmark)
      EditState.BufferBookmark += adjustment;
  }

void EditBufferInsert (unsigned int pos, char *Chars, int size)
  {
    char *EditBuffer_;
    //
    UndoAdd (pos, size, 0, 0, NULL);
    //
    EditBuffer_ = (char *) malloc (EditState.BufferSize + size + 1);
    if (EditBuffer_ == NULL)
      ConsoleBeep_ ();
    else
      {
        MemMove (EditBuffer_, EditState.Buffer, pos);
        if (Chars)
          MemMove (EditBuffer_ + pos, Chars, size);
        else
          MemSet (EditBuffer_ + pos, ' ', size);
        MemMove (EditBuffer_ + pos + size, EditState.Buffer + pos, EditState.BufferSize - pos);
        free (EditState.Buffer);
        EditState.Buffer = EditBuffer_;
        EditState.BufferSize += size;
        EditState.Buffer [EditState.BufferSize] = 0;   // terminate last line
        EditAdjustBlockAndSwap (pos, size);   // Move definition of Block
      }
  }

void EditBufferDelete (unsigned int pos, int size)
  {
    UndoAdd (pos, 0, size, 0, &EditState.Buffer [pos]);
    //
    MemMove (EditState.Buffer + pos, EditState.Buffer + pos + size, EditState.BufferSize - pos - size);
    EditState.BufferSize -= size;
    EditState.Buffer [EditState.BufferSize] = 0;   // terminate last line
    EditAdjustBlockAndSwap (pos, -size);   // Correct Block & Bookmarks
    EditBufferGoStartLine (&EditState.BlockA);   // Adjust in case of merged Block lines
    EditBufferGoStartLine (&EditState.BlockB);
  }

void EditBufferOverwrite (unsigned int pos, char *Chars, int size)
  {
    UndoAdd (pos, 0, 0, size, &EditState.Buffer [pos]);
    //
    MemMove (EditState.Buffer + pos, Chars, size);
  }

bool IsIdentifier (char ch)
  {
    if (ch >= '0')
      if (ch <= '9')
        return true;
    if (ch == '_')
      return true;
    ch &= ~('a' ^ 'A');   // make upper case
    if (ch >= 'A')
      if (ch <= 'Z')
        return true;
    return false;
  }

int EditCurrentChar (int Offset)
  {
    unsigned int pos;
    //
    pos = EditState.BufferPos + EditState.PageX + Offset;
    if (pos > EditState.BufferSize)
      return -1;
    return EditState.Buffer [pos];
  }

int EditCurrentWord (void)
  {
    int Result;
    //
    while (EditState.PageX > 0 && IsIdentifier (EditCurrentChar (-1)))   //  EditState.Buffer [EditState.BufferPos + EditState.PageX - 1])))
      EditState.PageX--;
    Result = 0;
    while (true)
      {
        if (!IsIdentifier (EditCurrentChar (Result)))
          break;
        Result++;
      }
    return Result;
  }

int EditCurrentWord_ (void)
  {
    int Result;
    //
    Result = EditCurrentWord ();
    if (Result == 0)
      if (EditCurrentChar (0) == ' ')
        {
          while (EditState.PageX > 0 && EditCurrentChar (-1) == ' ')
            EditState.PageX--;
          while (EditCurrentChar (Result) == ' ')
            Result++;
        }
    return Result;
  }

// Look for matching bracket/quote. Return new Index if found
unsigned int StrFindMatchingBracket (unsigned int Index, bool SingleLine)
  {
    unsigned int Res;
    int b0, b;
    int c;
    int Dir;
    int Sum;
    bool Quote;
    //
    Res = Index;
    b0 = IsBracket (EditState.Buffer [Index]);
    if (b0)
      {
        Dir = Sign (b0);
        Sum = 0;
        Quote = false;
        while (true)
          {
            c = -1;
            if (Res >= EditState.BufferSize)   // outside the Buffer
              break;
            c = EditState.Buffer [Res];
            if (c <= 0 || (SingleLine && EditIsEOL (c)))
              break;
            if (IsQuote (c) == 2)   // only look at double quotes
              Quote = !Quote;
            if (!Quote)
              {
                b = IsBracket (c);
                if (b)
                  if (b == b0)
                    Sum++;
                  else if (b == -b0)
                    if (--Sum == 0)   // Found
                      return Res;
              }
            Res += Dir;
          }
      }
    // exits on not found
    return Index;
  }

void EditFindMatchingBracket (void)
  {
    int Index, IndexRes;
    //
    Index = EditState.BufferPos + EditState.PageX;
    IndexRes = StrFindMatchingBracket (Index, false);
    if (IndexRes == Index)
      ConsoleBeep_ ();
    else
      EditBufferPosMove (IndexRes);
  }

void SetLineColour (int y, unsigned int pos)
  {
    ConsoleColourFG (Colours [ColBodyFG]);
    ConsoleCursor (0, y + 1);
    if ((pos >= EditState.BlockA) && (pos < EditState.BlockB))   // inside block
      ConsoleColourBG (Colours [ColBodyBGSel]);
    else
      ConsoleColourBG (Colours [ColBodyBG]);
    PutCharN (' ', ConsoleSizeX - 1);
    ConsoleCursor (0, y + 1);
  }


////////////////////////////////////////////////////////////////////////////
//
// Syntax Highlighting

typedef enum {seTypes, seReserved, sePredef, seSymbols, seString, seComment, seZZZZ} _SyntaxElement;

#define sElementType 0x0F
#define sQuote 0x40
#define sEOL 0x80

byte ElementToColour (byte Element)
  {
    return (Colours [ColSyntaxHighlight + (Element & sElementType)]);
    //return ColGray;
  }

bool SyntaxCaseInsensitive = false;
bool SyntaxBrackets = false;

typedef struct _HighlightItem
  {
    struct _HighlightItem *Next;
    char *Text;
    byte SyntaxElement;   // bits0-3=_SyntaxElement bit6=Quote bit7=ExtendEndLine
  } _HighlightItem;

_HighlightItem *HighlightItems = NULL;

void HighlightItemsFree (void)
  {
    _HighlightItem *Item, *Item_;
    //
    Item = HighlightItems;
    while (Item)
      {
        Item_ = Item->Next;
        free (Item->Text);
        free (Item);
        Item = NULL;
        Item = Item_;
      }
    HighlightItems = NULL;
  }

//bool SyntaxUpCase [] = {false, false, true, true};   // Language is case insensitive?

typedef enum {sgNone, sgParams, sgItems, sgZZZZ} _SyntaxGroup;
//const char *SyntaxGroupTags [] = {"[Params]", "[Items]", NULL};

void SyntaxLoad (char *Name)
  {
    char *Filename;
    char *Path;
    _TextFile file;
    char *Line;
    char *x;
    _SyntaxGroup Group;
    _HighlightItem *Item;
    char Object [128], c;
    int Val;
    bool Error;
    //
    TextFileInit (&file);
    HighlightItemsFree ();
    Item = (_HighlightItem *) &HighlightItems;
    if (Name [0])   // Syntax Specified exists
      {
        Filename = malloc (MaxPath);
        Val = -1;   // flag AND Alias index
        x = Filename;
        StrToStr (&x, Name);
        while (true)
          {
            StrToStr (&x, ".syntax");
            *x = 0;
            StrToLower (Filename);
            Path = FindFileResource (Filename);
            if (Path || Val >= 0)
              break;
            Val = StringArraySearch (ExtensionAlias, SIZEARRAY (ExtensionAlias), Name);
            if (Val < 0)
              break;
            Line = StrPosCh_ (ExtensionAlias [Val], tab);
            if (Line == NULL)
              break;
            x = Filename;
            StrToStr (&x, Line + 1);
          }
        if (Path && TextFileOpen (&file, Path, foRead))
          {
            Error = false;
            Group = sgNone;
            while (true)
              {
                Line = TextFileReadln (&file, false);
                if (Line == NULL || Error)
                  break;
                if (Line [0])
                  if (Line [0] == '/' && Line [1] == '/' && Line [2] == '/')   // Comment
                    ;
                  else  // not a comment
                    {
                      x = Line;
                      while (*x == ' ')
                        x++;
                      c = StrGetItem (&x, Object, SIZEARRAY (Object), false);
                      if (StrSame_ (Object, "[Params]"))
                        Group = sgParams;
                      else if (StrSame_ (Object, "[Items]"))
                        Group = sgItems;
                      else switch (Group)
                        {
                          case sgNone:
                            break;
                          case sgParams:
                            if (StrSame_ (Object, "CaseSensitive"))
                              SyntaxCaseInsensitive = false;
                            else if (StrSame_ (Object, "CaseInsensitive"))
                              SyntaxCaseInsensitive = true;
                            else if (StrSame_ (Object, "Brackets"))
                              SyntaxBrackets = true;
                            else
                              Error = true;
                            break;
                          case sgItems:
                            Val = StrGetHex (&x);
                            if (Val >= 0 && c == tab)   // valid hex
                              {
                                Item->Next = (_HighlightItem *) malloc (sizeof (_HighlightItem));
                                Item = Item->Next;
                                Item->Next = NULL;
                                Item->Text = NULL;
                                StrAssign (&Item->Text, Object);
                                Item->SyntaxElement = Val;
                              }
                            else
                              Error = true;
                            break;
                        }
                    }
                if (Error)
                  {
                    x = Message;
                    StrToStr (&x, "|ERROR| in Syntax File: ");
                    if (StrLength (Line) + (x - Message) < sizeof (Message))
                      StrToStr (&x, Line);
                    *x = 0;
                    break;
                  }
              }
            TextFileClose (&file);
          }
        else
          if (Message [0] == 0)
            StrCopy (Message, "No Syntax File");
        free (Filename);
        if (Path)
          free (Path);
      }
  }

//#### fix this, put into console.c and add unicode
int CalculateConsoleX (char St [], int Index)   // Calculate Console x position to St [Index]
  {
    int i;
    int x;
    char c;
    //
    i = 0;
    x = 0;
    while (true)
      {
        if (i >= Index)
          break;
        c = St [i];
        if (EditIsEOL (c))
          break;
        if (c == tab)
          x = ((x / EditSetup.TabSize) + 1) * EditSetup.TabSize;
        else
          x++;
        i++;
      }
    return x;
  }

#define cError  0x0100
#define cTarget 0x0200

void PutCharColour (byte Ch, word Col, int *x, int Offset)   // Put Coloured character
  {
    int FG, BG, BGOld;
    //
    if (*x >= Offset)
      {
        FG = Colours [ColBodyFG];
        BG = ConsoleBG;
        BGOld = BG;
        if (Col & cError)   // Error flag
          {
            FG = Colours [ColErrorFG1];
            BG = Colours [ColErrorBG];
          }
        else
          {
            if (Col & cTarget)   // Highlight current search target
              BG ^= Colours [ColBodyBG] ^ Colours [ColBodyBGSel];
            FG = Col & ColFGMax;
          }
        if (FG != ConsoleFG)
          ConsoleColourFG (FG);
        if (BG != ConsoleBG)
          ConsoleColourBG (BG);
        PutCharPlain (Ch);
        if (BG != BGOld)
          ConsoleColourBG (BGOld);
      }
    (*x)++;
  }

void PutStringColour (char St [], word Colour [], int Offset)   // Put coloured string and expand tabs
  {
    int x;   // x Position in a zero based console
    int Index;   // index into St
    byte c;
    word Col;
    //
    Index = 0;
    x = 0;
    while (true)
      {
        if (ConsoleX > ConsoleSizeX)
          break;
        c = St [Index];
        if (EditIsEOL (c))
          {
            PutCharPlain (' ');   // remove leftover cha after a delete
            break;
          }
        Col = Colour [Index];
        if (c == tab)
          do   // Step onto next tab
            PutCharColour (' ', Col, &x, Offset);
          while (x % EditSetup.TabSize);
        else
          PutCharColour (c, Col, &x, Offset);
        Index++;
      }
  }

bool HighlightItemCompare (char *St, const _HighlightItem *Item)   // Does *St match HighlightItem
  {
    bool Identifier;
    int i;
    bool x;
    char c;
    //
    if (Item->Text == NULL)
      return false;
    Identifier = IsIdentifier (St [0]);
    i = 0;
    while (true)
      {
        c = St [i];
        if (Item->Text [i] == 0)   // Match
          return !Identifier || !IsIdentifier (c);   // whole Identifiers only
        if (Item->Text [i] == ' ')   // SPACE == any white space
          x = (c <= ' ');
        else if (SyntaxCaseInsensitive) //SyntaxUpCase [EditSetup.Syntax]
          x = UpCase (c) == UpCase (Item->Text [i]);
        else
          x = (c == Item->Text [i]);
        if (!x)
          return false;
        i++;
      }
  }

const char *CharReverseSet = "{([<>])}";

char CharReverse (char Ch)
  {
    int i;
    //
    i = StrPosCh ((char *) CharReverseSet, Ch);
    if (i < 0)
      return Ch;
    return CharReverseSet [7 - i];
  }

unsigned int StrMatchLen;

unsigned int StrPosFromLine (char *St, char *Target, unsigned int From, _StrPosMode StrPosMode)
  {
    while (true)
      {
        StrMatchLen = StrMatch (&St [From], Target, 0, StrPosMode);
        if (StrMatchLen)   // Found
          return From;
        if (St [From] == 0 || St [From] == lf)   // Not found
          return -1;
        From++;
      }
  }

unsigned int LineSearch (char *St, char *Target, unsigned int From)
  {
    int Len, a;
    char *Target_;
    //
    // Make reverse image of Target
    Len = StrLength (Target);
    Target_ = malloc (Len + 1);
    for (a = 0; a < Len; a++)
      Target_ [a] = CharReverse (Target [Len - a - 1]);
    Target_ [Len] = 0;
    // Search for this in St
    From = StrPosFromLine (St, Target_, From, spmStrict);
    // Finish up
    free (Target_);
    return From;
  }

_HighlightItem *HighlightItemSearch (char St [], int *Index)
  {
    int Len;
    unsigned int i;
    _HighlightItem *Item;
    //
    Item = HighlightItems;
    Len = 0;
    while (true)
      {
        if (Item == NULL)
          break;
        if (HighlightItemCompare (&St [*Index], Item))   // Match
          {
            Len = StrLength (Item->Text);
            break;
          }
        Item = Item->Next;
      }
    if (Len)   // Found
      {
        if (Item->SyntaxElement & sEOL)   // Highlight to end of line
          while (!EditIsEOL (St [*Index]))
            (*Index)++;
        else if (Item->SyntaxElement & sQuote)   // Highlight to next occurrence
          {
            i = LineSearch (St, Item->Text, *Index + Len);
            if (i == -1)   // Not Found => Error
              {
                *Index += Len;
                Item = NULL;
              }
            else
              *Index = i + Len;
          }
        else   // Highlight fixed length string
          *Index += Len;
      }
    else if (IsIdentifier (St [*Index]))   // St is an identifier / reserved word
      while (IsIdentifier (St [*Index]))   // so step past entire identifier
        (*Index)++;
    else   // Not in Item list, not an identifier => Error
      {
        Item = NULL;
        (*Index)++;
      }
    return Item;
  }

struct
  {
    int Bracket;
    int Pos;
  } Brackets [32];   // Stack of open brackets

void CheckBrackets (char St [], word Colour []) //, int PosX)
  {
    int x, b;
    int Bracket;
    //
    Bracket = 0;
    x = 0;
    while (!EditIsEOL (St [x]))
      {
        b = 0;
        if (Colour [x] == ElementToColour (seSymbols))   // Ignore comments and any other non symbols
          b = IsBracket (St [x]);
        if (Abs (b) >= 3)   // ignore {}
          b = 0;
        if (b > 0)   // Open Bracket so save it
          {
            Brackets [Bracket].Bracket = b;
            Brackets [Bracket].Pos = x;
            if (Bracket + 1 < SIZEARRAY (Brackets))
              Bracket++;
          }
        else if (b < 0)   // Close Bracket so check it
          if (Bracket == 0)   // No Bracket to match
            Colour [x] = cError | Colours [ColBodyFG];
          else   // Does closing bracket match
            {
              Bracket--;
              if (Brackets [Bracket].Bracket != -b)   // Mismatching )
                {
                  Colour [x] = cError | Colours [ColBodyFG];
                  Colour [Brackets [Bracket].Pos] = cError | Colours [ColBodyFG];
                }
            }
        x++;
      }
    while (Bracket)   // Unclosed Brackets
      Colour [Brackets [--Bracket].Pos] = cError | Colours [ColBodyFG];
  }

void HighlightTarget (char St [], word Colour [], int Offset, char *Target)
  {
    int a, i;
    //
    if (Target && Target [0])
      {
        a = Offset;
        while (true)
          {
            a = StrPosFromLine (St, Target, a, Setup.FindModeContents);
            if (a < 0)
              break;
            if (a - Offset >= ConsoleSizeX)
              break;
            //for (i = 0; Target [i]; i++)
            for (i = 0; i < StrMatchLen; i++)
              Colour [a + i] |= cTarget;
            a++;
          }
      }
  }

void EditDrawHighlightLine (char *Line, int Offset)
  {
    word *StrColour;
    int x, x_, Col;
    _HighlightItem const *Item;
    int Len;
    //
    Len = 0;
    while (!EditIsEOL (Line [Len]))
      Len++;
    StrColour = (word *) malloc (Len * sizeof (word)); // (Offset + ConsoleSizeX);
    for (x = 0; x < Len; x++)
      StrColour [x] = Colours [ColBodyFG];
    //if (EditSetup.Syntax != slNone)
      {
        x = 0;
        while (true)
          {
            while ((Line [x] == ' ') || (Line [x] == '\t'))
              x++;
            if (EditIsEOL (Line [x]))
              break;
            x_ = x;
            Item = HighlightItemSearch (Line, &x);
            Col = -1;
            if (x == x_)   // Error
              {
                Col = cError | Colours [ColBodyFG];
                x++;
              }
            if (Item)   // Colouring to do
              Col = ElementToColour (Item->SyntaxElement);
            if (Col >= 0)   // Copy Colour to StrColour
              while (true)
                {
                  if (x_ >= Len)
                    break;
                  if (x_ >= x)
                    break;
                  StrColour [x_] = Col;
                  x_++;
                }
          }
      }
    if (SyntaxBrackets)
      CheckBrackets (Line, StrColour); //, EditState.PageX);
    HighlightTarget (Line, StrColour, Offset, EditTargets [0]);
    PutStringColour (Line, StrColour, Offset);
    //PutStringN (&Line [Offset], ConsoleSizeX);
    free (StrColour);
  }


////////////////////////////////////////////////////////////////////////////
//
// Editor Main Functions

void EditDrawLine (void)
  {
    ConsoleSizeX--;
    SetLineColour (EditState.PageY, EditState.BufferPos);
    //PutStringN (&EditBuffer [EditBufferPos + PageX0], ConsoleSizeX);
    EditDrawHighlightLine (&EditState.Buffer [EditState.BufferPos], EditState.PageX0);
    ConsoleSizeX++;
    DrawLine = false;
  }

void EditDrawHelp (void)
  {
    ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
    //PutStringHighlight ("|PgeUp|/|Dn Home End Ins Del ^K ^X  ^F|ind-|^N|ext-|^P|rev  |^B|lock-|^G|o-|^C|opy-M|^o|ve-|^D|elete  |^E|xchange  |^H|elp  Esc", ColourHelpFG1, ColourHelpFG2);
    PutStringHighlight ("|Up|/|Dn|..  |Ins Del  ^F|ind-|^N|ext-|^P|rev  |^B|lock  |^_|=Help  |Esc|", Colours [ColHelpFG2]);
    DrawHelp = false;
  }

/*
const char *EditHelpPage [] =
  {
    "_FBC - TEXT EDITOR",
    "",
    "_Move around:",
    "|arrows|\tMove around",
    "|PgeUp|\tTop Page / Page Up",
    "|PgeDn|\tBottom Page / Page Down",
    "|Home|\tStart of Line",
    "|Home Home|\tStart of File",
    "|End|\tEnd of Line",
    "|End End|\tEnd of File",
    "|Sh-|(above)\tDefine / Extend Block (whole Lines)",
    "",
    "|^ <-| or |^T|\tMove previous word",
    "|^ ->| or |^Y|\tMove next word",
    "",
    "|^E|\teXchange position with the Bookmark",
    "|^G|o to\tGo to Line Number",
    "|^W|\tGo to Matching Bracket |() [] {}|",
    "",
    "_Search:",
    "|^F|ind\tFind text:",
    "	  |^F| changes Find Mode: Strict, Case Insensitive OR Allow wilds (|#@&?|)",
    "	  |up down| goes thru search history",
    "|^N|ext\tFind next occurrence",
    "|^P|rev\tFind previous occurrence",
    "",
    "_Make Changes:",
    "\tJust type characters to insert text",
    "|Ins|\tInsert Space",
    "|Del|\tDelete character at cursor",
    "|BackSpace|\tDelete character left of cursor / Append lines",
    "|Enter|\tNew Line / Split Line",
    "|Alt-Enter|\tNew Line (no split). Same as |End|, |Enter|",
    "|Tab|\tInsert Tab character",
    "",
    "|^K|\tChange Case of Word: lower > Mixed > UPPER",
    "|^A|\tAuto Complete identifier (Based on preceeding text. Repeatable)",
    "|^D|\tDuplicate Line",
    "|^Del|\tDelete word OR Spaces",
    "|Alt-Del| or |^L|\tDelete to End of Line",
    "|Sh-Del|\tDelete Line",
    "|^up|\tMove line up",
    "|^down|\tMove line down",
    "|Sh <-| or |^]|\tOutdent Line / Block",
    "|Sh ->| or |^\\|\tIndent Line / Block",
    "",
    "_Clipboard Comands:",
    "|^C|\tBlock Copy to Clipboard. If no Block defined, use Word / Quote / Bracketted",
    "|^X|\tBlock Cut to Clipboard. \"",
    "|^V|\tPaste Clipboard at cursor",
    "",
    "_Block Comands:",
    //"|Sh-up|/|down|\tDefine / Extend Block (whole Lines)",
    "|^B^B|\tDefine Block begin / end",
    "|^B^A|\tDefine Block as entire file",
    "|^B^G|\tGo to Block",
//    "|^B^C|\tCopy Block to cursor",
//    "|^B^O|\tmOve Block to cursor",
//    "|^B^D|\tDelete Block",
    "|^B^T|\tBlock expand/compress Tabs",
    "|^B^H|\tBlock convert between Hex and Hex/\"ASCII\"",
    "|^B^I|\tBlock convert between Hex and ASCII",
    "|^B^F|\tBlock Format: Fill lines with words, Break lines, Preserve paragraph breaks",
    "|^B^R|\tRead Block from file",
    "|^B^W|\tWrite Block to file",
    "|^B^C|\tCompare Block with Clipboard",
    "\t  Specify the Sync Threshold (number ofcharacters)",
    "\t  Result will be in Clipboard",
    "\t  Common sections are included",
    "\t  Text unique to the original Clipboard are shown |<####example####>|",
    "\t  Text unique to the Block are shown |<<####example####>>|",
    "",
    #ifdef MacrosEnabled
    "_Macros:",
    "|^R Key|\tRecord subsequent keystrokes to a Key/Alt-Key/^Key. |^R| finishes",
    "\tThese can contain any commands and/or text. Create super commands",
    //"|^R Key ^R|\tRecord the identifier at cursor",
    "|^R Key ^R|\tClear the macro on Key",
    "|Key|\tReplay recorded keystrokes. If in Block, it repeats until out of Block",
    //"|^^|\tCheck character at cursor, stop Macro if not a match",
    "",
    #endif
    "_Other:",
    "|^Z|\tUndo (unlimited)",
    "set|^U|p\tEditor settings",
    "	Parameters: (use |Up| / |Down|)",
    "	  {1} Type Mode - Insert OR Overwrite (where typing replaces rather than inserts)",
    "	  {2} Tab Size - Defines how tabs are displayed AND converted (see |^B^T|)",
    "	  {3} Auto Indent - After |Enter| spaces are added to match previous line",
    "	  {4} Outdent Destructive - Allows non blank characters to be deleted while Outdenting",
    "	  {5} Syntax Highlight - Manually select sytax (normally set by file\'s extension)",
    ""
    "|^S|ave\tSave File",
    "|Esc|\tQuit:",
    "\t  Allows renaming the file before saving",
    "\t  |Enter| to save",
    "\t  |Esc| returns to editor",
    "\t  |^D| to Discard changes and exit",
    "\t  |^F| to change the file format: |LF| > |CR+LF| > |CR|",
    //NULL
  };
*/


///////////////////////////////////////////////////////////////////////////////
//
// Draw Page

const char *LineFormatName [] = {"LF", "CR+LF", "CR"};

void CharToStr_ (char **St, word Ch)
  {
    if (Ch < ' ')
      {
        CharToStr (St, '^');
        CharToStr (St, Ch | '@');
      }
    else if (Ch == ' ')
      StrToStr (St, "<SPACE>");
    else if (Ch < 0x80)
      CharToStr (St, Ch);
    else // (Ch >= 0x80)
      {
        if ((Ch >= KeyF1) && (Ch < KeyF1 + 12))
          {
            CharToStr (St, 'F');
            IntToStr (St, Ch - KeyF1 + 1);
          }
        else if ((Ch >= KeyCntrlF1) && (Ch < KeyCntrlF1 + 12))
          {
            StrToStr (St, "^F");
            IntToStr (St, Ch - KeyCntrlF1 + 1);
          }
        else if ((Ch >= KeyShiftF1) && (Ch < KeyShiftF1 + 12))
          {
            StrToStr (St, "Sh-F");
            IntToStr (St, Ch - KeyShiftF1 + 1);
          }
        else if ((Ch >= KeyAltA) && (Ch < KeyAltA + 26))
          {
            StrToStr (St, "Alt-");
            CharToStr (St, Ch - KeyAltA + 'A');
          }
        else if ((Ch >= KeyAlt0) && (Ch < KeyAlt0 + 10))
          {
            StrToStr (St, "Alt-");
            CharToStr (St, Ch - KeyAlt0 + '0');
          }
        else
          {
            CharToStr (St, '<');
            IntToHex (St, Ch, 0 | IntToLengthZeros);
            CharToStr (St, '>');
          }
      }
  }

void PutChar_ (word c)
  {
    char s [8], *sp;
    //
    sp = s;
    CharToStr_ (&sp, c);
    *sp = 0;
    PutString (s);
  }

/*
#define MacroShow 10

void PutMacroString (byte *St)
  {
    int i;
    //
    i = 0;
    while (true)
      {
        if (St [i] == 0)
          break;
        if (i >= MacroShow)
          {
            PutString ("..");
            break;
          }
        PutChar_ (St [i++]);
      }
  }
*/

void ShowMacroKey (word Num, int Col)
  {
    int Col_;
    //
    Col_ = ConsoleFG;
    ConsoleColourFG (Col);
    PutChar_ (Num);
    //PutInt (Num + 1, 0);
    ConsoleColourFG (Col_);
    PutCharPlain (' ');
  }

void PutSeparator (void)
  {
    byte c;
    //
    c = ConsoleFG;
    ConsoleColourFG (Colours [ColBodyBG]);
    PutString (" | ");
    ConsoleColourFG (c);
  }

void EditDrawTitle (void)
  {
    ConsoleLine (0, Colours [ColTitleFG1], Colours [ColTitleBG]);
    PutString ("EDIT ");
    ConsoleColourFG (Colours [ColTitleFG2]);
    if (EditState.Changed)
      PutCharPlain ('*');
    PutString (EditState.Filename);
    ConsoleColourFG (Colours [ColTitleFG1]);
    PutSeparator ();
    PutString (OverwriteNames [EditSetup.Overwrite]);
    PutSeparator ();
    if (EditState.Hex)
      PutString (" HEX");
    else
      PutString (LineFormatName [EditState.LineFormat]);
    PutSeparator ();
    //PutInt (EditState.BufferPos, IntToLengthCommas);
    if (EditState.BlockA != EditState.BlockB)   // Block defined
      {
        PutInt (EditState.BlockB - EditState.BlockA, IntToLengthCommas);
        PutString (" / ");
      }
    PutInt (EditState.BufferSize, IntToLengthCommas);
    #ifdef MacrosEnabled
    int i;
    ConsoleColourFG (Colours [ColTitleFG1]);
    PutSeparator ();
    for (i = 0; (i < MacroNum) && (ConsoleX < ConsoleSizeX); i++)
      if (GetKeyMacroRecord && (MacroRecord == i))
        ShowMacroKey (i, Colours [ColTitleFG2]);
      else if (Macros [i])
        {
          ShowMacroKey (i, Colours [ColTitleFG1]);
          //PutCharPlain ('=');
          //PutMacroString (Macros [i]);
        }
    #endif
  }

void EditDrawPage (void)
  {
    int Lines;
    unsigned int pos, pos_;
    bool Done;
    int y;
    //unsigned int PosTop;
    //
    Done = false;
    Lines = ConsoleSizeY - 2;
    y = EditState.PageY;
    pos = EditBufferPosCalc (EditState.BufferPos, DrawPageFrom - EditState.PageY, &y);
    if (y == 0)
      PosTop = pos;
    if (y < 0)   // PageY impossible for current EditBufferPos
      EditState.PageY -= y;   // move cursor up to match shifted text after a move
    while (DrawPageFrom < Lines)
      {
        SetLineColour (DrawPageFrom, pos);
        pos_ = EditBufferPosCalc (pos, +1, NULL);
        if (!Done)
          {
            if (pos == EditState.BufferPos)
              EditState.PageY = DrawPageFrom;
            if ((pos == pos_) || (pos + EditState.PageX0 < pos_))
              EditDrawHighlightLine (&EditState.Buffer [pos], EditState.PageX0);
            //PutStringN (&EditBuffer [pos + PageX0], ConsoleSizeX);
          }
        if (pos == pos_)
          Done = true;
        pos = pos_;
        DrawPageFrom++;
      }
    PosBot = pos;
    DrawScrollBar (1, ConsoleSizeY - 2,
                   PosTop, PosBot, EditState.BufferSize,
                   Colours [ColBodyFG], Colours [ColBodyBG]);
                   //ColourTitleBG, ColourBodyBG);
    DrawPageFrom = -1;
    DrawLine = false;
  }


///////////////////////////////////////////////////////////////////////////////
//
// Main Commands

#define CmdFindNext Cntrl('N')
#define CmdFindPrev Cntrl('P')

void EditUndo (void)
  {
    if (!UndoRemove ())
      ConsoleBeep_ ();
  }

//const char crlf_ [] = {cr, lf};
//const char lf_ = lf;
const char Space_ = ' ';
const char Tab_ = tab;

void EditWordNext (void)
  {
    unsigned int Pos;
    bool Flag;
    //
    Pos = EditState.BufferPos + EditState.PageX;
    Flag = false;
    while (true)
      {
        if (Pos >= EditState.BufferSize)
          break;
        if (EditState.Buffer [Pos] <= ' ')
          Flag = true;
        else
          if (Flag)
            break;
        Pos ++;
      }
    EditBufferPosMove (Pos);
  }

void EditWordPrev (void)
  {
    int Pos;
    bool Flag;
    //
    Pos = EditState.BufferPos + EditState.PageX;
    Flag = false;
    while (true)
      {
        if (Pos <= 0)
          break;
        if (EditState.Buffer [Pos - 1] > ' ')
          Flag = true;
        else
          if (Flag)
            break;
        Pos--;
      }
    EditBufferPosMove (Pos);
  }

bool EditInsertChar (int Ch)
  {
    char c;
    bool Done;
    int i;
    //
    Done = true;
    if (((Ch >= ' ') && (Ch < 0x7F)) || (Ch == '\t'))
      {
        c = Ch;
        if (EditSetup.Overwrite && (EditCurrentChar (0) != '\n'))
          EditBufferOverwrite (EditState.BufferPos + EditState.PageX, &c, sizeof (c));
        else   // Insert
          EditBufferInsert (EditState.BufferPos + EditState.PageX, &c, sizeof (c));
        EditState.PageX++;
        DrawLine = true;
      }
    else if ((Ch == KeyEnter) || (Ch == KeyAltEnter))
      {
        if (Ch == KeyAltEnter)
          EditState.PageX = EditLineLength (EditState.BufferPos);
        i = 0;
        if (EditSetup.AutoIndent)
          while (EditState.Buffer [EditState.BufferPos + i] == ' ')
            i++;
        EditBufferInsert (EditState.BufferPos + EditState.PageX, (char *) &lf_, 1);
        EditPointerMove (+1);
        if (i)
          EditBufferInsert (EditState.BufferPos, NULL, i);   // insert spaces for imdent
        EditState.PageX = i;
        DrawPageFrom = EditState.PageY - 1;
      }
    else
      Done = false;
    return Done;
  }

void EditChangeCase (void)
  {
    int l;
    int i;
    int f;   // Flags: bit0 = LowerCase present, bit1 = UpperCase
    char *s;
    bool first;
    //
    l = EditCurrentWord ();
    if (l)
      {
        s = malloc (l);
        f = 0x00;
        MemMove (s, EditState.Buffer + EditState.BufferPos + EditState.PageX, l);
        for (i = 0; i < l; i++)
          if (IsAlpha (s [i]))
            if (s [i] & ('A' ^ 'a'))
              f |= 0x01;
            else
              f |= 0x02;
        if (f)
          {
            first = true;
            for (i = 0; i < l; i++)
              if (IsAlpha (s [i]))
                {
                  if ((f == 0x03) || ((f == 0x01) && first))
                    s [i] &= ~('A' ^ 'a');
                  else
                    s [i] |= ('A' ^ 'a');
                  first = false;
                }
            EditBufferOverwrite (EditState.BufferPos + EditState.PageX, s, l);
            EditState.PageX += l;
            DrawLine = true;
          }
        free (s);
      }
    else
      ConsoleBeep_ ();
  }

void EditAutoComplete (bool Repeat)   // search for a previous word that starts the same as this one
  {
    static int LenSearch;
    static unsigned int Pos;
    static char PastResults [256];
    int LenA, LenB, a, l;
    char s [100];
    char *cp;
    //
    // Determine the "key"
    LenA = EditCurrentWord ();
    if (!Repeat || (Pos == -1))
      {
        LenSearch = LenA;
        Pos = EditState.BufferPos + EditState.PageX;
        PastResults [0] = 0;
      }
    DrawLine = true;
    if (LenSearch)
      while (Pos > 0)
        {
          // Find the first occurrence of the key going backwards
          Pos = StrPosFromBackwards (EditState.Buffer, Pos - 1, &EditState.Buffer [EditState.BufferPos + EditState.PageX], LenSearch, spmNoCase);
          // no matches?
          if (Pos == -1)
            break;
          // found a matching word
          if ((Pos == 0) || !IsIdentifier (EditState.Buffer [Pos - 1]))
            {
              // Find length of this identifier
              LenB = 0;
              while (IsIdentifier (EditState.Buffer [Pos + LenB]))
                LenB++;
              // Check for previous result
              a = -1;
              if (LenB + 3 < SIZEARRAY (s))
                {
                  cp = s;
                  CharToStr (&cp, ':');
                  StrToStrN (&cp, &EditState.Buffer [Pos], LenB);
                  CharToStr (&cp, ':');
                  *cp = 0;
                  a = StrPos_ (PastResults, s);
                }
              if (a < 0)   // this is a new result
                {
                  // Replace identifier-at-cursor with completed identifier
                  EditBufferDelete (EditState.BufferPos + EditState.PageX, LenA);
                  EditBufferInsert (EditState.BufferPos + EditState.PageX, &EditState.Buffer [Pos], LenB);
                  // Add completed identifier to PastResults
                  l = StrLength (PastResults);
                  if (l + LenB + 3 < SIZEARRAY (PastResults))
                    {
                      cp = &PastResults [l];
                      CharToStr (&cp, ':');
                      StrToStrN (&cp, &EditState.Buffer [Pos], LenB);
                      CharToStr (&cp, ':');
                      *cp = 0;
                    }
                  // Finish up
                  EditState.PageX += LenB;
                  return;
                }
            }
        }
    EditState.PageX += LenA;   // step past out typed word
    ConsoleBeep_ ();
  }

void EditDeleteLine (void)
  {
    unsigned int pos;
    //
    pos = EditBufferPosCalc (EditState.BufferPos, +1, NULL);   // Find start next line
    if (pos > EditState.BufferPos)   // If line exists
      {
        EditBufferDelete (EditState.BufferPos, pos - EditState.BufferPos);
        DrawPageFrom = EditState.PageY;
      }
  }

void EditDeleteEndLine (void)
  {
    unsigned int pos;
    //
    // Find end of line
    pos = EditState.BufferPos + EditState.PageX;
    while (!EditIsEOL (EditState.Buffer [pos]))
    //while ((pos < EditState.BufferSize) && (EditState.Buffer [pos] >= ' '))
      pos++;
    // Delete rest of line (if there is any)
    //if (pos > EditState.BufferPos + EditState.PageX)   // line extends past cursor
      {
        EditBufferDelete (EditState.BufferPos + EditState.PageX, pos - (EditState.BufferPos + EditState.PageX));
        DrawPageFrom = EditState.PageY;
      }
  }

void EditDeleteWord (void)
  {
    unsigned int l;
    //
    l = EditCurrentWord_ ();
    if (l)
      {
        EditBufferDelete (EditState.BufferPos + EditState.PageX, l);
        DrawPageFrom = EditState.PageY;
      }
    else
      ConsoleBeep_ ();
  }

bool InBlock (void)
  {
    if (EditState.BufferPos >= EditState.BlockA)
      if (EditState.BufferPos < EditState.BlockB)
        return true;
    return false;
  }

bool InBlock_ (void)
  {
    if (EditState.BufferPos >= EditState.BlockA)
      if (EditState.BufferPos <= EditState.BlockB)
        return true;
    return false;
  }

void EditFindNext (void)
  {
    unsigned int pos;
    //
    pos = StrPosFrom (EditState.Buffer, EditState.BufferPos + EditState.PageX + 1, EditTargets [0], Setup.FindModeContents);
    if (pos != -1)
      EditBufferPosMove (pos);
    else
      ConsoleBeep_ ();
  }

void EditFindPrev (void)
  {
    unsigned int pos;
    //
    pos = EditState.BufferPos + EditState.PageX - 1;
    if (pos != -1)
      pos = StrPosFromBackwards (EditState.Buffer, pos, EditTargets [0], 0, Setup.FindModeContents);
    if (pos != -1)
      EditBufferPosMove (pos);
    else
      ConsoleBeep_ ();
  }


#define EditTargetMax 128

void EditFind (void)
  {
    char *Default;
    int l;
    char c;
    //
    Default = malloc (EditTargetMax);
    l = EditCurrentWord ();
    if (l >= EditTargetMax)
      l = EditTargetMax - 1;
    if (l)
      MemMove (Default, &EditState.Buffer [EditState.BufferPos + EditState.PageX], l);
    Default [l] = 0;
    while (true)
      {
        EditConsolePrompt ();
        PutStringHighlight ("Find [|^F|indMode: ", Colours [ColQueryFG2]);
        PutString (StrPosModeNames [Setup.FindModeContents]);
        PutStringHighlight ("]: ", Colours [ColQueryFG2]);
        ConsoleColourFG (Colours [ColQueryFG2]);
        c = EditStringArray (EditTargets, SIZEARRAY (EditTargets), EditTargetMax, Default);
        if (c == Cntrl ('F'))
          Setup.FindModeContents = (Setup.FindModeContents + 1) % (spmZZZZ);
        else if ((c == KeyEnter) || (c == CmdFindNext))
          {
            DrawPageFrom = 0;
            EditFindNext ();
            break;
          }
        else if (c == CmdFindPrev)
          {
            DrawPageFrom = 0;
            EditFindPrev ();
            break;
          }
        else if (c == esc)
          break;
        else
          ConsoleBeep_ ();
      }
    free (Default);
    DrawHelp = true;
  }

/*
void EditBreak ()
  {
    char c;
    //
    EditConsolePrompt ();
    PutString ("What is expected at cursor: ");
    c = GetKeyWait (true);
    if (c < ' ' || c >= 0x80 || c != EditState.Buffer [EditState.BufferPos + EditState.PageX])
      ConsoleBeep_ ();   // Beep and Stop Macro
    DrawHelp = true;
  }
*/

void EditLineGoto (void)
  {
    longint line;
    //
    line = EditBufferGetCurrentLine () + 1;
    EditConsolePrompt ();
    PutString ("Go to Line ");
    ConsoleColourFG (Colours [ColQueryFG2]);
    if (EditLongint (&line) == KeyEnter)
      {
        EditBufferPosMove (0);
        EditPointerMove (line - 1);
      }
    DrawHelp = true;
  }

int BookmarksSwapCount = 0;

void EditSwapWithBookmarks (bool Repeat)
  {
    unsigned int Pos;
    //
    Pos = EditState.BufferPos + EditState.PageX;
    EditBufferPosMove (EditState.BufferBookmark);
    EditState.BufferBookmark = Pos;
  }

bool EditQuit (void)
  {
    char Filename [MaxPath];
    char c;
    _LineFormat f;
    //
    if (!EditState.Changed)
      return true;
    f = EditState.LineFormat;
    DrawHelp = true;
    while (true)
      {
        EditConsolePrompt ();
        StrCopyN (Filename, EditState.Filename, sizeof (Filename));
        PutStringHighlight ("Save File [|^F|ormat=", Colours [ColQueryFG2]);
        if (EditState.Hex)
          PutString ("HEX");
        else
          PutString (LineFormatName [f]);
        PutStringHighlight (" |Enter|, |Esc|, |^D|iscard]: ", Colours [ColQueryFG2]);
        c = EditString__ (Filename, sizeof (Filename));
        if (c == KeyEnter)
          {
            //StrAssign (&EditState.Filename, Filename);
            StrAssign (&DirItem, Filename);
            DrawTitle = true;
            //ChangeEOL (&EditState.Buffer, fLF, f, &EditState.BufferSize);
            EditState.LineFormat = f;
            return (EditSaveFile (Filename));
          }
        if (c == Cntrl ('D'))
          return true;
        if (c == esc)
          return false;
        if (c == Cntrl ('F') && !EditState.Hex)
          f = (f + 1) % fXXXX;
        else
          ConsoleBeep_ ();
      }
  }

void EditBlock (void)
  {
    bool MakeSingle;
    //
    MakeSingle = false;
    if (EditState.BlockA == EditState.BlockB)   // no block
      MakeSingle = true;
    else if (EditState.BlockB != EditBufferPosCalc (EditState.BlockA, +1, NULL))   // single line Block
      MakeSingle = true;
    if (MakeSingle)
      {
        EditState.BlockA = EditState.BufferPos;
        EditState.BlockB = EditBufferPosCalc (EditState.BlockA, +1, NULL);
      }
    else   // make larger block
      if (EditState.BufferPos < EditState.BlockA)
        EditState.BlockA = EditState.BufferPos;
      else
        EditState.BlockB = EditBufferPosCalc (EditState.BufferPos, +1, NULL);
    DrawPageFrom = 0;
  }

void EditBlockAll (void)
  {
    EditState.BlockA = 0;
    EditState.BlockB = EditState.BufferSize;
    DrawPageFrom = 0;
  }

void EditBlockGo (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        EditBufferPosMove (EditState.BlockA);
        //EditBufferPos = BlockA;
        //DrawPageFrom = 0;
      }
  }

int EditBlockSelectWord (void)
  {
    int Len;
    unsigned int Index, IndexRes;
    char c;
    int QVal;
    //
    Len = 0;
    Index = EditState.BufferPos + EditState.PageX;
    c = EditState.Buffer [Index];
    if (IsBracket (c))
      {
        IndexRes = StrFindMatchingBracket (Index, true);
        if (IndexRes != Index)
          if (IndexRes > Index)
            Len = IndexRes - Index + 1;
          if (IndexRes < Index)
            {
              EditState.PageX -= Index - IndexRes;
              Len = Index - IndexRes + 1;
            }
      }
    else if ((QVal = IsQuote (c)) > 0)
      {
        Len = 1;
        while (true)
          {
            c = EditState.Buffer [Index + Len];
            if (EditIsEOL (c))
              {
                Len = 0;
                break;
              }
            Len++;
            if (IsQuote (c) == QVal)
              break;
          }
      }
    else
      Len = EditCurrentWord ();
    return Len;
  }

bool EditBlockClipboardCopyCut (bool Cut)
  {
    unsigned int Selection;
    unsigned int Len;
    char *p;
    //
    EditorClipboardPartLine = false;
    p = Message;
    if (Cut)
      StrToStr (&p, "Cut");
    else
      StrToStr (&p, "Copied");
    StrToStr (&p, " to Clipboard: ");
    //if (EditState.BlockA == EditState.BlockB)   // no block  ? !InBlock()
    if (!InBlock_ ())
      {
        Len = EditBlockSelectWord ();   // is there a selectable word/bracketted/quote
        if (Len == 0)
          Len = EditLineLength (EditState.BufferPos + EditState.PageX);
        if (Len)
          {
            EditorClipboardPartLine = true;
            Selection = EditState.BufferPos + EditState.PageX;
            CharToStr (&p, '|');
            StrToStrN (&p, &EditState.Buffer [Selection], Len);
            CharToStr (&p, '|');
          }
      }
    else
      {
        Len = EditState.BlockB - EditState.BlockA;
        Selection = EditState.BlockA;
        StrToStr (&p, "Block");
      }
    if (Len == 0)   // Nothing clipped ...
      p = Message;   // ... so no Message
    *p = 0;   // Terminate Message
    if (Len)
      {
        if (EditorClipboard)
          free (EditorClipboard);
        EditorClipboardSize = Len;
        EditorClipboard = malloc (Len + 1);
        MemMove (EditorClipboard, &EditState.Buffer [Selection], Len);
        EditorClipboard [Len] = 0;   // Terminate: just for Compare command
        if (Cut)
          {
            EditBufferDelete (Selection, Len);
            EditState.BlockA = EditState.BlockB = 0;
            DrawTitle = true;
            DrawPageFrom = 0;
          }
        return true;
      }
    ConsoleBeep_ ();
    return false;
  }

/*
void EditBlockClipboardCut (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        // Copy to clipboard
        if (EditorClipboard)
          free (EditorClipboard);
        EditorClipboardSize = EditState.BlockB - EditState.BlockA;
        EditorClipboard = malloc (EditorClipboardSize);
        memcpy (EditorClipboard, &EditState.Buffer [EditState.BlockA], EditorClipboardSize);
        // Delete from EditBuffer
        EditState.BufferPos = EditState.BlockA;
        EditBufferDelete (EditState.BlockA, EditState.BlockB - EditState.BlockA);
        EditState.BlockA = EditState.BlockB = 0;
        DrawTitle = true;
        DrawPageFrom = 0;
        StrCopy (Message, "Block cut to Clipboard", SIZEARRAY (Message));
      }
  }
*/

void EditBlockClipboardPaste (void)
  {
    _LineFormat f;
    //
    if (EditorClipboard)
      {
        f = DetermineLineFormat (EditorClipboard, EditorClipboardSize);
        ChangeEOL (&EditorClipboard, f, fLF, &EditorClipboardSize);
        if (EditorClipboardPartLine)
          {
            EditBufferInsert (EditState.BufferPos + EditState.PageX, EditorClipboard, EditorClipboardSize);
            EditState.PageX += EditorClipboardSize;
          }
        else
          EditBufferInsert (EditState.BufferPos, EditorClipboard, EditorClipboardSize);
        DrawPageFrom = 0;
      }
    else
      ConsoleBeep_ ();
  }

void EditBlockClipboardCompare (void)
  {
    char *c;
    int CompareThreshold = 16;
    //
    if ((EditorClipboard == NULL) || (EditState.BlockA == EditState.BlockB))
      StrCopy (Message, "**** Fill the Clipboard AND Define a Block BEFORE Comparing them");
    else
      {
        // Specify the Sync Threshold
        EditConsolePrompt ();
        PutString ("Compare Clipboard with Block: Specify Sync Threshold: ");
        ConsoleColourFG (Colours [ColQueryFG2]);
        if (EditInt (&CompareThreshold, 4, 10000) == KeyEnter)
          {
            // save current clipboard
            c = EditorClipboard;
            // assign it to current block
            EditorClipboard = NULL;
            EditBlockClipboardCopyCut (false);
            // do the comparison
            CompareTextBlocks (c, EditorClipboard, CompareThreshold);
            // tidy up
            free (EditorClipboard);
            free (c);
            // update the clipboard
            EditorClipboard = CompareResult;   // Editor will free this
            EditorClipboardSize = CompareResultLength;
            CompareResult = NULL;
            // Help Message
            c = Message;
            StrToStr (&c, "Comparison result in Clipboard. Differences |");
            IntToStr (&c, CompareResultDifferences);
            *c = 0;
          }
      }
  }

/*
void EditBlockCopy (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        EditBufferInsert (EditState.BufferPos, &EditState.Buffer [EditState.BlockA], EditState.BlockB - EditState.BlockA);
        DrawPageFrom = 0;
      }
  }

void EditBlockDelete (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        EditState.BufferPos = EditState.BlockA;
        //PageY = 0;
        EditBufferDelete (EditState.BlockA, EditState.BlockB - EditState.BlockA);
        EditState.BlockA = EditState.BlockB = 0;
        DrawPageFrom = 0;
      }
  }

void EditBlockMove (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        EditBufferInsert (EditState.BufferPos, &EditState.Buffer [EditState.BlockA], EditState.BlockB - EditState.BlockA);
        EditBufferDelete (EditState.BlockA, EditState.BlockB - EditState.BlockA);
        EditState.BlockA = EditState.BlockB = 0;
        DrawPageFrom = 0;
      }
  }
*/

void LineIndent (unsigned int Pos)
  {
    if (EditState.Buffer [Pos] == '\t')
      EditBufferInsert (Pos, (char *) &Tab_, 1);
    else
      EditBufferInsert (Pos, (char *) &Space_, 1);
    DrawLine = true;
  }

void LineOutdent (unsigned int Pos)
  {
    bool go;
    //
    go = true;
    if (EditIsEOL (EditState.Buffer [Pos]))
      go = false;
    if (!EditSetup.OutdentErases)
      if ((EditState.Buffer [Pos] != ' ') && (EditState.Buffer [Pos] != '\t'))
        go = false;
    if (go)
      {
        EditBufferDelete (Pos, 1);
        DrawLine = true;
      }
  }

void EditBlockIndent (void)
  {
    unsigned int pos;
    //
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        pos = EditState.BlockA;
        while (pos < EditState.BlockB)
          {
            LineIndent (pos);
            //EditBufferInsert (pos, (char *) &Space_, 1);
            pos = EditBufferPosCalc (pos, +1, NULL);
          }
        DrawPageFrom = 0;
      }
  }

void EditBlockOutdent (void)
  {
    unsigned int pos;
    //
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        pos = EditState.BlockA;
        while (pos < EditState.BlockB)
          {
            LineOutdent (pos);
            //if (EditState.Buffer [pos] == ' ')
            //  EditBufferDelete (pos, 1);
            pos = EditBufferPosCalc (pos, +1, NULL);
          }
        DrawPageFrom = 0;
      }
  }

void EditBlockTabs (void)
  {
    unsigned int pos;
    unsigned int tabs;
    int x, n, delta;
    char *p;
    //
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        // Count existing tabs in block
        tabs = 0;
        pos = EditState.BlockA;
        while (pos < EditState.BlockB)
          {
            if (EditState.Buffer [pos] == tab)
              tabs++;
            pos++;
          }
        p = Message;
        StrToStr (&p, "Block: ");
        if (tabs)
          StrToStr (&p, "tabs -> spaces");
        else
          StrToStr (&p, "spaces -> tabs");
        *p = 0;
        pos = EditState.BlockA;
        x = 0;
        n = 0;
        while (pos < EditState.BlockB)
          {
            if (EditIsEOL (EditState.Buffer [pos]))
              {
                x = 0;
                n = 0;
              }
            else
              if (tabs)   // expand tabs
                {
                  if (EditState.Buffer [pos] == tab)
                    {
                      delta = EditSetup.TabSize - (x % EditSetup.TabSize) - 1;
                      EditState.Buffer [pos] = ' ';
                      EditBufferInsert (pos, NULL, delta);
                    }
                  x++;
                }
              else   // contract tabs
                {
                  if (EditState.Buffer [pos] == ' ')
                    {
                      n++;
                      if ((x % EditSetup.TabSize) == (EditSetup.TabSize - 1))   // tab stop
                        {
                          EditState.Buffer [pos] = '\t';
                          EditBufferDelete (pos - (n - 1), n - 1);
                          pos = pos - (n - 1);
                          n = 0;
                        }
                    }
                  else
                    n = 0;
                  x++;
                }
            pos++;
          }
        DrawPageFrom = 0;
      }
  }

void EditBlockRead (void)
  {
    int file;
    unsigned int BlockSize;
    char *Block;
    bool OK;
    _LineFormat f;
    //
    EditConsolePrompt ();
    PutString ("Block Read from file: ");
    if (EditString__ (EditBlockFilename, sizeof (EditBlockFilename)) == KeyEnter)
      {
        OK = false;
        file = FileOpen (EditBlockFilename, foRead);
        if (file >= 0)
          {
            BlockSize = lseek (file, 0, SEEK_END);
            lseek (file, 0, SEEK_SET);
            Block = (char *) malloc (BlockSize);
            if (read (file, Block, BlockSize) == BlockSize)
              {
                f = DetermineLineFormat (Block, BlockSize);
                ChangeEOL (&Block, f, fLF, &BlockSize);
                EditBufferInsert (EditState.BufferPos, Block, BlockSize);
                EditState.BlockA = EditState.BufferPos;
                EditState.BlockB = EditState.BlockA + BlockSize;
                OK = true;
              }
            else
              StrCopy (Message, (char *) MessageErrorRead);
            close (file);
            DrawPageFrom = 0;
            free (Block);
          }
        if (!OK)
          StrCopy (Message, (char *) MessageErrorRead);
      }
    DrawHelp = true;
  }

void EditBlockWrite (void)
  {
    if (EditState.BlockA == EditState.BlockB)
      ConsoleBeep_ ();
    else
      {
        EditConsolePrompt ();
        PutString ("Block Write to file: ");
        if (EditString__ (EditBlockFilename, sizeof (EditBlockFilename)) == KeyEnter)
          EditWriteToFile (EditBlockFilename, &EditState.Buffer [EditState.BlockA], EditState.BlockB - EditState.BlockA);
      }
    DrawHelp = true;
  }

unsigned int BlockStepSpace (unsigned int Pos, bool *Break)
  {
    *Break = false;
    while ((Pos < EditState.BlockB) && (EditState.Buffer [Pos] <= ' '))
      if (EditIsEOL (EditState.Buffer [Pos]))
        {
          Pos ++; //= EditEOLSize ();
          if (EditState.Buffer [Pos] <= ' ')
            *Break = true;
        }
      else
        Pos++;
    return Pos;
  }

unsigned int BlockStepWord (unsigned int Pos)
  {
    while ((Pos < EditState.BlockB) && (EditState.Buffer [Pos] > ' '))
      Pos++;
    return Pos;
  }

void EditBlockFormat (void)
  {
    unsigned int bs;
    unsigned int PosA, PosB, PosC;
    int x;
    char *Res, *Res_;
    unsigned int ResSize;
    bool Break;
    int Breaks;
    //
    bs = EditState.BlockB - EditState.BlockA;
    if (bs == 0)
      ConsoleBeep_ ();
    else
      {
        Res = NULL;
        while (true)
          {
            ResSize = 0;
            Res_ = Res;
            PosA = EditState.BlockA;
            x = 0;
            while (PosA < EditState.BlockB)
              {
                Breaks = 0;
                PosB = BlockStepSpace (PosA, &Break);
                if (Break)
                  Breaks = 2;
                PosC = BlockStepWord (PosB);
                if (Breaks == 0)
                  if (x)
                    if (x + 1 + (PosC - PosB) >= ConsoleSizeX)   // goes past line end
                      Breaks = 1;
                while (Breaks--)
                  {
                    ResSize += CharToStr (&Res_, lf);
                    x = 0;
                  }
                if (x)
                  {
                    ResSize += CharToStr (&Res_, ' ');
                    x++;
                  }
                if (PosC > PosB)
                  {
                    ResSize += StrToStrN (&Res_, &EditState.Buffer [PosB], PosC - PosB);
                    x += PosC - PosB;
                  }
                PosA = PosC;
              }
            ResSize += CharToStr (&Res_, lf);
            if (Res == NULL)
              Res = malloc (ResSize);
            else
              break;
          }
        EditBufferDelete (EditState.BlockA, bs);
        EditBufferInsert (EditState.BlockA, Res, ResSize);
        EditState.BlockB = EditState.BlockA + ResSize;
        free (Res);
        EditState.BufferPos = EditState.BlockA;
        DrawPageFrom = 0;
      }
  }

void EditBlockHexBinary (void)
  {
    unsigned int bsz;
    char *Text, *x;
    unsigned int TextSize;
    //
    Text = NULL;
    bsz = EditState.BlockB - EditState.BlockA;
    if (bsz == 0)
      ConsoleBeep_ ();
    else
      {
        // Interpret HEX/ASCII into raw Data
        x = &EditState.Buffer [EditState.BlockA];
        TextSize = StrGetHexAscii_ (&x, NULL, 0, bsz);
        if (x != &EditState.Buffer [EditState.BlockB])   // Junk after last hex
          TextSize = -1;
        if (TextSize == -1)   // Error, therefore already binary
          Text = DataToHex (&EditState.Buffer [EditState.BlockA], bsz, &TextSize, ConsoleSizeX - 1);
        else   // not Error
          {
            Text = (byte *) malloc (TextSize);
            x = &EditState.Buffer [EditState.BlockA];
            StrGetHexAscii_ (&x, Text, TextSize, bsz);
          }
        EditState.BufferPos = EditState.BlockA;
        EditBufferDelete (EditState.BlockA, bsz);
        EditBufferInsert (EditState.BlockA, Text, TextSize);
        EditState.BlockB = EditState.BlockA + TextSize;
        free (Text);
        DrawPageFrom = 0;
      }
  }

void EditBlockHexAscii (void)
  {
    unsigned int bsz;
    byte *Data;
    unsigned int DataSize;
    char *Text, *x;
    unsigned int TextSize;
    //
    bsz = EditState.BlockB - EditState.BlockA;
    if (bsz == 0)
      ConsoleBeep_ ();
    else
      {
        // Interpret HEX/ASCII into raw Data
        x = &EditState.Buffer [EditState.BlockA];
        DataSize = StrGetHexAscii_ (&x, NULL, 0, bsz);
        if (x != &EditState.Buffer [EditState.BlockB])   // Junk after last hex
          DataSize = -1;
        if (DataSize == -1)   // Error
          {
            StrCopy (Message, (char *) MessageErrorHex);
            EditBufferPosMove (EditBufferPosCalc (x - EditState.Buffer + 1, -1, NULL));
            EditState.PageX = x - EditState.Buffer - EditState.BufferPos;
          }
        else   // not Error
          {
            Data = (byte *) malloc (DataSize);
            x = &EditState.Buffer [EditState.BlockA];
            StrGetHexAscii_ (&x, Data, DataSize, bsz);
            // Now reformat differently
            if (StrChCount (&EditState.Buffer [EditState.BlockA], '\"', bsz) > 0)
              Text = DataToHex (Data, DataSize, &TextSize, ConsoleSizeX - 1);
            else
              Text = DataToHexAscii (Data, DataSize, &TextSize, ConsoleSizeX - 1);
            EditState.BufferPos = EditState.BlockA;
            EditBufferDelete (EditState.BlockA, bsz);
            EditBufferInsert (EditState.BlockA, Text, TextSize);
            EditState.BlockB = EditState.BlockA + TextSize;
            free (Data);
            free (Text);
            DrawPageFrom = 0;
          }
      }
  }

const char *EditSetupFieldsNames [] = {"Type Mode", "Tab Size", "Auto Indent", "Outdent Destructive", "Syntax", NULL};
//const char *SyntaxNames [] = {"None", "C/C++", "Pascal", "HTML", NULL};

_EditSetup EditSetup_;

char **SyntaxList;

void BuildSyntaxList (void)
  {
    // TODO
  }

int EditSetupFieldsEdit (int Field)
  {
    switch (Field)
      {
        case 0: // Overwrite
          return EditEnum ((_enum *) &EditSetup_.Overwrite, OverwriteNames);
          //return EditBool (&EditSetup_.Overwrite);
        case 1: // Define TAB size
          return EditInt (&EditSetup_.TabSize, 2, 32);
        case 2: // Toggle Auto Indent
          return EditBool (&EditSetup_.AutoIndent);
        case 3: // Toggle Outdent Clears
          return EditBool (&EditSetup_.OutdentErases);
        default: // Syntax  TODO:Cycle list
          return EditString__ (EditSetup_.Syntax, sizeof (EditSetup.Syntax));
        //default: // Cycle Find mode
        //  return EditEnum ((_enum *) &EditSetup_.FindMode, FindModeNames);
      }
  }

void EditSettings (void)
  {
    ConsoleLine(ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Editor Settings ");
    EditSetup_ = EditSetup;
    if (EditFields (EditSetupFieldsNames, EditSetupFieldsEdit, Colours [ColQueryFG2]))
      {
        EditSetup = EditSetup_;
        SyntaxLoad (EditSetup.Syntax);
        DrawTitle = true;
        DrawPageFrom = 0;
      }
    DrawHelp = true;
  }

int MacroFind (int Item)   // Find Index for Item
  {
    static int Item_ = 0;
    static int Index = 0;
    //
    if (Item < Item_)
      {
        Item_ = 0;
        Index = 0;
      }
    while (true)
      {
        if (Index >= SIZEARRAY (Macros))
          return -Item_;
        if (Macros [Index])
          {
            if (Item == Item_)   // found
              return Index;
            Item_++;
          }
        Index++;
      }
  }

void ShowPageItemMacro (void *Data, int Item, int xOffset)
  {
    int Index;
    char *cp;
    //
    Index = MacroFind (Item);   // Find Index for Item
    if (Index > 0)
      if (Macros [Index])
        {
          // Display Macro [Index]
          PutChar_ (Index);
          ConsoleCursor (8, ConsoleY);
          ConsoleColourFG (Colours [ColBodyFG]);
          cp = Macros [Index];
          while (*cp)
            PutChar_ (*cp++);
        }
  }

void ShowMacros (void)
  {
    int siz, i;
    int y;
    //
    MacroFind (0);   // Reset internal position
    siz = -MacroFind (SIZEARRAY (Macros));
    ConsoleLine (0, Colours [ColTitleFG1], Colours [ColTitleBG]);
    PutString ("EDITOR MACROS:");
    ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
    PutStringHighlight ("|Up|/|Dn|..  |Del|  |Esc|", Colours [ColHelpFG2]);
    y = 0;
    while (true)
      {
        i = ShowGenericPage (1, 1, ShowPageItemMacro, NULL, siz, Colours [ColBodyFGDir], Colours [ColBodyBG], Colours [ColBodyBGSel], &y, NULL);
        if (i == esc)
          break;
        if (i == KeyDel)
          {
            i = MacroFind (y);
            if (i >= 0)
              if (Macros [i])
                {
                  free (Macros [i]);
                  Macros [i] = NULL;
                  siz--;
                }
          }
        else
          ConsoleBeep_ ();
      }
    DrawPageFrom = 0;
    DrawTitle = true;
    DrawHelp = true;
  }

void EditDragBlock (int Dirn)
  {
    unsigned int pos;
    //
    pos = EditState.BufferPos;
    EditPointerMove (Dirn);
    if (pos == EditState.BlockA && EditState.BlockA != EditState.BlockB)
      EditState.BlockA = EditState.BufferPos;
    else if (pos == EditState.BlockB && EditState.BlockA != EditState.BlockB)
      EditState.BlockB = EditState.BufferPos;
    else
      {
        EditState.BlockA = Min (pos, EditState.BufferPos);
        EditState.BlockB = Max (pos, EditState.BufferPos);
      }
    DrawTitle = true;
    DrawPageFrom = 0;
  }

bool EditProcessCntrlShiftArrow (int c)
  {
    bool Used;
    //
    Used = true;
    if ((c == KeyShiftUp) || (c == KeyAltUp))
      EditDragBlock (-1);
    else if ((c == KeyShiftDown) || (c == KeyAltDown))
      EditDragBlock (+1);
    else if ((c == KeyShiftPageDown) || (c == KeyAltPageDown))
      EditDragBlock (ConsoleSizeY - 2);
    else if ((c == KeyShiftPageUp) || (c == KeyAltPageUp))
      EditDragBlock (2 - ConsoleSizeY);
    else if ((c == KeyShiftHome) || (c == KeyAltHome))
      EditDragBlock (-99999999);
    else if ((c == KeyShiftEnd) || (c == KeyAltEnd))
      EditDragBlock (+99999999);
    else if ((c == KeyShiftLeft) || (c == KeyAltLeft) || (c == Cntrl (']')))
      if (InBlock ())
        EditBlockOutdent ();
      else
        LineOutdent (EditState.BufferPos);
    else if ((c == KeyShiftRight) || (c == KeyAltRight) || (c == Cntrl ('\\')))
      if (InBlock ())
        EditBlockIndent ();
      else
        LineIndent (EditState.BufferPos);
    else if ((c == KeyCntrlLeft) || (c == Cntrl ('T')))   // Word Left
      EditWordPrev ();
    else if ((c == KeyCntrlRight) || (c == Cntrl ('Y')))   // Word Right
      EditWordNext ();
    else
      Used = false;
    return Used;
  }

void EditProcessCommandBlock (void)
  {
    int c;
    //
    ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
    //PutStringHighlight ("Block: |^B|lock |^G|o |^C|opy M|^o|ve |^D|elete |^A|ll |^T|abs |^R|ead |^W|rite   |Esc|", ColourHelpFG1, ColourHelpFG2);
    PutStringHighlight ("Block: |^B|lock |^G|o |^A|ll |^T|abs |^H|ex b|^I|nary |^R|ead |^W|rite |^F|ormat |^C|ompare  |Esc|", Colours [ColHelpFG2]);
    c = GetKeyWait (false);
    ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
    if (c ==  Cntrl ('B'))   // Block Define
      EditBlock ();
    else if (c == Cntrl ('A'))   // Block All
      EditBlockAll ();
    else if (c == Cntrl ('G'))   // Block Go
      EditBlockGo ();
//    else if (c == CmdCopy)   // Block Copy
//      EditBlockCopy ();
//    else if ((c == CmdMove) || (c == Cntrl ('M')))   // Block Move
//      EditBlockMove ();
//    else if (c == CmdDelete)   // Block Delete
//      EditBlockDelete ();
    else if (c == Cntrl ('T'))   // Block Tabs expand / contract
      EditBlockTabs ();
    else if (c == Cntrl ('R'))   // Block Read
      EditBlockRead ();
    else if (c == Cntrl ('W'))   // Block Write
      EditBlockWrite ();
    else if (c == Cntrl ('F'))   // Block Format
      EditBlockFormat ();
    else if (c == Cntrl ('H'))   // Block Hex
      EditBlockHexAscii ();
    else if (c == Cntrl ('I'))   // Block bInary
      EditBlockHexBinary ();
    else if (c == Cntrl ('C'))   // Block Compare to Clipboard
      EditBlockClipboardCompare ();
    else if (c == esc)
      ;
    else
      ConsoleBeep_ ();
    DrawHelp = true;
    DrawTitle = true;
  }

void EditProcessCommand (int c, int cPrev)
  {
    unsigned int pos1, pos2;
    int Lines;
    //
    Lines = ConsoleSizeY - 2;
    if (!EditInsertChar (c))
      if (c == KeyBackSpace)
        {
          if (EditState.BufferPos + EditState.PageX > 0)
            if (EditState.PageX == 0)   // Merge lines
              {
                pos1 = EditBufferPosCalc (EditState.BufferPos, -1, NULL);
                EditState.PageX = EditLineLength (pos1);
                if (!EditSetup.Overwrite)
                  EditBufferDelete (pos1 + EditState.PageX, EditState.BufferPos - pos1 - EditState.PageX);
                EditState.BufferPos = pos1;
                EditState.PageY--;
                DrawPageFrom = Max (0, EditState.PageY);
              }
            else
              {
                EditState.PageX--;
                if (EditSetup.Overwrite)
                  EditBufferOverwrite (EditState.BufferPos + EditState.PageX, (char *) &Space_, 1);
                else
                  EditBufferDelete (EditState.BufferPos + EditState.PageX, 1);
                DrawLine = true;
              }
        }
      else if (c == KeyDel)
        {
          c = EditCurrentChar (0);
          if (c)
            {
              EditBufferDelete (EditState.BufferPos + EditState.PageX, 1);
              if (EditIsEOL (c))
                DrawPageFrom = EditState.PageY;
              DrawLine = true;
            }
          else
            ConsoleBeep_ ();
        }
      else if (c == KeyIns)
        {
          EditBufferInsert (EditState.BufferPos + EditState.PageX, (char *) &Space_, 1);
          DrawLine = true;
        }
      else if (c == KeyUp)
        EditPointerMove (-1);
      else if (c == KeyDown)
        EditPointerMove (+1);
      else if (c == KeyLeft)
        if (EditState.PageX == 0)
          {
            EditPointerMove (-1);
            EditState.PageX = EditLineLength (EditState.BufferPos);
          }
        else
          EditState.PageX--;
      else if (c == KeyRight)
        if (EditState.PageX >= EditLineLength (EditState.BufferPos))
          {
            EditPointerMove (+1);
            EditState.PageX = 0;
          }
        else
          EditState.PageX++;
      else if (c == KeyHome)
        if (cPrev == KeyHome)
          EditBufferPosMove (0);
        else
          EditState.PageX = 0;
      else if (c == KeyEnd)
        if (cPrev == KeyEnd)
          EditBufferPosMove (EditBufferPosCalc (EditState.BufferSize, -1, NULL));
        else
          EditState.PageX = EditLineLength (EditState.BufferPos);
      else if (c == KeyPageUp)
        if (EditState.PageY > 0)
          EditPointerMove (-EditState.PageY);
        else
          {
            EditPointerMove (- Lines + 1);
            EditState.PageY = 0;
            DrawPageFrom = 0;
          }
      else if (c == KeyPageDown)
        if (EditState.PageY < Lines - 1)
          EditPointerMove (Lines - 1 - EditState.PageY);
        else
          {
            EditPointerMove (Lines - 1);
            DrawPageFrom = 0;
          }
      else if (c == Cntrl ('G'))   // Go to line
        EditLineGoto ();
      else if (c == Cntrl ('E'))   // Exchange with Bookmarks
        EditSwapWithBookmarks (cPrev == Cntrl ('E'));
      else if (c == Cntrl ('F'))   // Find
        EditFind ();
      else if (c == CmdFindNext)   // Find Next
        EditFindNext ();
      else if (c == CmdFindPrev)   // Find Prev
        EditFindPrev ();
      else if (c == Cntrl ('W'))   // Find Matching Parenthases
        EditFindMatchingBracket ();
      //else if (c == Cntrl ('^'))   // Break if not ...
      //  EditBreak ();
      else if (c == Cntrl ('U'))   // Change Editor Settings
        EditSettings ();
      //
      else if (c == Cntrl ('K'))   // Change Case
        EditChangeCase ();
      else if (c == Cntrl ('A'))   // Auto complete
        EditAutoComplete (cPrev == Cntrl ('A'));
      //
      else if (c == Cntrl ('X'))   // Block Cut to Clipboard
        EditBlockClipboardCopyCut (true);
      else if (c == Cntrl ('C'))   // Block Copy to Clipboard
        EditBlockClipboardCopyCut (false);
      else if (c == Cntrl ('V'))   // Block Paste from Clipboard
        EditBlockClipboardPaste ();
      //
      else if (c == Cntrl ('Z'))   // Undo
        EditUndo ();
      //
      else if (c == KeyCntrlDel)   // Delete word
        EditDeleteWord ();
      else if (c == KeyAltDel)   // Delete End of Line
        EditDeleteEndLine ();
      else if (c == Cntrl ('L'))   // Delete End of Line
        EditDeleteEndLine ();
      else if (c == KeyShiftDel)
        EditDeleteLine ();
      else if (c == KeyCntrlUp)   // Move line up
        {
          if (EditState.BufferPos > 0)
            {
              pos1 = EditState.BufferPos;
              pos2 = EditBufferPosCalc (EditState.BufferPos, +1, NULL);
              EditPointerMove (-1);
              EditBufferInsert (EditState.BufferPos, &EditState.Buffer [pos1], pos2 - pos1);
              EditBufferDelete (pos1 + (pos2 - pos1), pos2 - pos1);
              DrawPageFrom = EditState.PageY;
            }
        }
      else if (c == KeyCntrlDown)   // Move line down
        {
          if (EditState.BufferPos < EditState.BufferSize)
            {
              pos1 = EditBufferPosCalc (EditState.BufferPos, +1, NULL);
              pos2 = EditBufferPosCalc (pos1, +1, NULL);
              EditBufferInsert (pos2, &EditState.Buffer [EditState.BufferPos], pos1 - EditState.BufferPos);
              EditBufferDelete (EditState.BufferPos, pos1 - EditState.BufferPos);
              EditPointerMove (+1);
              DrawPageFrom = EditState.PageY - 1;
            }
        }
      else if (c == Cntrl ('D'))   // Duplicate line
        {
          pos1 = EditBufferPosCalc (EditState.BufferPos, +1, NULL);
          EditBufferInsert (pos1, &EditState.Buffer [EditState.BufferPos], pos1 - EditState.BufferPos);
          DrawPageFrom = EditState.PageY;
        }
      else if (c == Cntrl ('O'))   // Toggle Overwrite
        {
          EditSetup.Overwrite = !EditSetup.Overwrite;
          DrawTitle = true;
        }
      //
      else if (c == Cntrl ('B'))   // Block Commands
        EditProcessCommandBlock ();
      else
        if (!EditProcessCntrlShiftArrow (c))
          if (c == Cntrl ('_'))   // ^_ Help
            {
              ShowHelpPageFile_ ("fbcedit.hlp");
              //ShowHelpPage (EditHelpPage, SIZEARRAY (EditHelpPage));
              DrawPageFrom = 0;
              DrawTitle = true;
              DrawHelp = true;
            }
          else
            if (c)
              ConsoleBeep ();
  }

void MacroRecordStart (word c)
  {
    char *x;
    //
    MacroRecord = c;
    GetKeyMacroRecordSize = MacroSize;
    GetKeyMacroRecord = Macro;
    x = Message;
    StrToStr (&x, "Recording Macro on |");
    CharToStr_ (&x, MacroRecord);
    CharToStr (&x, '|');
    *x = 0;
  }

void UpdateExtensionAlias (char *Ext)
  {
    char *s, *ps;
    if (Ext [0])
      if (!StrSame_ (EditSetup.Syntax, Ext + 1))
        {
          s = malloc (MaxPath);
          ps = s;
          StrToStr (&ps, Ext + 1);
          if (EditSetup.Syntax [0])
            {
              CharToStr (&ps, tab);
              StrToStr (&ps, EditSetup.Syntax);
              *ps = 0;
              StringArraySearchReplace (ExtensionAlias, SIZEARRAY (ExtensionAlias), s);
            }
          else
            {
              *ps = 0;
              StringArraySearchRemove (ExtensionAlias, SIZEARRAY (ExtensionAlias), s);
            }
          free (s);
        }
  }

bool EditFile (char *Filename, char *Target, bool Hex)
  {
    int c, cPrev;
    char *Ext;
    //
    EditState.Filename = Filename;
    FileWritten = false;
    EditSetup.Syntax [0] = 0;
    Ext = StrGetFileExtension (Filename);
    if (Ext [0])
      {
        StrCopyN (EditSetup.Syntax, &Ext [1], sizeof (EditSetup.Syntax));
        SyntaxLoad (EditSetup.Syntax);
      }
    //EditSetup.Overwrite = false;
    //EditSetup.OutdentErases = false;
    //EditSetup.FindMode = spmNoCase;
    EditState.BlockA = EditState.BlockB = 0;
    if (EditLoadFile (Hex))
      {
        EditState.BufferPos = 0;
        EditState.BufferBookmark = 0;
        EditState.PageX = 0;
        EditState.PageY = 0;
        EditState.PageX0 = 0;
        // Restore
        if (!Target || !Target [0])   // No target
          {
            c = EditRestoreFind (Filename);
            if (c >= 0)
              {
                EditBufferPosMove (EditRestore [c].Pos);
                EditState.BufferBookmark = EditRestore [0].Bookmark;
              }
          }
        else
          {
            StringArrayAdd (EditTargets, SIZEARRAY (EditTargets), Target);
            EditFindNext ();
          }
        DrawPageFrom = 0;
        DrawHelp = true;
        DrawTitle = true;
        c = 0;
        while (true)
          {
            if (EditState.PageY < 0)
              {
                EditState.PageY = 0;
                DrawPageFrom = 0;
              }
            if (EditState.PageY >= ConsoleSizeY - 2)
              {
                EditState.PageY = ConsoleSizeY - 3;
                DrawPageFrom = 0;
              }
            while (CalculateConsoleX (&EditState.Buffer [EditState.BufferPos], EditState.PageX) - EditState.PageX0 >= ConsoleSizeX)
              {
                EditState.PageX0 += 8;
                DrawPageFrom = 0;
              }
            while (CalculateConsoleX (&EditState.Buffer [EditState.BufferPos], EditState.PageX) - EditState.PageX0 < 0)
              {
                EditState.PageX0 -= 8;
                DrawPageFrom = 0;
              }
            if (DrawTitle)
              EditDrawTitle ();
            DrawTitle = false;
            if (DrawPageFrom >= 0 && !GetKeyBuffered ())
              EditDrawPage ();
            if (DrawLine)
              EditDrawLine ();
            if (Message [0])
              {
                ConsoleLine (ConsoleSizeY - 1, Colours [ColHelpFG1], Colours [ColHelpBG]);
                PutStringHighlight (Message, Colours [ColHelpFG2]);
                if (Message [0] == '*')
                  ConsoleBeep_ ();
                Message [0] = 0;
                DrawHelp = true;
              }
            else if (DrawHelp)
              EditDrawHelp ();
            if (EditState.PageX > EditLineLength (EditState.BufferPos))
              EditState.PageX = EditLineLength (EditState.BufferPos);
            //
            ConsoleCursor (CalculateConsoleX (&EditState.Buffer [EditState.BufferPos], EditState.PageX) - EditState.PageX0, EditState.PageY + 1);
            cPrev = c;
            c = 0;
            #ifdef MacrosEnabled
            if ((GetKeyMacro) && MacroBlock && !InBlock ())   // Playing Macro AND Move out of Block?
              GetKeyMacro = NULL;   // Stop Macro
            if (GetKeyMacro && (*GetKeyMacro == 0))   // Just finished playing a macro
              if (MacroBlock && InBlock ()) // Started Macro in Block AND Still in Block
                GetKeyMacro = Macros [MacroPlay];   // Restart Macro
            c = GetKey_ ();
            if (c >= 0)   // Real Key Pressed
              GetKeyMacro = 0;   // then cancel playing macro
            else
              c = GetKeyWait (MacroRecord >= 0);
            if (c == GetKeyWaitResizeOccured)
              {
                DrawTitle = true;
                DrawHelp = true;
                DrawPageFrom = 0;
              }
            else if (MacroRecord < 0)   // New Macro, get assigned Key
              {
                if (c == Cntrl ('R'))   // OR Show all Macros
                  {
                    ShowMacros ();
                    MacroRecord = 0;
                  }
                else if (c == esc)   // no Macro on Esc
                  MacroRecord = 0;
                else
                  MacroRecordStart (c);
                DrawTitle = true;
                c = 0;
              }
            else if (c == Cntrl ('R'))   // Start / Stop Recoding Macro
              {
                if (GetKeyMacroRecord)   // Currently recording a Macro?
                  {
                    *(--GetKeyMacroRecord) = 0;   // Remove the unwanted ^R character
                    if (Macro [0])
                      {
                        StrAssign ((char **) &Macros [MacroRecord], Macro);
                        StrCopy (Message, "Macro Complete");
                      }
                    else
                      {
                        StrAssign ((char **) &Macros [MacroRecord], NULL);
                        StrCopy (Message, "Macro Deleted");
                      }
                    GetKeyMacroRecord = NULL;   // Stop Recording
                    //MacrosChanged = true;
                    DrawTitle = true;
                  }
                else   // Not recording a Macro so start
                  {
                    MacroRecord = -1;
                    StrCopy (Message, "Record Macro: Select Macro Key/Alt-Key/^Key   OR |^R| to view Macros");
                  }
                c = 0;
              }
            else if ( (Macros [c]) &&   // A Macro for c exists
                      (GetKeyMacro == NULL) &&   // We are not Playing a Macro
                      (GetKeyMacroRecord == NULL) )   // We are not Recording a Macro
              { // Start Playing a Macro
                MacroPlay = c;
                GetKeyMacro = Macros [MacroPlay];
                MacroBlock = InBlock ();
                c = 0;
              }
            else
            #else
            c = GetKeyWait (true);
            #endif
            if (c >= 0)
              if (c == esc)
                {
                  if (EditQuit ())
                    break;
                }
              else if (c == Cntrl ('S'))
                {
                  if (!EditSaveFile (EditState.Filename))
                    StrCopy (Message, (char *) MessageErrorWrite);
                }
              else
                EditProcessCommand (c, cPrev);
          }
      }
    EditRestoreSet (Filename, EditState.BufferPos + EditState.PageX);
    if (EditState.Buffer)
      free (EditState.Buffer);
    UndoFree ();
    EditState.Buffer = NULL;
    HighlightItemsFree ();
    #ifdef MacrosEnabled
    GetKeyMacro = NULL;
    GetKeyMacroRecord = NULL;
    #endif
    UpdateExtensionAlias (Ext);
    return FileWritten;
  }

void EditFree (void)
  {
    int i;
    //
    if (EditorClipboard)
      free (EditorClipboard);
    EditorClipboard = NULL;
    EditorClipboardSize = 0;
    EditRestoreFree ();
    for (i = 0; i < MacroNum; i++)
      StrAssign ((char **) &Macros [i], NULL);
    StringArrayFree (EditTargets, SIZEARRAY (EditTargets));
  }


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
const byte ElementColour1 [seZZZZ] =   // Black theme
  {
    ColGreen,
    ColCyan,
    ColPurple,
    ColRed,
    ColYellow | ColItalic,
    ColBlue | ColItalic,  //ColBrown,
  };

const byte ElementColour2 [seZZZZ] =   // White theme
  {
    ColGreenDark,
    ColCyanDark,
    ColPurple,
    ColMaroon,
    ColBlueDark | ColItalic,
    ColBlue | ColItalic,  //ColBrown,
  };

const byte ElementColour3 [seZZZZ] =   // Blue theme
  {
    ColGreen,
    ColCyan,
    ColPurple,
    ColWhite,
    ColYellow | ColItalic, //ColBlue,
    ColGray | ColItalic,
  };

const byte *ElementColour [] = {ElementColour1, ElementColour2, ElementColour3};
*/

