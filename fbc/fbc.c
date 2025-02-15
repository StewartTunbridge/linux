////////////////////////////////////////////////////////////////////////////
//
// FILE BROWSER CONSOLE
// ====================
//
// "Visual" console app to perform filing and editing.
// Visual file list, smart copy, smart move, properties, smart rename, delete
// Visual Text editor: Macros, syntax highlighting, auto complete
//
// 29 Aug 2013 Genesis Filing tool for Linux Console. (Power to the people)
// 08 Jan 2014 Complete + Bug fixes
// 18 Mar 2015 Add Text Edit
// 03 Apr 2015 Editor Extensions (inc macros)
// 05 Apr 2015 Fix Move bug (and small memory leak)
// 06 Nov 2015 Get working on Ubuntu on PC + tiny changes
// 21 Mar 2016 Cope with (and display) Symbolic Links
// 26 Mar 2016 Other fixes and faster sort
// 06 Apr 2016 Tidy. Small bugs/changes
// 07 Apr 2016 Keep list of Edit.Targets. Save lists to ~/.fbc
// 08          Edit: Add ^K change case word. Edit.Find: Default word at cursor
// 16          Edit: Reorganize Block Commands
// 17          Edit: Syntax Highlighting
// 22          Add Separate ^Edit command
// 02 May 2016 Add Colour Themes (and highlight executables)
// 03 May 2016 Edit: Support TAB normally
// 11 May 2016 Edit: Allow some changeable settings (^S)
// 20 May 2016 Display Tagged as background colour
//             Syntax Highlight for C/C++ and HTML
// 21 Jun 2016 Support for larger files (just a #define, was already there)
// 08 Jul 2016 Rename: Allow auto numbering
// 29 Jul 2016 Permissions: Add Owner & Group
// 01 Aug 2016 Macros: Define/Redefine all keys. Save / Load.
// 24 Aug 2016 Multiple Bookmarks
// 02 Nov 2016 Edit: Autocomplete
// 11 Jan 2017 Search: Show Results: Trim Directory
// 24 Mar 2017 small display / keycode fixes
// 25 Jul 2017 Simplify Paths (Bookmarks)
// 26 Jul 2017 Copy/Move: Simplify: Select from bookmarks
// 27 Jul 2017 Editor: Autocomplete: Filter repeats
//  7 Aug 2017 Bookmarks: simplify
//  8 Aug 2017 Memory Leaks: Detect and remove
// 15 Aug 2017 Add Log function
//  7 Sep 2017 Highlight Link files
//  8 Sep 2017 Link files: Highlight and show Target
// 11 Sep 2017 Edit: Multiple Bookmarks
// 15 Sep 2017 File View: Add Help
//	       File View: Add ^Go command
//	       Edit: Add ^Z clear to end of line
//             Edit: Add ^Save.  Move ^Setup to ^U.
// 14 Mar 2018 Permissions: bug fixes re directories
//             Rename: Start auto numbering at 1 (not 0)
//  5 Apr 2018 Cosmetic changes
// 13 Apr 2018 Colour Themes from command line parameter: 1 2 3 and ^T
// 18 Apr 2018 Move Colour Theme to Setup
//             Editor: Variables into a single struct
// 24 Apr 2018 Editor changes
//             Editor cursor restore most recent files
// 29 Apr 2018 Editot: Add Undo
//  1 May 2018 Copy/Move: Create destination directory tree
//  4 May 2018 Directory Format setup
//  8 May 2018 Permissions -> Properties + Add DateTime
// 19 May 2018 File Open: Find associated shell script: EXT.sh and exec EXT.sh FILENAME PATH
// 23 May 2018 Edit: Find includes wild chars: ? # @
// 26 May 2018 Add Scroll Bars
//  3 Jun 2018 Copy/Move: Destination newer/occupied: Add ReverseCopy, Rename
// 14 Jun 2018 Find: Add File Contents, Unique Only, Duplicated Only
// 19 Jun 2018 Add ^Viewlog
// 21 Jun 2018 Add Sync Directories
//  2 Jul 2018 File Open: Windows: Call ShellExecuteA ("open") instead of lookinf for a bash script
//  6 Jul 2018 setUp:
//               Copy/Move: Add DateTime tolerance (automated)
//               Copy/Move: Don't copy files with ????
// 10 Jul 2018 setUp: Add Search: allow wild
//  4 Aug 2018 Breakdown Summary by Extension or Directory
//             Files: Organize into Date etc Directories
//  6 Sep 2018 Dir: Sort: Replace Shell sort with Merge. Much faster
//  7 Sep 2018 Setup: Enter key on file: Edit or Open
// 14 Sep 2018 Setup: Add external editor (for Paul)
// 24 Sep 2018 Setup: Show files: Add Directory Size: Item Size / Sum of all contents recursively
//  2 Oct 2018 Edit: AltDel => Delete End of Line
//                   CtrlDel => Delete Word
// 26 Oct 2018 OrganiZe: Add extension to mask
// 12 Nov 2018 sYnc: Add pending opertions to Log //Add List option
// 18 Nov 2018 Setup: Copy/sYnc: Tollerate whole hour differences in File.DateTime
// 27 Feb 2019 Copy/Sync: Possible bug fix
// 18 Mar 2019 Copy/Sync: Fix bugs
// 24 Mar 2019 Edit: Pascal syntax. Bug fixes
// 17 Jun 2019 Move: If rename fails, Copy then Delete
// 23 Jun 2019 Permissions: Fixed problem with Directory attributes
//             New: Add create soft link (no hard link yet)
//  3 Jul 2019 console.c: Create and use PutCharPlain (c) in string output
// 30 Jul 2019 Edit: Find: ^P => Find Previous
//             Add Edit Hex (^H)
//             UTF character now show as n "bright" $ characters
// 06 Aug 2019 Start alpha menu system
// 30 Sep 2019 Bug fixes: Syncing, Edit:Block Format, Seek
// 03 Oct 2019 Config Save/Load: Use *char[] for field names
// 04 Dec 2019 Respond immediately to Terminal Size Change
// 08 Dec 2019 Add param -resize: This disallows resizing
// 16 Dec 2019 Open: Use xdg-open if no ~/EXT.sh
// 04 Mar 2020 Edit: Block Compare with Clipboard
// 28 Mar 2020 Better Windows support (but is still slowwwwwww)
// ?? Apr 2020 Summary: Allow selection. Tags matching file OR goes to directory
// 25 Apr 2020 Copy/Move/Delete: Add summary
// 12 Jun 2020 Summary,Tree: Add Seek by typing
// 19 Sep 2020 Copy/Move:
//               From "Search" list: Destination includes the local path (as displayed)
//               New File Exists promt: 'A'll added
//             Breakdown: "SPACE": Tags the corresponding Files. "Enter": goes to the first matching
//             Directory List: Add sort by Owner/Group
//             Rename: Target mask allows extending original name/ext. Eg "*xx.*"
//             Edit: SaveAs: Includes text format [LF/CR+LF/CR]. ^F to change
//             ^Del: If on spaces (and not following a word), deletes spaces
//             Find/Edit-Find/Contents: Add ^F to cycle thru Find Modes
// 20 Nov 2020 Setup: Include local path in Destination
// 16 Oct 2020 Change Case: allow for _
//  8 Jan 2020 Edit: soft syntax highlights: *.syntax
// 12 Jan 2021 Add Tag/Untag using Shift + arrow/pgup/pgdn/home/end
// 18 Jan 2021 Colour Themes: Keep in 2D array
// 28 Jan 2021 Configs in sub folder
//             Separate text files for Syntaxs
// 10 Mar 2021 Find: WildMode: ~<char> = match != <char>
// 21 Mar 2021 Separate text files for Help: *.hlp
// 26 Mar 2021 StrPos: Wild: allow ^<char>
//             Display all Macros
// 21 Apr 2021 Custom Colours
// 23 Jun 2021 Find File: Add Size
// 29 Sep 2021 Syntax, Open: Add ExtensionAlias
// 07 Oct 2022 Copy/Move: Skip SymLinks
// 31 May 2023 Tidy Bookmarks & tab, IP display
// 23 Jun 2024 Add Time Offset
// 17 Sep 2024 File Search: Same DateTime check: use FileDateTimeCompare instead of ==
// 21 Sep 2024 ReadDirCallBack: Fixed Error: no longer searcher Dir "file" for contents
// 24 Oct 2024 keystrokes: byte -> word.  Macros expanded ^Fn, Sh-Fn
//
// TO DO
// -----
// / Edit: Bug after deleting everything
// / Summary: way of showing part tag
// / HelpLine: show shift
//Optimize change colour codes for special chars
//Edit: UDF-8?
//Edit: bug with tab
//Use Special characters creatively
// Make clipboard available in EditString (if single line)
//   Change EditString_: Process ^V
// ?Copy: To SD card: Date out by exactly 1 month
//####Compare: Sometimes misses resync
// Mount Device (from /dev) into selected empty Folder
// Break up big functions, Menu() not to call ProcessCommand(c)
// Attributes: Make ls compatible: attr[0] = {-|d|l|b|c|p|s}
// Edit:
//   Handle text type header
//   Support UTF-8, UTF-16 correctly
// Paul Ideas:
//   Use environment var for "Config" instead of "home" (see XDG.CONFIG specification)
//   / Set colours individually
//   Key bindings
//   Config: Split into .fbc & .fbchistory
//   Undo: group single line edits etc
//   ??? use environment var "Editor"
//
////////////////////////////////////////////////////////////////////////////


const char AppName [] = "fbc";
const char Revision [] = "4.442";

#define _FILE_OFFSET_BITS 64   // Support for large files

#include <malloc.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _Windows
  #include <io.h>
  #include <Shlobj.h>
  #include "..\Lib\Lib.c"
  #include "..\Lib\Console.c"
  #include "..\Lib\ConsoleLib.c"
  #include "..\Lib\Dir.c"
  #include "..\Lib\About.c"
  #include "..\Lib\EditFields.c"
  #include "..\Lib\ShowGenericPage.c"
#else
  #include <linux/limits.h>
  #include <pwd.h>
  #include <grp.h>
  #include "../Lib/Lib.c"
  #include "../Lib/Console.c"
  #include "../Lib/ConsoleLib.c"
  #include "../Lib/Dir.c"
  #include "../Lib/About.c"
  #include "../Lib/EditFields.c"
  #include "../Lib/ShowGenericPage.c"
#endif

#ifdef _Windows
  #define F_OK 0
#endif

#define ListingColumnsAttributes 0x01
#define ListingColumnsOwners 0x02
#define ListingColumnsDateTime 0x04
#define ListingColumnsSize 0x08
#define ListingColumnsMax 0x0F

typedef struct
  {
    enum {ctBlack, ctWhite, ctBlue, ctZZZZ} ColourTheme;
    bool CopySyncTollerateHours;
    char CopyIgnore [100];
    bool CopyMoveLocalPath;
    enum {eoEdit, eoOpen, eoZZZZ} EnterOp;
    bool ExecuteReadDir;
    char ExternalEditor [32];
    //
    unsigned int ListingColumns;   // Columns to display. See below
    //
    _StrPosMode FindModeName;
    _StrPosMode FindModeContents;
    longint FindMaxSize;
    _SortMode SortMode;
  } _Setup;

_Setup Setup = {ctWhite, true, "", true, eoEdit, true, "", ListingColumnsDateTime | ListingColumnsSize, spmNoCase, spmNoCase, 0, sName};

char *ExtensionAlias [32];

#define CmdCopy   Cntrl('C')
#define CmdMove   Cntrl('O')
#define CmdDelete Cntrl('D')
//#define CmdRename Cntrl('R')
#define CmdNew    Cntrl('N')

char *DirStart;   // Starting Directory
char *Paths [8];   // Bookmarks
int PathsIndex;
char *DirItem;
int DirItemIndex = -1;
int HomeDirLen;

bool HelpShow = true;
bool TitleShow = true;

char Message [128];

bool Debug = false;
bool AllowResize = true;

bool DoRename;

//const char* FindModeNames [] = {"Strict", "Case Insensitive", "Allow Wilds (#@?)", NULL};//####

#include "fbccolours.c"
#include "compare.c"
#include "fbclog.c"
#include "fbclib.c"
#include "fbcfiling.c"
#include "fbctree.c"
#include "fbcsummary.c"

#define IncludeEdit

#ifdef IncludeEdit
  #include "fbcedit.c"
#endif


////////////////////////////////////////////////////////////////////////////
//
// MAJOR FUNCTIONS

void FilingStatsInit (void)
  {
    StatCopy = 0;
    StatMove = 0;
    StatDelete = 0;
  }

void FilingStatsShow (void)
  {
    char *s;
    //
    if (StatCopy + StatMove + StatDelete)
      {
        s = Message;
        StrToStr (&s, "Summary: ");
        if (StatCopy)
          {
            IntToStr (&s, StatCopy);
            StrToStr (&s, (char *) CommandName (CmdCopy, tPast));
          }
        if (StatMove)
          {
            IntToStr (&s, StatMove);
            StrToStr (&s, (char *) CommandName (CmdMove, tPast));
          }
        if (StatDelete)
          {
            IntToStr (&s, StatDelete);
            StrToStr (&s, (char *) CommandName (CmdDelete, tPast));
          }
        *s = 0;
      }
  }

_Action CopyMoveDeleteItem (char Command, _DirEntry *Item, char *PathDest, int RootLen)
  {
    char *Source, *Dest, *Dest_, *c;
    _DirEntry *Dir, *d;
    _Action go;
    struct stat st;
    //
    if (GetKey () == esc)   // escape key pressed
      return aAbort;
    go = aYes;
    Source = GetItemPath (Item); //MakeFilePath (Item->Path, Item);
    if (Command != CmdDelete)
      if (StrCompare (Source, PathDest) == 0)
        go = aSkip;
      else if (Item->SymLink)
        go = aSkip;
    Dest = NULL;
    Dest_ = malloc (MaxPath);
    c = Dest_;
    if (PathDest)
      {
        StrToStr (&c, PathDest);
        if (RootLen && Setup.CopyMoveLocalPath)
          if (StrLength (Item->Path) > RootLen)
            {
              CharToStr (&c, PathDelimiter);
              StrToStr (&c, &Item->Path [RootLen]);
            }
        *c = 0;
        Dest = GetItemPathFrom (Dest_, Item);
        if (Command != CmdDelete)
          if (StrCompare (Source, Dest) == 0)   // Source & Destination the same
            go = aSkip;
          else   // Make sure destination dir exists
            if (stat (Dest_, &st))   // Directory does NOT exist
              if (!MakePath (Dest_))
                go = aError;
      }
    free (Dest_);
    if (go == aYes)
      {
        if (Item->Directory)
          {
            if (go == aYes)
              if (chdir (Item->Path) != 0)
                go = aError;
            if (go == aYes)
              {
                if (chdir (Item->Name) != 0)
                  go = aError;
                Dir = NULL;
                ReadDirSearch (&Dir, NULL, rdmSingle, NULL, 0);
                d = Dir;
                while (true)
                  {
                    if (d == NULL)
                      break;
                    go = CopyMoveDeleteItem (Command, d, Dest, 0);
                    if ((go == aError) || (go == aAbort))
                      break;
                    d = d->Next;
                  }
                FreeDir (Dir);
                if ((Command == CmdDelete) || (Command == CmdMove))
                  if (go == aYes)
                    {
                      if (chdir (Item->Path) != 0)
                        go = aError;
                      if (go == aYes)
                        go = deleteFolder (Item->Name);
                    }
              }
          }
        else   // a file
          {
            //ShowActionFile (Command, Source);
            switch (Command)
              {
                case CmdCopy:   go = copyFile (Source, Dest); break;
                case CmdMove:   go = moveFile (Source, Dest); break;
                case CmdDelete: go = deleteFile (Source);     break;
              }
          }
      }
    CheckAction (Item, &go);
    free (Source);
    if (Dest)
      free (Dest);
    return go;
  }


/////////////////////////////////////////////////////////////////////
//
// SYNC DIRECTORY

int SyncFilesCopy;
int SyncNewerOverwrite;
int SyncDirectoriesDelete;
int SyncFilesDelete;

_Action SyncDirectory (_DirEntry *Item, char *PathDest, bool Pass)
  {
    char *Source, *Source_;
    char *Dest, *Dest_;
    _DirEntry *DirSource;
    _DirEntry *DirDestination;
    _DirEntry *ds, *dd;
    bool Copy, Newer;
    _Action Res;
    char St [MaxPath], *s;
    //
    Res = aError;
    Source = GetItemPath (Item);
    Dest = GetItemPathFrom (PathDest, Item);
    DirSource = NULL;
    DirDestination = NULL;
    if (Item->Directory)
      {
        if (chdir (Source) == 0)
          {
            ReadDirSearch (&DirSource, NULL, rdmSingle, NULL, 0);
            if (MakePath (Dest))
              {
                if (chdir (Dest) == 0)
                  {
                    ReadDirSearch (&DirDestination, NULL, rdmSingle, NULL, 0);
                    Res = aYes;
                  }
              }
          }
        CheckAction (Item, &Res);
        // Go thru Source directory
        ds = DirSource;
        while (true)
          {
            if (Res != aYes)
              break;
            if (ds == NULL)
              break;
            // Enter Directories Recursively
            if (!ds->SymLink)  // an actual file
              {
                Copy = false;
                Newer = false;
                // Find d->Name in Destination
                dd = DirDestination;
                while (true)
                  {
                    if (dd == NULL)   // not found
                      {
                        Copy = true;
                        break;
                      }
                    if (StrCompare (dd->Name, ds->Name) == 0)   // found
                      {
                        if (ds->Directory)
                          Copy = true;
                        else
                          {
                            if (dd->Size != ds->Size)
                              Copy = true;
                            if (FileDateTimeCompare (dd->DateTime, ds->DateTime) != 0)
                              {
                                Copy = true;
                                if (FileDateTimeCompare (dd->DateTime, ds->DateTime) > 0)
                                  Newer = true;
                              }
                          }
                        dd->Name [0] = 0;   // Mark this entry protected
                        break;
                      }
                    dd = dd->Next;
                  }
                if (Copy)
                  {
                    if (ds->Directory)
                      Res = SyncDirectory (ds, Dest, Pass);
                    else
                      {
                        if (Pass)
                          {
                            Source_ = GetItemPath (ds);
                            Dest_ = GetItemPathFrom (Dest, ds);
                            if (!CopyFileNow (Source_, Dest_, Newer))
                              Res = aError;
                            free (Source_);
                            free (Dest_);
                            Source_ = Dest_ = NULL;
                          }
                        else
                          {
                            SyncFilesCopy++;
                            if (Newer)
                              SyncNewerOverwrite++;
                            s = St;
                            StrToStr (&s, "Copy Required ");
                            StrToStr (&s, ds->Path);
                            CharToStr (&s, PathDelimiter);
                            StrToStr (&s, ds->Name);
                            if (Newer)
                              {
                                StrToStr (&s, " Newer by ");
                                StrTimeDelta (&s, dd->DateTime - ds->DateTime);
                              }
                            *s = 0;
                            LogWrite_ (St);
                          }
                      }
                  }
              }
            CheckAction (ds, &Res);
            ds = ds->Next;
          }
        // Any items left in the destination are to be deleted
        dd = DirDestination;
        while (true)
          {
            if (Res != aYes)
              break;
            if (dd == NULL)
              break;
            if (dd->Name [0])
              {
                if (Pass)
                  Res = CopyMoveDeleteItem (CmdDelete, dd, NULL, 0);
                else
                  {
                    if (dd->Directory)
                      SyncDirectoriesDelete++;
                    else
                      SyncFilesDelete++;
                    s = St;
                    StrToStr (&s, "Delete Required ");
                    StrToStr (&s, dd->Path);
                    CharToStr (&s, PathDelimiter);
                    StrToStr (&s, dd->Name);
                    *s = 0;
                    LogWrite_ (St);
                  }
              }
            CheckAction (ds, &Res);
            dd = dd->Next;
          }
      }
    //CheckAction (dd, &Res);
    if (Source)
      free (Source);
    if (Dest)
      free (Dest);
    if (DirSource)
      FreeDir (DirSource);
    if (DirDestination)
      FreeDir (DirDestination);
    //CheckAction (Item, &Res);
    return Res;
  }

/*
/////////////////////////////////////////////////////////////////////
//
// HELP PAGE

const char *HelpPage [] =
  {
    "_FBC - FILE BROWSER for CONSOLE",
    "",
    "Displays and manipulates files and directories intuitively",
    "Commands operate on multiple objects when Tagged",
    "Symbolic Links (*) are listed with the target item (=>)",
    "Pipes (>) are listed",
    "Title bar shows the current directory and statistics",
    "Sorting column is highlighted",
    "Inside many commands: |Up| / |Down| / |Tab| cycle thru parameters, past entries and Bookmarks",
    "Command line: {Initial Path || |0| || |1| || |2|}",
    // about the current directory or the Tagged items",
    "",
    "_Change Selection:",
    "|Up| / |Down|	Change Selection",
    "|PgeUp|	Top Page / Page Up",
    "|PgeDn|	Bottom Page / Page Down",
    "|Home|	Select First",
    "|End|	Select Last",
    "|Sh-|(above)	Tag/Untag while changing selection",
    "|a|-|z| |0|-|9|	Type part of a name to jump there. Works for all lists",
    "",
    "_Commands",
    "|SPACE|	Tag/Untag selected item",
    "|^A|ll 	Tag/Untag all items",
    "|Enter|	Move into selected Directory",
    "	Edit / Execute / Open File",
    "	Go to Target of a Symbolic Link",
    "|Esc|	Move to Parent Directory",
    "|Tab|	Cycle between Bookmarks",
    "|F5|	Re-read Directory (see Setup {6})",
    "|^F|ind	Find Files within current directory. |Up| and |Down| to change parameters:",
    "	{1} Named - specify the file name (or part thereof)",
    "	  Change Find Mode |^F|: Strict, NoCase OR Wild (|#@&?|)",
    "	    Strict: exact match required",
    "	    NoCase: letter case is ignored",
    "	    Wild: |#|=0-9, |@|=a-z,A-Z, |&|=any nonletter |?|=anything |~<char>|=anything but <char>",
    "	      eg |@@@@-####|, |&word&|, |abc~d|",
    "	{2} Containing (optional)",
    "	  Change Find Mode |^F|: as above",
    "	{3} Search sub directories (Yes = recursive search)",
    "	{4} Inclusions: All files, Unique files or Duplicate files",
    "	{5..7} Only when Inclusions is not All files, Specify uniqueness:",
    "	  {5} Name must be unique, {6} Size .., {7} Datetime ..",
    "	Results are listed with relative paths*",
    "|^G|oto	Go to a directorty. Creates a bookmark first",
    "|/| 	Menu: Bring up the menu: |F|ile |T|ag |M|anage |V|iew |S|etup |Q|uit",
    "|^Q|uit",
    "",
    "",
    "_|F|ile Menu:",
#ifdef _Windows
    "|O|pen selected file (|^\\| or |Alt-Enter|): using Windows file association",
#else
    "|O|pen selected file (|^\\| or |Alt-Enter|):",
    "  Run bash script: |ext.sh FILENAME FILEPATH| if it exists, otherwise use |xdg-open FILENAME|",
    "    |ext| is the file extension lower-cased. Eg |floom.Txt| will use |txt.sh|",
    "    FILENAME is the local filename with extension",
    "    FILEPATH is the current directory",
#endif
    "|E|dit (|^E|): Text edit selected file with the built in editor",
    "  This includes syntax highlighting, macros, auto complete, formatting, conversion ...",
    "  The Syntax is defined in |ext.syntax| where |ext| is the file extension lower-cased",
    "  An alternative editor can be selected in Setup {7}",
    "Edit |H|ex (|^H|): Edit selected file in raw hex",
    "e|X|ecute (|^X|): Run a command line, default is the selected item",
    "|N|ew (|^N|): Create a new File, Directory or Link",
    "|C|opy (|^C|) seleted item(s)",
    "|M|ove (|^O|) seleted item(s)",
    "|D|elete (|Del|) seleted item(s) permanently",
    "|R|ename (|^R| or |F2|) seleted item(s) using a mask: Examples",
    "   Change name: |nnnnn.xxx|",
    "   Change name, keep extensions: |nnnnn.*|",
    "   Keep name, change extension: |*.xxx|",
    "   Auto increment portion: |FileNo###.*|",
    "   Auto increment prepending original name: |##*.*|",
    "|P|roperties (|^P|): Change attributes of seleted item(s)",
    "   Parameters: (use |Up| / |Down|). Only modified parameters are applied",
    "     {1} Attributes: Use |Left| / |Right| to move and |SPACE| to toggle",
    "     {2} Owner",
    "     {3} Group",
    "     {4} Date and Time",
    "|T|ime (|^T|): Offset the file time by whole hours of selected item(s)",
    "",
    "_|T|ag Menu:",
    "|A|ll	Tag / Untag all (|^A|)",
    "|I|nvert	Reverse tagging",
    //"|T|o-here	Tag/Untag from previous tag (or top) to current (|^T|)",
    "|E|nd	Tag/Untag to end",
    "",
    "_|M|anage Menu:",
    "|S|ync: Duplicates Directories into the Destination",
    "   Only copies files / directories as needed",
    "   Deletes unmatched files / directories from the Destination",
    "   Scans first then presents an action summary before proceeding",
    "   The list of required copy and delete operations are added to the log",
    "|O|rganize: Moves all files into created directories by date and/or extension:",
    "  A Mask determines the directory structure",
    "  This can contain |X| for the file extension, |YYYY|, |YY|, |MM|, |DD| for the file Year, Month and Day",
    "  Examples: |YY/MM/DD YYYY-MM X/YYYY|",
    "",
    "_|V|iew Menu:",
    //"|^S|ort	Cycle between sorting columns",
    "|S|ort (|^S|) by Filename, Extension, Path*, Age, Size",
    //"tag|A|ll (|^A|) items (or none)",
    "|C|olumns to display: Attributes, Owners, Date Time, Size",
    "|T|ree: Show tree of local Directories",
    "  Navigate to desired location, examining statistics",
    "  Press |Enter| to go there otherwise press |Esc|",
    "|B|reakdown (|^B|): Shows file list grouped by Extension or Directory with statistics:",
    "  Columns are Count, Total Size, Extension/Directory",
    "  |^S|ort by different columns",
    "  |Enter| go to the Directory OR Tag matching Extensions",
    "  |Esc| to quit",
    "|D|irSize (|^L|): Show Directory size of file contents (recursive)",
#ifdef _Windows
    "|L|og: Opens fbc operation log, \\fbc.log",
#else
    "|L|og: Opens fbc operation log, ~/fbc.log",
#endif
    "",
    "_|S|etup Menu (|^U|): |Up| and |Down| to change parameters:",
    "  {1} Colour theme [^Custom]: Pich from 3 default colour palettes",
    "      Type ^C to adjust each colour indiviually",
    //"  {2..5} File listing: Columns to be displayed",
    "  {2} Copy/Sync: Tolerance regional time differences (whole hours) when checking existing files",
    "  {3} Copy: Ignore files containing ANY of these (space separated) items",
    "  {4} Search: Copy/Move: Include local path in Destination",
    "  {5} Default action of the |Enter| key on a non-executable file",
    "  {6} Execute/Open/Edit: Reread Directory",
    //"  {9} Directory Size: Show system size OR Show Total Contents Size (recursive)",
    "  {7} External text editor (blank for internal editor)",
    "",
    "_Resource Files: Syntax (and Shell scripts in Linux):",
    "  Searched for in order:",
    "    -Current Directory",
    "    -Starting Directory",
    #ifdef _Windows
    "    -Home Directory: $HOMEPATH",
    #else
    "    -Home Directory: $HOME",
    "    -Config Directory: ~/.config/fbc  (Linux only)",
    #endif // _Windows
    "    -Anywhere along $PATH"
    //"  The filename is always lower-cased before searching"
  };
*/


/////////////////////////////////////////////////////////////////////
//
// Main Commands

_DirEntry *Dir;   // Current Directory Listing
bool DirRead;   // True if Dir needs to be reread
bool DirShow;   // True if Dir need to be redisplayed

int DirCount [2];   // Files, Directories
longint DirSize;
int TaggedCount [2];   // Count of Tagged [Files, Directories]
longint TaggedSize;

void CalculateTagged (_DirEntry *Dir)
  {
    DirCount [0] = 0;
    DirCount [1] = 0;
    DirSize = 0;
    TaggedCount [0] = 0;
    TaggedCount [1] = 0;
    TaggedSize = 0;
    while (Dir)
      {
        DirCount [Dir->Directory] ++;
        if (!Dir->Directory)
          DirSize += (longint) Dir->Size;
        if (Dir->Tagged)
          {
            TaggedCount [Dir->Directory]++;
            if (!Dir->Directory)
              TaggedSize += (longint) Dir->Size;
          }
        Dir = Dir->Next;
      }
  }

int TaggedCountSum (void)
  {
    return TaggedCount [0] + TaggedCount [1];
  }

void PutTaggedDesc (_DirEntry *Dir)
  {
    if (TaggedCountSum () == 0)
      PutString (Dir->Name);
    else
      {
        if (TaggedCount [0])   // Files tagged
          {
            PutNumName (TaggedCount [0], "file", "files");
            if (TaggedCount [1])
              PutString (" and ");
          }
        if (TaggedCount [1])   // Directories tagged
          PutNumName (TaggedCount [1], "Directory", "Directories");
      }
    //PutString (": ");
  }


/////////////////////////////////////////////////////////////////////
//
// Edit File(s) Properties

const char *PropertiesFieldNames [] = {"Attributes", "Owner", "Group", "Date Time", NULL};
const char AttribTab [] = "rwxrwxrwx";

_DirEntry *PropSelected;
int PropMask;
char PropOwner [32];
char PropGroup [32];
struct tm *PropDateTime;
byte PropertiesChanged;

#define S_IALL (S_IRWXU|S_IRWXG|S_IRWXO)

bool SetPropertiesPermissions (_DirEntry *Item)
  {
    mode_t NewMode;
    bool OK;
    //
    OK = true;
    NewMode = (Item->Attrib & ~S_IALL) | (PropMask & S_IALL);
    if (PropertiesChanged & Bit [0])
      //if (!Item->Directory && !Item->SymLink)
      //if (!Item->Directory)
        if (chmod (Item->Name, NewMode) != 0)   // fail
          OK = false;
    #ifndef _Windows
    struct passwd *Onr;
    struct group *Grp;
    //
    StrAssign (&DirItem, Item->Name);
    if (PropertiesChanged & (Bit [1] | Bit [2]))
      if (!Item->SymLink)
        {
          Onr = getpwnam (PropOwner);
          Grp = getgrnam (PropGroup);
          if (Onr && Grp)
            if (chown (Item->Name, Onr->pw_uid, Grp->gr_gid) != 0)   // fail
              OK = false;
        }
    #endif
    return OK;
  }

bool SetProperties (_DirEntry *Item)
  {
    bool OK;
    time_t t;
    char s [128], *sp;
    //
    OK = true;
    SelectDirectory (Item);
    if (!SetPropertiesPermissions (Item))
      OK = false;
    t = mktime (PropDateTime);
    if (PropertiesChanged & Bit [3])
      if (!FileDateTimeWrite (Item->Name, t))
        OK = false;
    if (Log && PropertiesChanged)
      {
        LogWrite ("Permissions ");
        LogWrite (Item->Path);
        LogWrite ("/");
        LogWrite (Item->Name);
        sp = s;
        if (PropertiesChanged & Bit [0])
          {
            StrToStr (&sp, "  Attributes ");
            IntToStrBase (&sp, PropMask & 0x1FF, 9 | IntToLengthZeros, 2);
          }
        if (PropertiesChanged & Bit [1])
          {
            StrToStr (&sp, "  Owner ");
            StrToStr (&sp, PropOwner);
          }
        if (PropertiesChanged & Bit [2])
          {
            StrToStr (&sp, "  Group ");
            StrToStr (&sp, PropGroup);
          }
        if (PropertiesChanged & Bit [3])
          StrToStr (&sp, "  Date/Time");
        *sp = 0;
        LogWrite_ (s);
      }
    return OK;
  }

int PropertiesFieldEdit (int Field)
  {
    int c;
    //
    EditChanged = false;
    switch (Field)
      {
        case 0: // Attributes
          c = EditMask (&PropMask, AttribTab, 9);
          break;
        #ifdef _Windows
        case 1: // Owner
        case 2: // Group
          return -1;
        #else
        case 1: // Owner
          c = EditString_ (PropOwner, sizeof (PropOwner));
          break;
        case 2: // Group
          c = EditString_ (PropGroup, sizeof (PropGroup));
          break;
        #endif
        case 3: // Date Time
          while (true)
            {
              c = EditDateTime (PropDateTime);
              if ((c == esc) || (c >= 0))
                break;
            }
          break;
        default:
          c = esc;
      }
    if (EditChanged)
      PropertiesChanged |= Bit [Field];
    return c;
  }

// Prompt and Change Properties. Returns true is successful
bool Properties (_DirEntry *Selected)
  {
    _DirEntry *d;
    struct passwd *User;
    struct group *Group;
    _Action Res;
    //
    StrAssign (&DirItem, Selected->Name);
    if (!Selected)
      return false;
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Properties of ");
    PutTaggedDesc (Selected);
    PutNewLine ();
    PropSelected = Selected;
    PropMask = Selected->Attrib;
    #ifndef _Windows
    User = getpwuid (Selected->UID);
    if (User)
      StrCopyN (PropOwner, User->pw_name, sizeof (PropOwner));
    Group = getgrgid (Selected->GID);
    if (Group)
      StrCopyN (PropGroup, Group->gr_name, sizeof (PropGroup));
    #endif
    PropDateTime = localtime (&Selected->DateTime);
    PropertiesChanged = 0;
    if (EditFields (PropertiesFieldNames, PropertiesFieldEdit, Colours [ColQueryFG2]))
      {
        Res = aYes;
        if (TaggedCountSum () == 0)   // Single file / folder
          {
            if (!SetProperties (Selected))
              Res = aError;
            CheckAction (Selected, &Res);
          }
        else   // multiple files/folders
          {
            d = Dir;
            while (true)
              {
                if (d == NULL)
                  break;
                if (d->Tagged)
                  if (!SetProperties (d))
                    Res = aError;
                CheckAction (d, &Res);
                if (Res != aYes)
                  break;
                d = d->Next;
              }
          }
        return true;
      }
    return false;
  }
//#endif // _Windows

_Action YearMonthDayExtItem (_DirEntry *Item, char *Mask)
  {
    _Action Res;
    struct tm *tmp;
    char *Dest, *Ext;
    char *d, *dd, *m;
    unsigned int p;
    //
    if (Item->Directory)
      return aYes;
    Res = aError;
    tmp = localtime (&Item->DateTime);
    Ext = StrGetFileExtension (Item->Name);
    // Build destination path
    Dest = malloc (MaxPath); //(strlen (Item->Name) + strlen (Item->Path) + strlen (Mask) + strlen (Ext) + 32);
    d = Dest;
    if (HomeDirLen)
      StrToStrN (&d, Item->Path, HomeDirLen);
    else
      StrToStr (&d, Item->Path);
    CharToStr (&d, PathDelimiter);
    m = d;   // Start of Mask in Dest
    StrToStr (&d, Mask);
    *d = 0;
    if (tmp)
      {
        // Substitute date fields
        p = StrPos_ (m, "YYYY");
        if (p != -1)
          {
            dd = &m [p];
            IntToStrFill (&dd, tmp->tm_year + 1900, 4 | IntToLengthZeros);
          }
        p = StrPos_ (m, "YY");
        if (p != -1)
          {
            dd = &m [p];
            IntToStrFill (&dd, tmp->tm_year % 100, 2 | IntToLengthZeros);
          }
        p = StrPos_ (m, "MM");
        if (p != -1)
          {
            dd = &m [p];
            IntToStrFill (&dd, tmp->tm_mon + 1, 2 | IntToLengthZeros);
          }
        p = StrPos_ (m, "DD");
        if (p != -1)
          {
            dd = &m [p];
            IntToStrFill (&dd, tmp->tm_mday, 2 | IntToLengthZeros);
          }
      }
    if (Ext)
      {
        p = StrPos_ (m, "X");
        if (p != -1)
          {
            StrDelete (&m [p], 1);
            if (Ext [0])   // any extension
              StrInsertStr (&m [p], &Ext [1]);
            else
              StrInsertStr (&m [p], "NoExt");
          }
      }
    // Make the path and move file into it
    if (StrCompare (Dest, Item->Path) != 0)   // Is Dest different to current location
      {
        chdir (Item->Path);
        if (MakePath (Dest))
          {
            chdir (Item->Path);
            StrAppend (Dest, PathDelimiter);
            StrConcat (Dest, Item->Name);
            Res = moveFile (Item->Name, Dest);
          }
      }
    free (Dest);
    CheckAction (Item, &Res);
    return Res;
  }


/////////////////////////////////////////////////////////////////////
//
// Time Offset File(s)

bool TimeOffset (_DirEntry *Item, long Offset)
  {
    time_t t;
    char St [MaxPath], *s;
    //
    s = St;
    StrToStr (&s, "Offset ");
    StrToStr (&s, Item->Path);
    CharToStr (&s, PathDelimiter);
    StrToStr (&s, Item->Name);
    StrToStr (&s, " by ");
    IntStrToStrFirst = true;
    IntStrToStr (&s, Offset, "hour", "hours");
    //IntToStr (&s, Offset);
    //StrToStr (&s, " hours");
    *s = 0;
    // Show progress
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString (St);
    // Perform the Offset
    t = Item->DateTime + Offset * 60 * 60;
    if (!FileDateTimeWrite (Item->Name, t))
      return false;
    if (Log)
      LogWrite_ (St);
  }

bool CommandTimeOffset (_DirEntry *Selected)
  {
    _DirEntry *d;
    _Action Res;
    long Offset;
    //
    StrAssign (&DirItem, Selected->Name);
    if (!Selected)
      return false;
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Time Offset ");
    PutTaggedDesc (Selected);
    PutString (" by (+/- hours): ");
    Offset = 0;
    ConsoleColourFG (Colours [ColQueryFG2]);
    if (EditLongint (&Offset) == KeyEnter)
      if (Offset)
        {
          Res = aYes;
          if (TaggedCountSum () == 0)   // Single file / folder
            {
              if (!TimeOffset (Selected, Offset))
                Res = aError;
              CheckAction (Selected, &Res);
            }
          else   // multiple files/folders
            {
              d = Dir;
              while (true)
                {
                  if (d == NULL)
                    break;
                  if (d->Tagged)
                    if (!TimeOffset (d, Offset))
                      Res = aError;
                  CheckAction (d, &Res);
                  if (Res != aYes)
                    break;
                  d = d->Next;
                }
            }
          return true;
        }
    return false;
  }


/////////////////////////////////////////////////////////////////////
//
// EXECUTE SHELL COMMAND

char* Commands [32];

void ExecuteCommand (char *Command)
  {
#ifdef _Windows
    char *Command_;
    char *Shell;
#else
#endif
    //
    ConsoleUninit (false);
    ConsolePrompt (ColWhite, ColBlack | ColBright);
    //PutString (Paths [PathsIndex]);
    //PutNewLine ();
    PutChar ('$');
    PutString (Command);
    PutNewLine ();
    //ConsoleFoot ();
#ifdef _Windows
    Shell = getenv ("COMSPEC");
    Command_ = (char *) malloc (strlen (Command) + 16);
    strcpy (Command_, "/k");
    strcat (Command_, Command);
    ShellExecuteA (0, NULL /*"open"*/, Shell, Command_, NULL, SW_RESTORE);
    free (Command_);
#else
    system (Command);
    //execl ("/bin/sh", "sh", "-c", Command, (char *) 0);
#endif
    ConsoleInit (false);
    LogWrite ("Execute ");
    LogWrite_ (Command);
    PutNewLine ();
    PutNewLine ();
    ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
    WaitKey ();
    DirShow = true;
    if (Setup.ExecuteReadDir)
      DirRead = true;
  }

bool Execute (_DirEntry *Item)
  {
    ConsolePrompt (ColWhite, ColBlack | ColBright);
    if (Item)
      {
        StrAssign (&DirItem, Item->Name);
        PutString (Item->Path);
      }
    //PutNewLine ();
    PutString ("\n$ ");
    if (EditStringArray (Commands, SIZEARRAY (Commands), 128, Item ? Item->Name : NULL) == KeyEnter)
      {
        //ConsoleFoot ();
        if (Item)
          SelectDirectory (Item);
        //ConsoleClear (ColWhite, ColBlack);
        ExecuteCommand (Commands [0]);
        return true;
      }
    return false;
  }

bool ExecuteScript (_DirEntry *Item, bool Force)
  {
    char *Script, *ScriptPath, *Command, *s;
    int l, x;
    bool Res;
    //
    ConsolePrompt (ColWhite, ColBlack | ColBright);
    StrAssign (&DirItem, Item->Name);
    SelectDirectory (Item);
#ifdef _Windows
    x = ShellExecuteA (GetDesktopWindow (), "open", Item->Name, NULL, NULL, SW_SHOW);
    Res = x > 32;
#else
    // Find Script name & allocate strings
    Res = false;
    x = StrPosLastCh (Item->Name, '.');
    l = StrLength (Item->Name);
    if ((x > 0) && (l > x + 1))
      {
        // Build script file name
        Script = malloc (PATH_MAX);
        s = Script;
        StrToStr (&s, &Item->Name [x + 1]);
        StrToStr (&s, ".sh");
        *s = 0;
        StrToLower (Script);   // map Extension to lower case
        ScriptPath = FindFileResource (Script);
        Command = malloc (PATH_MAX);
        s = Command;
        if (!FileExists (ScriptPath))   // Shell script missing
          {
            // Build an xdg-open Command line
            StrToStr (&s, "xdg-open \"");
            StrToStr (&s, Item->Name);
            StrToStr (&s, "\"");
            *s = 0;
          }
        else   // Shell Script found
          {
            // Build a Script file Command line with parameters: FileName FilePath
            StrToStr (&s, "bash \"");
            StrToStr (&s, ScriptPath);
            StrToStr (&s, "\" \"");
            StrToStr (&s, Item->Name);
            StrToStr (&s, "\" \"");
            StrToStr (&s, Item->Path);
            CharToStr (&s, '\"');
            *s = 0;
          }
        // Run the script
        SelectDirectory (Item);
        ExecuteCommand (Command);
        Res = true;
        free (Command);
        free (Script);
        free (ScriptPath);
      }
#endif
    return Res;
  }


/////////////////////////////////////////////////////////////////////
//
// BATCH FILING

bool SelectDestination (bool Swap) //(_DirEntry *Dir, bool Revert)
  {
    while (true)
      {
        if (EditStringArray (Paths, SIZEARRAY (Paths), MaxPath, NULL) != KeyEnter)
          return false;
        ExpandPath (&Paths [0]);   // ~ expanded
        MakePath (Paths [0]);   // make entered directory
        if (chdir (Paths [0]) == 0)   // move to entered directory. Success?
          {
            StrAssign (&Paths [0], GetCurrentWorkingDirectory ());
            if (Swap)
              StringArrayPromote (Paths, SIZEARRAY (Paths), 1);
            return true;
          }
        ConsoleBeep ();
      }
  }
    /*
    int x0;
    int Col;
    int Index;
    char *St;
    int c;
    //
    St = malloc (MaxPath);
    x0 = ConsoleX;
    //Col = ConsoleFG;
    Index = PathsIndex;
    while (true)
      {
        ConsoleColourFG (Colours [ColQueryFG2]);
        ConsoleCursor (x0, ConsoleY);
        ConsoleClearEOL ();
        PutChar ('[');
        PutInt (Index + 1, 0);
        PutString ("] ");
        StrCopy (St, Paths [Index]);
        c = EditString (St, MaxPath, MaxPath);
        if (c == esc)
          {
            Index = -1;
            break;
          }
        if (c == KeyDown)
          Index = (Index + 1) % SIZEARRAY (Paths);
        else if (c == KeyUp)
          Index = (Index + SIZEARRAY (Paths) - 1) % SIZEARRAY (Paths);
        else if (c == cr)
          {
            ExpandPath (&St);   // ~ expanded
            MakePath (St);   // make entered directory
            if (chdir (St) == 0)   // move to entered directory. Success?
              {
                StringArrayAdd (Paths, SIZEARRAY (Paths), GetCurrentWorkingDirectory ());
                break;
              }
            else
            ConsoleBeep ();
          }
      }
    free (St);
    return Index;
  }*/

bool Rename (_DirEntry *Item, char *NewName, int Count)
  {
    char *NewName_;
    char *Ext;
    int pSlash, pDot, pAst, pHash;
    int nHash;
    bool OK;
    int n;
    char *s;
    char *St;
    //
    NewName_ = (char *) malloc (MaxPath);
    St = malloc (MaxPath);
    SelectDirectory (Item);
    StrCopy (NewName_, NewName);
    // Determine new extension or first part
    pSlash = StrPosLastCh (NewName, PathDelimiter);
    OK = false;
    if (pSlash < 0)   // Valid mask
      {
        // Check for wild * is first or extention part of original name
        while (true)
          {
            pAst = StrPosLastCh (NewName_, '*');
            if (pAst < 0)
              break;
            pDot = StrPosLastCh (NewName_, '.');
            Ext = StrGetFileExtension (Item->Name);
            if ((pAst < pDot) || (pDot < 0))   // replace name section
              {
                StrDelete (&NewName_ [pAst], 1);
                StrInsertStrN (&NewName_ [pAst], Item->Name, Ext - Item->Name);
              }
            else   // replace extension
              {
                StrDelete (&NewName_ [pAst], 1);
                if (*Ext)
                  StrInsertStrN (&NewName_ [pAst], &Ext [1], 0);
              }
          }
        // Check for #
        while (true)
          {
            pHash = StrPosCh (NewName_, '#');
            if (pHash < 0)
              break;
            s = &NewName_ [pHash];
            nHash = 0;
            while (*s++ == '#')
              nHash++;
            n = LogN (Count, 10);
            if (n > nHash)
              {
                StrInsertN (&NewName_ [pHash], n - nHash);
                nHash = n;
              }
            s = &NewName_ [pHash];
            IntToStrFill (&s, Count, nHash | IntToLengthZeros);
          }
        // Make status string
        s = St;
        StrToStr (&s, "Renaming ");
        StrToStr (&s, Item->Path);
        CharToStr (&s, PathDelimiter);
        StrToStr (&s, Item->Name);
        StrToStr (&s, " to ");
        StrToStr (&s, NewName_);
        *s = 0;
        // Show progress
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutString (St);
        // Perform the rename
        //if (!FileExists (NewName_))
          if (rename (Item->Name, NewName_) == 0)   // success
            {
              OK = true;
              StrAssign (&DirItem, NewName_);
              if (Log)
                LogWrite_ (St);
            }
      }
    free (NewName_);
    free (St);
    return OK;
  }

bool MakeLink (char *Name)
  {
    bool OK;
    //
    OK = false;
    #ifndef _Windows
    PutNewLine ();
    PutString ("Destination: ");
    if (SelectDestination (true))
      {
        chdir (Paths [0]);
        if (symlink (Paths [1], Name) == 0)
          OK = true;
      }
    #endif
    return OK;
  }

const char sNewFile [] = "NewFile";
const char sNewDirectory [] = "NewDirectory";
const char sNewLink [] = "NewLink";

bool New (void)
  {
    int f;
    _Action Res, Res_;
    char *pwd;
    //
    Res = aError;
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Create new ");
    switch (GetOption ("|F|ile, |D|irectory or |L|ink", Colours [ColQueryFG2]))
      {
        case 'F': // new File
        case KeyEnter:
          if (!FileExists ((char *) sNewFile))
            {
              f = FileOpen ((char *) sNewFile, foWrite);
              if (f >= 0)
                {
                  close (f);
                  StrAssign (&DirItem, (char *) sNewFile);
                  LogWrite ("Created File in ");
                  pwd = GetCurrentWorkingDirectory ();
                  LogWrite_ (pwd);
                  free (pwd);
                  Res = aYes;
                }
            }
          break;
        case 'D': // new Directory
          if (MakeDirectory ((char *) sNewDirectory))
            {
              StrAssign (&DirItem, (char *) sNewDirectory);
              LogWrite ("Created Directory in ");
              pwd = GetCurrentWorkingDirectory ();
              LogWrite_ (pwd);
              free (pwd);
              Res = aYes;
            }
          break;
        case 'L': // new Link
          if (MakeLink ((char *) sNewLink))
            {
              StrAssign (&DirItem, (char *) sNewLink);
              Res = aYes;
            }
          break;
        case esc:
          Res = aAbort;
          break;
      }
    Res_ = Res;
    CheckAction (NULL, &Res);
    return Res_ == aYes;
  }

byte EditStringFind (char *St, int StLen, _StrPosMode *StrPosMode)
  {
    byte Res;
    int x;
    //
    x = ConsoleX - 2;
    while (true)
      {
        ConsoleColourFG (Colours [ColQueryFG1]);
        ConsoleCursor (x, ConsoleY);
        PutStringHighlight (" [|^F|indMode=", Colours [ColQueryFG2]);
        PutString (StrPosModeNames [*StrPosMode]);
        PutStringHighlight ("]: |", Colours [ColQueryFG2]);
        Res = EditString_ (St, StLen);
        if (Res == Cntrl ('F'))
          *StrPosMode = (*StrPosMode + 1) % (spmZZZZ);
        else
          break;
      }
    return Res;
  }

const char *FindFieldNames [] = {"Named", "Containing", "Max Size", "Search Sub Folders",
                                 "Include", "Unique Name", "Unique Size", "Unique DateTime", NULL};
char FindTargetName [100];
char FindTargetContaining [100];
bool FindRecurse;
enum {fAll, fUniques, fDuplicates} FindSubset = fAll;
unsigned int FindSubsetCriteria = 0x07;

const char *FindSubsetNames [] = {"All files", "Unique files", "Duplicate files", NULL};

int FindFieldEdit (int Field)
  {
    switch (Field)
      {
        case 0: // File Name
          return EditStringFind (FindTargetName, sizeof (FindTargetName), &Setup.FindModeName);
        case 1: // File Containing
          return EditStringFind (FindTargetContaining, sizeof (FindTargetContaining), &Setup.FindModeContents);
        case 2: // Max Size
          return EditLongint (&Setup.FindMaxSize);
        case 3: // Recurse
          return EditBool (&FindRecurse);
        case 4: // Subset
          return EditEnum ((_enum *) &FindSubset, FindSubsetNames);
        case 5: // Unique match Name/Size/DateTime
        case 6:
        case 7:
        default:
          if (FindSubset == fAll)
            return -1;
          return EditBit (&FindSubsetCriteria, Bit [Field - 5]);
      }
  }

void DirAdd (_DirEntry **Dir, _DirEntry *Item)
  {
    *Dir = malloc (sizeof (_DirEntry));
    MemMove (*Dir, Item, sizeof (_DirEntry));
    (*Dir)->Next = NULL;
  }

void DirSubset (_DirEntry **Dir)
  {
    _DirEntry **d1, *d2;
    //
    if (FindSubset == fAll)
      return;
    // Tag duplicates
    d1 = Dir;
    while (true)
      {
        if (d1 == NULL)   // reached the end of Dir
          break;
        d2 = *d1;
        while (true)
          {
            if (d2 == NULL)   // end reached
              break;
            d2 = d2->Next;
            if (d2 == NULL)   // end reached
              break;
            if (!d2->Tagged)
              if ((~FindSubsetCriteria & 0x01) || (StrCompare (d2->Name, (*d1)->Name) == 0))
                if ((~FindSubsetCriteria & 0x02) || (d2->Size == (*d1)->Size))
                  if ((~FindSubsetCriteria & 0x04) || (FileDateTimeCompare (d2->DateTime, (*d1)->DateTime) == 0))
                    {
                      (*d1)->Tagged = true;
                      d2->Tagged = true;
                    }
          }
        d1 = (_DirEntry **) *d1;
      }
    // Remove unwanted
    d1 = Dir;
    while (true)
      {
        if (d1 == NULL)
          break;
        if (*d1 == NULL)
          break;
        if ((*d1)->Tagged != (FindSubset == fDuplicates))   // unwanted Item
          {
            // Delete item from linked list
            d2 = *d1;
            *d1 = d2->Next;
            FreeDirItemContents (d2);
            free (d2);
          }
        else
          {
            (*d1)->Tagged = false;
            d1 = (_DirEntry **) *d1;
          }
      }
  }

bool AnalyseDirectories;

_DirEntry *Find (void)
  {
    _DirEntry *Dir;
    //
    //FindTargetContaining [0] = 0;
    FindRecurse = true;
    //FindSubset = fAll;
    //FindSubsetCriteria = 0x07;   // Unique Test: Match Name,Size&Date
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Find Files ");
    if (EditFields (FindFieldNames, FindFieldEdit, Colours [ColQueryFG2]))
      {
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutString ("Searching ...");
        AnalyseDirectories = false;
        Dir = NULL;
        ReadDirSearch (&Dir, FindTargetName, FindRecurse?rdmRecurse:rdmSingle, FindTargetContaining, Setup.FindMaxSize);
        DirSubset (&Dir);
        if (Dir == NULL)
          StrCopy (Message, "No items found   :( ");
        return Dir;
      }
    return NULL;
  }


////////////////////////////////////////////////////////////////////////////
//
// Directory Show

int YOffset = 0;
int YSel = 0;

//char *PathDest;

void ShowAttrib (_DirEntry *Item)
  {
    int m;
    char *p;
    //
    //PutHHHHHH (Item->Attrib);
    //PutChar (' ');
    PutChar (Item->Directory ? 'd' : '-');
    PutChar (Item->SymLink   ? 'l' : '-');
    p = (char *) AttribTab;
    for (m = 0x100; m > 0; m = m >> 1, p++)
      if (*p != ' ')
        if (Item->Attrib & m)
          PutChar (*p);
        else
          PutChar ('-');
    PutChar (' ');
  }

void PutID8 (unsigned int ID)
  {
    int x;
    //
    x = ConsoleX;
    PutChar ('{');
    PutInt (ID, 0);
    PutChar ('}');
    PutCharN (' ', 8 - (ConsoleX - x));
  }

void ShowOwners (_DirEntry *Item)
  {
    #ifdef _Windows
    #else
    struct passwd *pw;
    struct group  *gr;
    //
    pw = getpwuid (Item->UID);
    gr = getgrgid (Item->GID);
    if (pw)
      PutStringN (pw->pw_name, 8);
    else
      PutID8 (Item->UID);
    PutChar (' ');
    if (gr)
      PutStringN (gr->gr_name, 8);
    else
      PutID8 (Item->GID);
    PutChar (' ');
    #endif
  }

void ShowItem (_DirEntry *Item, int line, bool Selected)
  {
    int FG, BG;
    //
    ConsoleSizeX--;
    FG = Colours [ColBodyFG];
    BG = Colours [ColBodyBG];
    if (Item)
      if (Selected)
        if (Item->Tagged)
          BG = Colours [ColBodyBGSelTag];
        else
          BG = Colours [ColBodyBGSel];
      else   // not selected
        if (Item->Tagged)
          BG = Colours [ColBodyBGTag];
    if (Item)
      if (Item->Directory)
        FG = Colours [ColBodyFGDir];
      else if (Executable (Item))
        FG = Colours [ColBodyFGExe];
    ConsoleLine_ (line, FG, BG);
    if (Item)
      {
        // Attributes
        if (Setup.ListingColumns & ListingColumnsAttributes)
          ShowAttrib (Item);
        // Attributes
        if (Setup.ListingColumns & ListingColumnsOwners)
          ShowOwners (Item);
        // Date Time
        if (Setup.ListingColumns & ListingColumnsDateTime)
          {
            //if (Debug)
            //  PutInt (Item->DateTime, 0);
            //else
              PutDateTimeLocalize (Item->DateTime, "%Y %b %d %H%M:%S");
            PutChar (' ');
          }
        // Count (Analysis mode only)
        if (AnalyseDirectories)
          {
            if (Item->Directory)
              PutInt (Item->Count, 7);
            else
              PutCharN (' ', 7);
            PutChar (' ');
          }
        // Size
        if ((Setup.ListingColumns & ListingColumnsSize) || AnalyseDirectories)
          {
            PutInt (Item->Size, 12 | IntToLengthCommas);
            PutChar (' ');
          }
        // {Path}/Filename
        //ConsoleCursor (ConsoleX - 1, ConsoleY);   // go back one
        if (Item->SymLink)
          PutChar ('*');
        else if (S_ISFIFO (Item->Attrib))
          PutChar ('>');
        else
          PutChar (' ');
        if (HomeDirLen)
          {
            if ((int) StrLength (Item->Path) > HomeDirLen)
            //if (Item->Path)
              {
                PutString (&Item->Path [HomeDirLen]);
                PutChar (PathDelimiter);
              }
          }
        PutString (Item->Name);
        if (Item->SymLink)
          {
            PutString (" => ");
            PutString (Item->SymLinkTarget);
          }
      }
    ConsoleSizeX++;
  }

void PutDirCountSize (int Count [], longint Size)
  {
    //int Col;
    //
    //Col = ConsoleFG;
    //if (Count [0])
      {
        PutInt (Count [0], 0 | IntToLengthCommas);
        //if (Count [1])
          PutChar ('+');
      }
    //if (Count [1])
      {
        //ConsoleColourFG (Colours [ColBodyFGDir]);
        PutInt (Count [1], 0 | IntToLengthCommas);
        //ConsoleColourFG (Col);
      }
    PutChar (' ');
    PutIntScaled (Size);
    PutChar ('B');
  }

void ShowDirectoryTitle (void)
  {
    char Titl [20];
    longint Capacity;
    longint Free;
    //int i;
    //
    CalculateTagged (Dir);
    ConsoleLine (0, Colours [ColTitleFG1], Colours [ColTitleBG]);
    PutString ("FBC-");
    PutString (Revision);
    PutChar (' ');
    ConsoleColourFG (Colours [ColTitleFG2]);
    if (HomeDirLen)
      {
        PutString ("Search \"");
        PutString (FindTargetName);
        PutChar ('\"');
        if (FindTargetContaining [0])
          {
            PutString (" containing \"");
            PutString (FindTargetContaining);
            PutChar ('\"');
          }
        if (FindSubset == fUniques)
          PutString (" Unique Items");
        if (FindSubset == fDuplicates)
          PutString (" Duplicate Items");
        PutString (" in ");
      }
    if (PathsIndex)
      {
        PutChar ('[');
        PutInt (PathsIndex, 0);
        PutString ("] ");
      }
    PutString (Paths [PathsIndex]);
    ConsoleLine (1, Colours [ColTitleFG1], Colours [ColTitleBG]);
    if (Setup.ListingColumns & ListingColumnsAttributes)
      ShowTitle ("Attributes ", Setup.SortMode == sAttr);
    if (Setup.ListingColumns & ListingColumnsOwners)
      ShowTitle ("Owner    Group   ", Setup.SortMode == sOwnerGroup);
    if (Setup.ListingColumns & ListingColumnsDateTime)
      ShowTitle ("Date           Time", Setup.SortMode == sDateTime);
    if (AnalyseDirectories)
      ShowTitle ("  Count", Setup.SortMode == sCount);
    if ((Setup.ListingColumns & ListingColumnsSize) || AnalyseDirectories)
      ShowTitle ("        Size", Setup.SortMode == sSize);
    StrCopy (Titl, " Filename");
    if (Setup.SortMode == sExt)
      StrConcat (Titl, " by Ext");
    else if (Setup.SortMode == sPath)
      StrConcat (Titl, " by Path");
    ShowTitle (Titl, (Setup.SortMode == sName) || (Setup.SortMode == sExt) || (Setup.SortMode == sPath));
    PutSeparator ();
    //ConsoleColourFG (ColourTitleFG2);
    //PutChar (' ');
    if (TaggedCountSum ())
      {
        PutChar ('[');
        PutDirCountSize (TaggedCount, TaggedSize);
        PutChar (']');
      }
    else
      PutDirCountSize (DirCount, DirSize);
    if (DiskSpace (Paths [PathsIndex], &Capacity, &Free))
      {
        PutSeparator ();
        PutString ("Free ");
        PutIntScaled (Free);
        PutString (" / ");
        PutIntScaled (Capacity);
      }
  }

void ShowDirectory (void)
  {
    int y, i;
    _DirEntry *d;
    //
    y = 2;
    i = 0;
    d = Dir;
    while (true)
      {
        if (y >= ConsoleSizeY - 2)
          break;
        if (i >= YOffset)
          ShowItem (d, y++, i == YSel);
        if (d)
          d = d->Next;
        i++;
      }
  }

int FindDirEntryName (char *Name)
  {
    int n;
    _DirEntry *d;
    //
    n = 0;
    d = Dir;
    while (true)
      {
        if (d == NULL)
          return 0;
        if (StrCompare (d->Name, Name) == 0)
          return n;
        d = d->Next;
        n++;
      }
  }

int CountDir (void)
  {
    int n;
    _DirEntry *d;
    //
    n = 0;
    d = Dir;
    while (d)
      {
        d = d->Next;
        n++;
      }
    return n;
  }

int FileLine (int File)
  {
    return (File + 2 - YOffset);
  }

int NumFilesPage (void)
  {
    return ConsoleSizeY - 4;
  }

bool SearchDir (char *Target, int *YSel, bool StepFirst)
  {
    int y;
    _DirEntry *d;
    //
    if (Dir)
      {
        y = *YSel;
        d = FindDirEntry (Dir, y);
        while (true)
          {
            // Step next
            if (StepFirst)
              {
                (y)++;
                d = d->Next;
                if (d == NULL)
                  {
                    d = Dir;
                    y = 0;
                  }
                // have we search all items in Dir
                if (y == *YSel)   // yes
                  break;
              }
            StepFirst = true;
            // Are we there yet
            if ((int) StrPos_ (d->Name, Target) != -1)   // found
              {
                *YSel = y;
                //ConsoleCursor(0,0); PutString(Target); PutChar(' '); PutInt(*YSel,0); putchar (' '); //####
                return true;
              }
          }
      }
    putchar (bel);
    Target [0] = 0;
    return false;
  }

void SortDirAnnounce (void)
  {
    ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Sorting ...");
    GetKey ();
    SortDir (&Dir, Setup.SortMode);
  }

void EditFileCommand (_DirEntry *Dir, bool Hex)
  {
    char *Command, *c;
    //
    DirShow = true;
    SelectDirectory (Dir);
    if (S_ISREG (Dir->Attrib))   // Is this a regular file?
      {
        if (Setup.ExternalEditor [0])   // External editor defined
          {
            Command = malloc (StrLength (Setup.ExternalEditor) + StrLength (Dir->Name) + 16);
            c = Command;
            StrToStr (&c, Setup.ExternalEditor);
            StrToStr (&c, " \"");
            StrToStr (&c, Dir->Name);
            CharToStr (&c, '\"');
            *c = 0;
            ExecuteCommand (Command);
            free (Command);
          }
        #ifdef IncludeEdit
        else
          {
            ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
            StrAssign (&DirItem, Dir->Name);
            if (EditFile (Dir->Name, HomeDirLen ? FindTargetContaining : NULL, Hex))
              {
                if (chmod (DirItem, Dir->Attrib) != 0)   // copy origial file's attributes
                  ConsoleBeep ();
                if (Setup.ExecuteReadDir)
                  DirRead = true;
                else
                  DirEntryFromFilename (DirItem, Dir);
                DirShow = true;
              }
            TitleShow = true;
          }
        #endif
      }
    chdir (Paths [PathsIndex]);
  }

const char* SetupFieldNames [] = {"Colour Theme [|^C|ustom]",
                                  //"Show Attributes",
                                  //"Show Owners",
                                  //"Show DateTime",
                                  //"Show Size",
                                  "Copy/sYnc: Tollerate regional time differences",
                                  "Copy: Ignore filenames with any of",
                                  "Search: Copy/Move: Include local path in Destination",
                                  //"Find Mode",
                                  "Action of the |Enter| key on a file",
                                  "Reread Directory after eXecute/Edit",
                                  //"Directory Size",
                                  "External Editor (optional)",
                                  NULL};

const char* ColourThemeNames [] = {"Black", "White", "Blue", NULL};

const char* EnterOpNames [] = {"Edit", "Open", NULL};
//const char* DirectorySizeContentsNames [] = {"Show system size", "Show Total Contents Size (recursive)", NULL};

//unsigned int ListingColumns_;
//bool DirectorySizeContents;
_Setup SetupCopy;

int SetupFieldEdit (int Field)
  {
    int c;
    //
    c = -1;
    switch (Field)
      {
        case 0: // Colour Theme
          c = EditEnum ((_enum *) &SetupCopy.ColourTheme, ColourThemeNames);
          if (SetupCopy.ColourTheme != Setup.ColourTheme)
            {
              ColoursCustom =  false;
              MemMove (Colours, ColourThemes [SetupCopy.ColourTheme], sizeof (Colours));
            }
          if (c == Cntrl ('C'))   // Custom
            {
              ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
              PutString ("Custom Colour Theme ");
              MemMove (ColoursCopy, Colours, sizeof (Colours));
              if (EditFields (ColoursFieldNames, ColoursFieldEdit, Colours [ColQueryFG2]))
                {
                  MemMove (Colours, ColoursCopy, sizeof (Colours));
                  ColoursCustom =  true;
                }
              ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
              c = 0;
            }
          break;
        //case 1: // Show Attributes
        //  c = EditBit (&ListingColumns_, ListingColumnsAttributes);
        //  break;
        //case 2: // Show Owners
        //  c = EditBit (&ListingColumns_, ListingColumnsOwners);
        //  break;
        //case 3: // Show DateTime
        //  c = EditBit (&ListingColumns_, ListingColumnsDateTime);
        //  break;
        //case 4: // Show Size
        //  c = EditBit (&ListingColumns_, ListingColumnsSize);
        //  break;
        case 1: // Copy/sYnc: Tollerate File DateTime differences of 1 hour
          c = EditBool (&SetupCopy.CopySyncTollerateHours);
          break;
        case 2: // Copy/Move: Don't copy files with
          c = EditString_ (SetupCopy.CopyIgnore, sizeof (SetupCopy.CopyIgnore));
          break;
        case 3: // Search: Copy/Move: Include local path in Destination
          c = EditBool (&SetupCopy.CopyMoveLocalPath);
          break;
        /*
        case 3: // Find: Allow wild chars
          c = EditEnum ((_enum *) &FindMode, FindModeNames);
          break;
        */
        case 4: // Enter: Select default operation
          c = EditEnum ((_enum *) &SetupCopy.EnterOp, EnterOpNames);
          break;
        case 5: // Reread Directory after eXecute
          c = EditBool (&SetupCopy.ExecuteReadDir);
          break;
        /*
        case 4: // Directory Size
          c = EditEnum ((_enum *) &DirectorySizeContents, DirectorySizeContentsNames);
          break;
        */
        case 6: // Just for Paul, an external editor
          c = EditString_ (SetupCopy.ExternalEditor, sizeof (SetupCopy.ExternalEditor));
          break;
        //default:
        //  c = esc;
      }
    return c;
  }

void SetupEdit (void)
  {
    SetupCopy = Setup;
    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
    PutString ("Setup ");
    if (EditFields (SetupFieldNames, SetupFieldEdit, Colours [ColQueryFG2]))
      Setup = SetupCopy;
    ColourThemeSet ();
    DirShow = true;
  }

bool ProcessEnterOpen (byte Ch)
  {
    _DirEntry *d;
    bool Open, Force;
    int i;
    char *Name;
    _Action Res;
    //
    Res = aYes;
    if ((Ch != KeyEnter) && (Ch != KeyAltEnter) && (Ch != Cntrl ('\\')))
      return false;
    Open = true;
    Force = false;
    if ((Ch == KeyEnter) && (Setup.EnterOp != eoOpen))
      Open = false;
    if (Ch != KeyEnter)
      Force = true;
    d = FindDirEntry (Dir, YSel);
    if (d)
      if (d->Directory)   // Go to directory
        {
          free (Paths [PathsIndex]);
          Paths [PathsIndex] = GetItemPath (d); //MakeFilePath (d->Path, d);
          if (chdir (Paths [PathsIndex]) != 0)   // can't change directory
            Res = aError;
          DirRead = true;
        }
      else if (d->SymLink)   // Symbolic Link
        {
          // does this contain a directory?
          i = StrPosLastCh (d->SymLinkTarget, PathDelimiter);
          if (i >= 0)   // Yes. So change directory
            {
              // Set Target file
              StrAssign (&DirItem, &d->SymLinkTarget [i + 1]);
              // Change directory
              d->SymLinkTarget [i] = 0;
              StrAssign (&Paths [PathsIndex], d->SymLinkTarget);
              if (chdir (Paths [PathsIndex]) != 0)   // can't change directory
                Res = aError;
              DirRead = true;
            }
          else   // no, just a file
            StrAssign (&DirItem, d->SymLinkTarget);
        }
      else   // it is a file
        {
          StrAssign (&DirItem, d->Name);
          if (HomeDirLen)   // Search Result so go there
            {
              StrAssign (&Paths [PathsIndex], d->Path);
              if (chdir (Paths [PathsIndex]) != 0)
                Res = aError;
              DirRead = true;
            }
          else   // open / Edit file
            if (Executable (d) && (Ch == KeyEnter))
              {
                Name = malloc (StrLength (d->Name) + 8);
                Name [0] = 0;
                StrConcat (Name, "\"");
                #ifndef _Windows
                StrConcat (Name, "./");
                #endif
                StrConcat (Name, d->Name);
                StrConcat (Name, "\"");
                ExecuteCommand (Name);
                free (Name);
              }
            else   // a non-executable, real file
              {
                StrAssign (&DirItem, d->Name);
                if (Open)
                  {
                    if (!ExecuteScript (d, Force))
                      EditFileCommand (d, false);
                  }
                else
                  EditFileCommand (d, false);
              }
        }
    CheckAction (d, &Res);
    return true;
  }

void ShowAbout (bool Full)
  {
    char IP [256], *IPx;
    //
    //ConsoleColourBG (ColBlack);
    //ConsoleColourFG (ColYellow);
    //PutNewLine ();
    //PutNewLine ();
    //ConsoleColourBG (ColGreenDark);
    About ((char *) AppName, Revision, "File Browser for Console");
    //ConsoleColourBG (ColBlack);
    //PutNewLine ();
    if (Full)
      {
        PutString ("Networks\n");
        IPx = IP;
        StrGetIPMAC (&IPx, IncludeName | IncludeIP | IncludeMAC | IncludeLF);
        *IPx = 0;
        PutString (IP);
        PutNewLine ();
        //PutCharN ('=', 64);
        PutNewLine ();
        //PutBoxStr (IP);
        //PutBoxStr ("<This is a line of crap>\n<Another Line>\n<Short>");
      }
    ConsoleColourFG (ColWhite);
    PutNewLine ();
  }

void CommandEdit (bool Hex)
  {
    _DirEntry *d;
    //
    d = FindDirEntry (Dir, YSel);
    if (!d->Directory)   // Not a directory
      {
        StrAssign (&DirItem, d->Name);
        EditFileCommand (d, Hex);
      }
  }

void CommandRename (void)
  {
    _DirEntry *d;
    _Action Res;
    char *p;
    int i;
    //
    d = FindDirEntry (Dir, YSel);
    if (d)
      {
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutString ("Examples: nnnnn.xxx  nnnnn.*  *.xxx  Thing###.*");
        PutNewLine ();
        PutString ("Rename ");
        PutTaggedDesc (d);
        PutString (" using ");
        p = (char *) malloc (256);
        StrCopyN (p, d->Name, 256);
        Res = aYes;
        if (EditString__ (p, 256) == KeyEnter)
          {
            if (TaggedCountSum () == 0)   // Single file / folder
              {
                if (!Rename (d, p, 1))
                  Res = aError;
                CheckAction (d, &Res);
              }
            else
              {
                d = Dir;
                i = 1;
                while (true)
                  {
                    if (d == NULL)
                      break;
                    if (d->Tagged)
                      if (!Rename (d, p, i++))
                        Res = aError;
                    CheckAction (d, &Res);
                    d = d->Next;
                  }
              }
            DirRead = true;
          }
        free (p);
      }
  }

void CommandCopyMoveDelete (byte c)
  {
    _DirEntry *d;
    _Action Action;
    //
    d = FindDirEntry (Dir, YSel);
    if (d)
      {
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutCommandName (c, tBase);
        PutTaggedDesc (d);
        Action = aAbort;
        if (c == CmdDelete)
          {
            PutString (": ");
            if (YesNo (Colours [ColQueryFG2]))
              Action = aYes;
          }
        else
          {
            PutString (" to -");
            ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
            if (SelectDestination (true))
              Action = aYes;
          }
        if (Action == aYes)
          {
            FilingStatsInit ();
            ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
            PutString (CommandName (c, true));
            PutString ("...");
            if (TaggedCountSum () == 0)   // Single file / folder
              CopyMoveDeleteItem (c, d, Paths [1], HomeDirLen);
            else
              {
                d = Dir;
                while (true)
                  {
                    if (d == NULL)
                      break;
                    if (d->Tagged)
                      {
                        Action = CopyMoveDeleteItem (c, d, Paths [1], HomeDirLen);
                        if (Action == aAbort)
                          break;
                      }
                    d = d->Next;
                  }
              }
            chdir (Paths [0]);
            DirRead = true;
            FilingStatsShow ();
          }
      }
  }

void CommandSync (void)
  {
    _DirEntry *d;
    _Action Action;
    bool OK;
    bool Pass;
    //
    d = FindDirEntry (Dir, YSel);
    if (d)
      {
        OK = true;
        if (TaggedCountSum () == 0)
          {
            if (!d->Directory)
              OK = false;
          }
        else if (TaggedCount [0])
          OK = false;
        if (OK)
          {
            Action = aYes;
            ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
            PutString ("Sync ");
            PutTaggedDesc (d);
            PutString (" into");
            ConsoleLine (ConsoleSizeY - 1, Colours [ColQueryFG1], Colours [ColQueryBG]);
            if (SelectDestination (true))
              {
                SyncFilesCopy = 0;
                SyncNewerOverwrite = 0;
                SyncDirectoriesDelete = 0;
                SyncFilesDelete = 0;
                LogWrite ("----Sync to ");
                LogWrite_ (Paths [1]);
                for (Pass = false; Pass <= true; Pass++)
                  {
                    ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
                    if (Pass)
                      {
                        if (SyncFilesCopy + SyncNewerOverwrite + SyncDirectoriesDelete + SyncFilesDelete == 0)
                          {
                            StrCopy (Message, "Nothing to do!");
                            Action = aAbort;
                          }
                        else
                          {
                            PutInt (SyncFilesCopy, 0);
                            PutString (" to Copy. ");
                            PutInt (SyncNewerOverwrite, 0);
                            PutString (" Newer files overwritten. ");
                            PutInt (SyncDirectoriesDelete, 0);
                            PutString (" Dirs to Delete. ");
                            PutInt (SyncFilesDelete, 0);
                            PutString (" Files to Delete");
                            PutNewLine ();
                            PutString ("Proceed ");
                            if (!YesNo (Colours [ColQueryFG2]))
                              Action = aAbort;
                          }
                      }
                    else
                      PutString ("Checking ...");
                    if (Action == aYes)
                      {
                        FilingStatsInit ();
                        if (TaggedCountSum () == 0)   // Single file / folder
                          Action = SyncDirectory (d, Paths [1], Pass);
                        else
                          {
                            d = Dir;
                            while (true)
                              {
                                if (d == NULL)
                                  break;
                                if (d->Tagged)
                                  {
                                    Action = SyncDirectory (d, Paths [1], Pass);
                                    CheckAction (d, &Action);
                                    if (Action != aYes)
                                      break;
                                  }
                                d = d->Next;
                              }
                          }
                        FilingStatsShow ();
                      }
                  }
                chdir (Paths [0]);
              }
            DirShow = true;
          }
        else   // a non directory was selected
          StrCopy (Message, "ERROR: Can only Sync Directories");
      }
  }

void CommandOrganize (void)
  {
    _DirEntry *d;
    _Action Action;
    int i;
    char *Name_;
    //
    d = FindDirEntry (Dir, YSel);
    if (d)
      {
        ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
        PutString ("organiZe ");
        PutTaggedDesc (d);
        Action = aYes;
        PutString (" into sub Directories using: ");
        PutNewLine ();
        Name_ = malloc (256);
        StrCopy (Name_, "X?YYYY?MM");
        while ((i = (int) StrPosCh (Name_, '?')) >= 0)
          Name_ [i] = PathDelimiter;
        if (EditString__ (Name_, 256) == KeyEnter)
          {
            if (TaggedCountSum () == 0)   // Single file / folder
              YearMonthDayExtItem (d, Name_);
            else
              {
                d = Dir;
                while (true)
                  {
                    if (d == NULL)
                      break;
                    if (d->Tagged)
                      {
                        Action = YearMonthDayExtItem (d, Name_);
                        if ((Action == aAbort) || (Action == aError))
                          break;
                      }
                    d = d->Next;
                  }
              }
            chdir (Paths [PathsIndex]);
            DirRead = true;
          }
        free (Name_);
      }
  }

bool Quit;
//
int YSelPrev;

bool ProcessCommand (int c, int cPrev)
  {
    static bool NewTagged = false;
    bool Res;
    _DirEntry *d;
    char *Name_;
    char *p;
    int i;
    int Toggle;
    //
    GetOptionDefault = 0;
    Res = true;
    p = NULL;
    if (c == KeyDel)
      c = CmdDelete;
    if (c == tab)   // Cycle thru bookmarks     //((cOld == tab) && (c != tab))
      {
        if ((cPrev == tab) || (PathsIndex == 0))
          PathsIndex = (PathsIndex + 1) % SIZEARRAY (Paths);
        else
          PathsIndex = 0;
        DirRead = true;
        if (chdir (Paths [PathsIndex]))   // directory does not exist
          ConsoleBeep ();
      }
    else   // Move current bookmark to top of list
      {
        StringArrayPromote (Paths, SIZEARRAY (Paths), PathsIndex);
        PathsIndex = 0;
        d = FindDirEntry (Dir, YSel);
        Toggle = -1;
        if (!ProcessEnterOpen (c))
          switch (c)
            {
              case KeyShiftUp:
                Toggle = YSel;
              case KeyUp:
                YSel--;
                break;
              //
              case KeyShiftDown:
                Toggle = YSel;
              case KeyDown:
                YSel++;
                break;
              //
              case KeyShiftHome:
                Toggle = YSel;
              case KeyHome:
                YSel = 0;
                break;
              //
              case KeyShiftEnd:
                Toggle = YSel;
              case KeyEnd:
                YSel = CountDir () - 1;
                break;
              //
              case KeyShiftPageDown:
              case KeyShiftRight:
                Toggle = YSel;
              case KeyPageDown:
              case KeyRight:
                if (YSel - YOffset < NumFilesPage () - 1)
                  YSel = YOffset + NumFilesPage () - 1;
                else
                  YSel = YSel + NumFilesPage () - 1;
                break;
              //
              case KeyShiftPageUp:
              case KeyShiftLeft:
                Toggle = YSel;
              case KeyPageUp:
              case KeyLeft:
                if (YSel > YOffset)
                  YSel = YOffset;
                else
                  YSel = YSel - NumFilesPage () + 1;
                break;
              //
              case Cntrl ('S'):
                while (true)
                  {
                    if (++Setup.SortMode >= sZZZZ)
                      Setup.SortMode = 0;
                    // repeat if not a valid Sort mode
                    if (Setup.SortMode == sDateTime)
                      if (~Setup.ListingColumns & ListingColumnsDateTime)
                        continue;
                    if (Setup.SortMode == sPath)
                      if (HomeDirLen == 0)
                        continue;
                    if (Setup.SortMode == sSize)
                      if (~Setup.ListingColumns & ListingColumnsSize)
                        continue;
                    if (Setup.SortMode == sCount)
                      if (!AnalyseDirectories)
                        continue;
                    if (Setup.SortMode == sOwnerGroup)
                      if (~Setup.ListingColumns & ListingColumnsOwners)
                        continue;
                    if (Setup.SortMode == sAttr)
                      if (~Setup.ListingColumns & ListingColumnsAttributes)
                        continue;
                    break;
                  }
                if (d)
                  StrAssign (&DirItem, d->Name);
                SortDirAnnounce ();
                DirShow = true;
                HelpShow = true;
                break;
              case esc: // Escape - Go up a level
                if (HomeDirLen == 0)
                  {
                    chdir ("..");
                    i = StrPosLastCh (Paths [PathsIndex], PathDelimiter);
                    if (i >= 0)
                      StrAssign (&DirItem, &Paths [PathsIndex] [i + 1]);
                  }
                DirRead = true;
                break;
              case Cntrl ('G'):
                ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
                PutString ("Location: ");
                Name_ = malloc (MaxPath);
                Name_ [0] = 0;
                if (EditString (Name_, MaxPath, MaxPath) == cr)
                  {
                    ExpandPath (&Name_);
                    if (chdir (Name_) == 0)
                      StringArrayAdd (Paths, SIZEARRAY (Paths), GetCurrentWorkingDirectory ());
                    else
                      ConsoleBeep ();
                  }
                free (Name_);
                DirRead = true;
                break;
              case ' ':
                Toggle = YSel;
                break;
              /*
              case Cntrl ('T'):
                //d = FindDirEntry (Dir, YSel);
                if (d)
                  {
                    t = !d->Tagged;
                    d = Dir;
                    i = 0;
                    while (d)
                      {
                        if (In (i, YSel, YSelPrev))
                          d->Tagged = t;
                        d = d->Next;
                        i++;
                      }
                    YSelPrev = YSel;
                    DirShow = true;
                  }
                break;
              */
              case Cntrl ('A'):
                d = Dir;
                while (d)
                  {
                    d->Tagged = (TaggedCountSum () == 0);
                    d = d->Next;
                  }
                DirShow = true;
                break;
              case CmdCopy:
              case CmdMove:
              case CmdDelete:
                CommandCopyMoveDelete (c);
                break;
              case Cntrl ('R'): // ^R or F2 => Rename
              case KeyF1+1:
                CommandRename ();
                break;
              //#ifndef _Windows
              case Cntrl ('P'): // Permissions
                if (Properties (d))
                  DirRead = true;
                break;
              //#endif // _Windows
              case Cntrl ('T'): // Time Offset
                if (CommandTimeOffset (d))
                  DirRead = true;
                break;
              case Cntrl ('E'): // Edit
              case Cntrl ('H'): // Edit Hex
                //d = FindDirEntry (Dir, YSel);
                if (d)
                  if (!d->Directory)   // Not a directory
                    {
                      StrAssign (&DirItem, d->Name);
                      EditFileCommand (d, c == Cntrl ('H'));
                    }
                break;
              case CmdNew: // New
                if (New ())
                  {
                    DirRead = true;
                    DoRename = true;
                  }
                break;
              case Cntrl ('X'): // Execute
                //d = FindDirEntry (Dir, YSel);
                Execute (d);
                break;
              case Cntrl ('F'): // Find
                d = Find ();
                if (d)
                  {
                    FreeDir (Dir);
                    Dir = d;
                    SortDirAnnounce ();
                    DirShow = true;
                    p = (char *) malloc (MaxPath + 1);
                    getcwd (p, MaxPath);
                    HomeDirLen = StrLength (p);
                    if (HomeDirLen > 1)
                    //if (p [HomeDirLen] == PathDelimiter)
                      HomeDirLen++;
                  }
                break;
              case Cntrl ('L'): // AnaLyse Directories
                AnalyseDirectories = !AnalyseDirectories;
                DirRead = true;
                break;
              case Cntrl ('Y'): // sYnc
                CommandSync ();
                break;
              case Cntrl ('B'): // Breakdown Summary
                ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
                if (HomeDirLen)   // results of search
                  {
                    PutString ("Breakdown Summary by ");
                    c = GetOption ("|E|xtension or |D|irectory", Colours [ColQueryFG2]);
                    if (c > 'A')
                      ShowSummary (Dir, c == 'D', HomeDirLen);
                  }
                else
                  ShowSummary (Dir, false, 0);
                DirShow = true;
                break;
              case Cntrl ('Z'): // organiZe
                CommandOrganize ();
                break;
              case Cntrl ('U'): // Setup
                SetupEdit ();
                break;
              #ifdef IncludeEdit
              case Cntrl ('V'): // View Log
                Name_ = LogName ();
                EditFile (Name_, NULL, false);
                free (Name_);
                DirShow = true;
                break;
              #endif
              case KeyF1+4: // F5 => Reload
                DirRead = true;
                break;
              default:
                Res = false;
            }
          YSel = Max (Min (CountDir () - 1, YSel), 0);   // Range check
          if (Toggle >= 0)
            {
              if (c != cPrev)
                if (YSel > Toggle)   // moving down
                  NewTagged = !d->Tagged;
                else   // moving up
                  {
                    d = FindDirEntry (Dir, YSel);
                    if (d)
                      NewTagged = !d->Tagged;
                  }
              d = Dir;
              i = 0;
              while (d)
                {
                  if (i >= Min (YSel, Toggle))
                    if (i < Max (YSel, Toggle)   // Within the range
                        || (i == YSel && !d->Next)   // Include the last item
                        || (i == YSel && YSel == Toggle))   // Single toggle
                      d->Tagged = NewTagged;
                  d = d->Next;
                  i++;
                }
              DirShow = true;
              //TitleShow = true;
            }
      }
    if (p)
      free (p);
    return Res;
  }

bool ProcessCommand_ (int c)
  {
    return ProcessCommand (c, 0);
  }


////////////////////////////////////////////////////////////////////////////
//
// Menu

char GetOption_ (char *Options)
  {
    ConsolePrompt (Colours [ColHelpFG1], Colours [ColHelpBG]);
    return GetOption (Options, Colours [ColHelpFG2]);
  }

void Menu (void)
  {
    char c;
    _DirEntry *d;
    bool t;
    //
    c = GetOption_ ("|F|ile |T|ag |M|anage |V|iew |S|etup |Q|uit");
    switch (c)
      {
        case 'F':  // File
          c = GetOption_ ("file> |O|pen |E|dit edit|H|ex e|X|ecute |N|ew |C|opy |M|ove |D|elete |R|ename |P|roperties |T|ime");
          switch (c)
            {
              case 'O': // Open
                ProcessEnterOpen (KeyAltEnter);
                break;
              case 'E': // Edit
                CommandEdit (false);
                break;
              case 'H': // edit Hex
                CommandEdit (true);
                break;
              case 'X': // eXecute
                d = FindDirEntry (Dir, YSel);
                Execute (d);
                break;
              case 'N': // New
                if (New ())
                  {
                    DirRead = true;
                    DoRename = true;
                  }
                break;
              case 'C': CommandCopyMoveDelete (CmdCopy); break;
              case 'M': CommandCopyMoveDelete (CmdMove); break;
              case 'D': CommandCopyMoveDelete (CmdDelete); break;
              case 'R': CommandRename (); break;   // Rename
              case 'P': ProcessCommand_ (Cntrl ('P')); break;   // Properties
              case 'T': ProcessCommand_ (Cntrl ('T')); break;   // Time Offset
            }
          break;
        //
        case 'T':  // Tag
          //c = GetOption_ ("tag> |A|ll |I|nvert |F|rom-here |T|o-here");
          //c = GetOption_ ("tag> |A|ll |I|nvert |T|o-here |E|nd");
          c = GetOption_ ("tag> |A|ll |I|nvert |E|nd");
          switch (c)
            {
              case 'A': ProcessCommand_ (Cntrl ('A')); break;   // All
              case 'I': // Invert
                d = Dir;
                while (d)
                  {
                    d->Tagged = !d->Tagged;
                    d = d->Next;
                  }
                //YSelPrev = 0;
                DirShow = true;
                break;
              //case 'T': ProcessCommand (Cntrl ('T')); break;   // from prev To here
              case 'E':
                d = FindDirEntry (Dir, YSel);
                if (d)
                  {
                    t = !d->Tagged;
                    while (d)
                      {
                        d->Tagged = t;
                        d = d->Next;
                      }
                    DirShow = true;
                  }
                break;
            }
          break;
        //
        case 'M': // Manage
          c = GetOption_ ("manage> |S|ync |O|rganize");
          switch (c)
            {
              case 'S': ProcessCommand_ (Cntrl ('Y')); break;   // Sync
              case 'O': ProcessCommand_ (Cntrl ('Z')); break;   // Organize
            }
          break;
        //
        case 'V': // View
          c = GetOption_ ("view> |S|ort |C|olumns |T|ree |B|reakdown |D|irSize |L|og");
          switch (c)
            {
              case 'S': // Sort
                //ProcessCommand (Cntrl ('S'), 0); break;
                c = GetOption_ ("view> Sort> |F|ilename |E|xt |P|ath |D|ateTime |S|ize");
                switch (c)
                  {
                    case 'F': Setup.SortMode = sName;      break;
                    case 'E': Setup.SortMode = sExt;       break;
                    case 'P': Setup.SortMode = sPath;      break;
                    case 'D': Setup.SortMode = sDateTime;  break;
                    case 'S': Setup.SortMode = sSize;      break;
                  }
                d = FindDirEntry (Dir, YSel);
                if (d)
                  StrAssign (&DirItem, d->Name);
                SortDirAnnounce ();
                DirShow = true;
                break;
              //case 'A': ProcessCommand (Cntrl ('A')); break;   // tag All
              case 'C': // columns
                c = GetOption_ ("view> columns> |A|ttributes |O|wners |D|ateTime |S|ize");
                switch (c)
                  {
                    case 'A': Setup.ListingColumns ^= ListingColumnsAttributes; break;
                    case 'O': Setup.ListingColumns ^= ListingColumnsOwners; break;
                    case 'D': Setup.ListingColumns ^= ListingColumnsDateTime; break;
                    case 'S': Setup.ListingColumns ^= ListingColumnsSize ; break;
                  }
                DirShow = true;
                break;
              //case 'F': ProcessCommand (Cntrl ('F')); break;   // Find Files
              case 'T': if (ShowTree (StrLength (Paths [PathsIndex])))
                          DirRead = true;
                        DirShow = true;
                        break;   // Tree
              case 'B': ProcessCommand_ (Cntrl ('B')); break;   // Breakdown
              case 'D': ProcessCommand_ (Cntrl ('L')); break;   // Analyse Directories
              case 'L': ProcessCommand_ (Cntrl ('V')); break;   // View Log
            }
          break;
        //
        case 'S': // Setup
          ProcessCommand_ (Cntrl ('U'));
          break;
        case 'Q': // Quit
          Quit = true;
          break;
      }
  }


////////////////////////////////////////////////////////////////////////////
//
// Config Save / Load

typedef enum {cgNone, cgCommands, cgTargets, cgDirs, cgParams, cgMacros, cgAlias, cgZZZZ} _ConfigGroup;
const char *ConfigGroupName [] =
  {
    "",
    "[Commands]",
    "[Targets]",
    "[Directories]",
    "[Params]",
    "[Macros]",
    "[Alias]"
  };

typedef enum {cpNone, cpSort, cpColour, cpColumns, cpCopySync, cpCopyIgnore, cpFindMode, cpEnterOp, cpExecuteReadDir, cpEditor, cpZZZZ} _ConfigParam;
const char *ConfigParamName [] =
  {
    "",
    "Sort=",
    "Colour=",
    "Columns=",
    "CopySync=",
    "CopyIgnore=",
    "FindMode=",
    "EnterOp=",
    "ExecuteReadDir=",
    "Editor="
  };

const char *ConfigFile = ".fbc";

bool TextFileOpenConfig (_TextFile *File, _FileOpenMode FileOpenMode)
  {
    bool Res;
    char *path, *p;
    //
    Res = false;
    path = malloc (MaxPath);
    p = path;
    StrPathConfig (&p, (char *) ConfigFile, (char *) AppName);
    if (FileOpenMode != foRead)
      MakeFilePath (path);
    TextFileInit (File);
    Res = TextFileOpen (File, path, FileOpenMode);
    #ifndef _Windows
    if (!Res && FileOpenMode != foRead)
      {
        p = path;
        StrPathConfig (&p, (char *) ConfigFile, NULL);
        Res = TextFileOpen (File, path, FileOpenMode);
      }
    #endif // _Windows
    free (path);
    return Res;
  }

int StrGetIntChecked (char **St, int Lo, int Hi)
  {
    int Res;
    //
    Res = StrGetInt (St);
    if (Res > Hi)
      Res = Hi;
    if (Res < Lo)
      Res = Lo;
    return Res;
  }

void ConfigLoad (void)
  {
    _TextFile File;
    char  *Line, *x;
    _ConfigGroup Group, GroupNew;
    _ConfigParam Param;
    unsigned int Size;
    int i, n;
    //
    StringArrayFree (Commands, SIZEARRAY (Commands));
    #ifdef IncludeEdit
    StringArrayFree (EditTargets, SIZEARRAY (EditTargets));
    #endif
    StringArrayFree (Paths, SIZEARRAY (Paths));
    StringArrayFree (ExtensionAlias, SIZEARRAY (ExtensionAlias));
    if (TextFileOpenConfig (&File, foRead))
      {
        Group = cgNone;
        n = 0;
        while (true)
          {
            Line = TextFileReadln (&File, false);
            if (Line == NULL)
              break;
            GroupNew = cgZZZZ;
            while (true)
              {
                GroupNew--;
                if (GroupNew == cgNone)
                  break;
                if ((int) StrPos_ (Line, (char *) ConfigGroupName [GroupNew]) >= 0)   // Group found
                  break;
              }
            if (GroupNew != cgNone)
              {
                Group = GroupNew;
                n = 0;
              }
            else switch (Group)
              {
                case cgNone:
                  break;
                case cgCommands:
                  if (n < SIZEARRAY (Commands))
                    StrAssign (&Commands [n++], Line);
                  break;
                #ifdef IncludeEdit
                case cgTargets:
                  if (n < SIZEARRAY (EditTargets))
                    StrAssign (&EditTargets [n++], Line);
                  break;
                #endif
                case cgDirs:
                  StringArrayAdd (Paths, SIZEARRAY (Paths), Line);
                  break;
                case cgParams:
                  Param = cpZZZZ;
                  while (true)
                    {
                      Param--;
                      if (Param == cpNone)
                        break;
                      if ((int) StrPos_ (Line, (char *) ConfigParamName [Param]) >= 0)   // Param found
                        break;
                    };
                  x = &Line [StrLength (ConfigParamName [Param])];
                  switch (Param)
                    {
                      case cpNone:
                        break;
                      case cpSort:
                        Setup.SortMode = (_SortMode) StrGetIntChecked (&x, 0, sZZZZ - 1);
                        break;
                      case cpColour:
                        Setup.ColourTheme = StrGetIntChecked (&x, 0, ctZZZZ - 1);
                        ColourThemeSet ();
                        if (*x)
                          {
                            ColoursCustom = true;
                            Param = 0;
                            while (true)
                              {
                                i = StrGetNum (&x);
                                if (i < 0 || i > ColFGMax)   // Missing / Corrupt colours
                                  {
                                    ColoursCustom = false;
                                    ColourThemeSet ();
                                    break;
                                  }
                                Colours [Param++] = i;
                                if (Param == ColZZZZ)
                                  break;
                              }
                          }
                        break;
                      case cpColumns:
                        Setup.ListingColumns = StrGetIntChecked (&x, 0, ListingColumnsMax);
                        break;
                      case cpCopySync:
                        Setup.CopySyncTollerateHours = StrGetIntChecked (&x, 0, true);
                        break;
                      case cpCopyIgnore:
                        StrCopy (Setup.CopyIgnore, x);
                        break;
                      case cpFindMode:
                        Setup.FindModeName = StrGetIntChecked (&x, 0, spmZZZZ - 1);
                        Setup.FindModeContents = StrGetIntChecked (&x, 0, spmZZZZ - 1);
                        break;
                      case cpEnterOp:
                        Setup.EnterOp = StrGetIntChecked (&x, 0, eoZZZZ - 1);
                        break;
                      case cpExecuteReadDir :
                        Setup.ExecuteReadDir = StrGetIntChecked (&x, 0, true);
                        break;
                      case cpEditor:
                        StrCopy (Setup.ExternalEditor, x);
                        break;
                    }
                  break;
                #ifdef IncludeEdit
                case cgMacros:
                  x = Line;
                  n = StrGetHex (&x);
                  if ((n >= 0) && (n < MacroNum))
                    if (*x++ == '=')
                      {
                        Size = StrGetHexAscii (&x, Macro, sizeof (Macro));
                        if ((Size != -1) && (Size != 0))
                          {
                            Macro [Size] = 0;
                            StrAssign ((char **) &Macros [n], Macro);
                          }
                      }
                  break;
                #endif
                case cgAlias:
                  StringArrayAdd (ExtensionAlias, SIZEARRAY (ExtensionAlias), Line);
                  break;
              }
          }
        TextFileClose (&File);
      }
  }

void ConfigSave (void)
  {
    _TextFile File;
    char  Line [256], *x;
    char *hex;
    unsigned int Size;
    int i;
    //
    if (TextFileOpenConfig (&File, foWrite))
      {
        TextFileWriteln (&File, (char *) ConfigGroupName [cgCommands]);
        StringArrayWrite (Commands, SIZEARRAY (Commands), File.ID, false);
        //
        #ifdef IncludeEdit
        TextFileWriteln (&File, (char *) ConfigGroupName [cgTargets]);
        StringArrayWrite (EditTargets, SIZEARRAY (EditTargets), File.ID, false);
        TextFileWriteln (&File, (char *) ConfigGroupName [cgMacros]);
        for (i = 0; i < MacroNum; i++)
          if (Macros [i])   // Macro defined
            {
              hex = DataToHexAscii (Macros [i], StrLength (Macros [i]), &Size, 0);
              x = Line;
              IntToHex (&x, i, 2 | IntToLengthZeros);
              CharToStr (&x, '=');
              StrToStr (&x, hex);
              *(--x) = 0;   // lose lf
              TextFileWriteln (&File, Line);
              if (hex)
                free (hex);
            }
        #endif
        //
        TextFileWriteln (&File, (char *) ConfigGroupName [cgDirs]);
        StringArrayWrite (Paths, SIZEARRAY (Paths), File.ID, true);
        //
        TextFileWriteln (&File, (char *) ConfigGroupName [cgAlias]);
        StringArrayWrite (ExtensionAlias, SIZEARRAY (ExtensionAlias), File.ID, true);
        //
        TextFileWriteln (&File, (char *) ConfigGroupName [cgParams]);
        StrCopy (Line, ConfigParamName [cpSort]);
        StrAppend (Line, Setup.SortMode + '0');
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpColour]);
        IntToStr (&x, Setup.ColourTheme);
        if (ColoursCustom)
          for (i = 0; i < ColZZZZ; i++)
            {
              CharToStr (&x, ' ');
              IntToStr (&x, Colours [i]);
            }
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpColumns]);
        IntToStr (&x, Setup.ListingColumns);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpCopySync]);
        IntToStr (&x, Setup.CopySyncTollerateHours);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpFindMode]);
        IntToStr (&x, Setup.FindModeName);
        CharToStr (&x, ' ');
        IntToStr (&x, Setup.FindModeContents);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpCopyIgnore]);
        StrToStr (&x, Setup.CopyIgnore);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpEnterOp]);
        IntToStr (&x, Setup.EnterOp);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpExecuteReadDir]);
        IntToStr (&x, Setup.ExecuteReadDir);
        *x = 0;
        TextFileWriteln (&File, Line);
        x = Line;
        StrToStr (&x, (char *) ConfigParamName [cpEditor]);
        StrToStr (&x, Setup.ExternalEditor);
        *x = 0;
        TextFileWriteln (&File, Line);
        //
        TextFileClose (&File);
      }
  }

#define SeekTargetSize 32
char SeekTarget [SeekTargetSize + 1];

int main (int argc, char *argv [])
  {
    int c, cprev;
    int YSelOld;
    int i;
    //bool f;
    //
    //SortDirDebug = true;
    DirStart = GetCurrentWorkingDirectory ();
    ConfigLoad ();
    //f = false;
    for (i = 1; i < argc; i++)
      if (argv [i][0] == '-')
        {
          if (StrSame_ (argv [i], "-debug"))
            Debug = true;
          else if (StrSame_ (argv [i], "-log"))
            Log = true;
          else if (StrSame_ (argv [i], "-nolog"))
            Log = false;
          else if (StrSame_ (argv [i], "-resize"))
            AllowResize = false;
          else if (IsDigit (argv [i][1]))
           Setup.ColourTheme = argv [i][1] - '0';
        }
      else
        {
          //f = true;
          if (chdir (argv [i]) == 0)   // valid directory
            DirStart = GetCurrentWorkingDirectory ();
        }
    StringArrayAdd (Paths, SIZEARRAY (Paths), DirStart);
    //
    ColourThemeSet ();
    if (Log)
      {
        LogWrite_ ("=================== ");
        LogClose ();
      }
    PathsIndex = 0;
    if (chdir (Paths [PathsIndex]))   // fail, specified dir is not OK
      StringArrayAdd (Paths, SIZEARRAY (Paths), GetCurrentWorkingDirectory ());  // so add current one
    ConsoleInit (true);
    Dir = NULL;
    DirRead = true;
    DoRename = false;
    SeekTarget [0] = 0;
    //DirItem = NULL;
    YSelOld = -1;
    while (true)
      {
        if (Quit)
          break;
        free (Paths [PathsIndex]);
        Paths [PathsIndex] = GetCurrentWorkingDirectory ();
        if (DirRead)
          {
            DirRead = false;
            ConsolePrompt (Colours [ColQueryFG1], Colours [ColQueryBG]);
            PutString ("Reading ");
            PutString (Paths [PathsIndex]);
            PutNewLine ();
            PutString ("...");
            FreeDir (Dir);
            Dir = NULL;
            if ((Setup.ListingColumns & ListingColumnsSize) && AnalyseDirectories)
              ReadDirSearch (&Dir, NULL, rdmAnalyse, NULL, 0);
            else
              ReadDirSearch (&Dir, NULL, rdmSingle, NULL, 0);
            if (Setup.SortMode == sPath)   // Sort by Path not allowed for 1 dir
              Setup.SortMode = sName;
            SortDirAnnounce ();
            DirShow = true;
            HomeDirLen = 0;
            YSel = 0;
          }
        if (DirItem)   // Item search required
          {
            YSel = FindDirEntryName (DirItem);
            free (DirItem);
            DirItem = NULL;
          }
        else if (DirItemIndex > 0)
          YSel = DirItemIndex;
        DirItemIndex = 0;
        if (Dir == NULL)
          YSel = 0;
        else if (FindDirEntry (Dir, YSel) == NULL)
          YSel = CountDir () - 1;
        if (YSel != YSelOld)
          {
            if (YSel < YOffset)
              {
                YOffset = YSel;
                DirShow = true;
              }
            if (YSel >= YOffset + NumFilesPage ())
              {
                YOffset = YSel - NumFilesPage () + 1;
                DirShow = true;
              }
            if (!DirShow)
              {
                if (YSelOld >= 0)
                  ShowItem (FindDirEntry (Dir, YSelOld), FileLine (YSelOld), false);
                ShowItem (FindDirEntry (Dir, YSel), FileLine (YSel), true);
              }
          }
        if (DirShow && !GetKeyBuffered ())
          {
            DirShow = false;
            ConsoleGetSize ();
            ShowDirectory ();
            TitleShow = true;
          }
        if (TitleShow)
          {
            ShowDirectoryTitle ();
            TitleShow = false;
          }
        DrawScrollBar (2, ConsoleSizeY - 3,
                       YOffset, YOffset + (ConsoleSizeY - 4), DirCount [0] + DirCount [1],
                       Colours [ColBodyFG], Colours [ColBodyBG]);
        if (DoRename)
          {
            CommandRename ();
            DoRename = false;
          }
        else
          {
            if (Message [0])
              {
                ConsolePrompt (Colours [ColHelpFG1], Colours [ColHelpBG]);
                PutStringHighlight (Message, Colours [ColHelpFG2]);
                Message [0] = 0;
              }
            else if (HelpShow)
              {
                ConsolePrompt (Colours [ColHelpFG1], Colours [ColHelpBG]);
                PutStringHighlight ("|Up/Dn..| +|Shift| or |SPACE| to tag  |Enter Esc Tab| |^A|ll |^F|ind |^G|o |^C|opy m|^O|ve |Del|", Colours [ColHelpFG2]);
                //PutStringHighlight ("|^ v PgeUp/Dn| use |Shift| to tag  |Enter Esc Tab SPACE| |^A|ll |^F|ind |^G|o |^C|opy m|^O|ve |Del|", ColourHelpFG1, ColourHelpFG2);
                PutNewLine ();
                PutStringHighlight ("|/|=Menu |^_|=Help |^Q|uit", Colours [ColHelpFG2]);
                HelpShow = false;
              }
            cprev = c;
            c = GetKeyWait (false);
            if (c == GetKeyWaitResizeOccured)
              DirShow = HelpShow = true;
            else
              {
                YSelOld = YSel;
                if (c == Cntrl ('Q'))   // Quit
                  {
                    ShowItem (FindDirEntry (Dir, YSel), FileLine (YSel), false);   // remove highlight
                    break;
                  }
                else if (c == '?')   // About
                  {
                    ConsoleClear (ColWhite, ColBlack | ColBright);
                    //ConsolePrompt (ColWhite, ColBlack | ColBright);
                    ShowAbout (true);
                    WaitKey ();
                    DirShow = true;
                  }
                else if (c == '/')   // Menu
                  Menu ();
                else if (c == Cntrl ('_'))   // ^_=Help
                  {
                    ShowHelpPageFile_ ("fbc.hlp");
                    DirShow = true;
                    HelpShow = true;
                  }
                else if (ProcessCommand (c, cprev))
                  SeekTarget [0] = 0;
                else
                  { // Search within current Directory
                    if (c == '=' || c == KeyF1+2)   // Seek next: '=' or F3
                      {
                        if (SeekTarget [0])
                          SearchDir (SeekTarget, &YSel, true);
                      }
                    else if ((c > ' ') && (c < 0x7F))
                      {
                        if (cprev == '=' || cprev == KeyF1+2)
                          SeekTarget [0] = 0;
                        if (StrLength (SeekTarget) < SeekTargetSize)
                          {
                            StrAppend (SeekTarget, c);
                            SearchDir (SeekTarget, &YSel, false);
                          }
                      }
                    else
                      ConsoleBeep ();
                  }
              }
          }
        LogClose ();
      }
    FreeDir (Dir);
    ConsoleUninit (false);
    ConsolePrompt (ColWhite, ColBlack | ColBright);
    //????PathsNormalize ();   // Move Paths [PathsIndex] to top
    ConfigSave ();
    StringArrayFree (Commands, SIZEARRAY (Commands));
    #ifdef IncludeEdit
    EditFree ();
    #endif
    StringArrayFree (Paths, SIZEARRAY (Paths));
    ShowAbout (false);
    free (DirStart);
    return 0;
  }
