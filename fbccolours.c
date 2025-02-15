//////////////////////////////////////////////////////////////////////////
//
// COLOUR THEMES
//

typedef enum
  {
    ColTitleFG1,
    ColTitleFG2,
    //ColTitleFG3,
    ColTitleBG,
    ColBodyFG,
    ColBodyFGDir,
    ColBodyFGExe,
    ColBodyBG,
    ColBodyBGSel,
    ColBodyBGTag,
    ColBodyBGSelTag,
    ColHelpFG1,
    ColHelpFG2,
    ColHelpBG,
    ColQueryFG1,
    ColQueryFG2,
    ColQueryBG,
    //ColProgressFG,
    //ColProgressBG,
    ColErrorFG1,
    ColErrorFG2,
    ColErrorBG,
    ColSyntaxHighlight,  // allow 8 different object highlight colours
    ColZZZZ = ColSyntaxHighlight + 8,
  } _Colour;

typedef byte _Colours [ColZZZZ];

_Colours Colours;

_Colours ColourThemes [ctZZZZ] =
  {
    {
      // ctBlack:
      ColBlack,   // ColourTitleFG1
      ColWhite,   // ColourTitleFG2
      //ColRed,   //ColourTitleFG3
      ColCyanDark,   // ColourTitleBG
      ColWhite,   // ColourBodyFG
      ColBlue,   // ColourBodyFGDir
      ColGreen,   // ColourBodyFGExe
      ColBlack,   // ColourBodyBG
      ColBlueDark,   // ColourBodyBGSel
      ColGreenDark,   // ColourBodyBGTag
      ColCyanDark,   // ColourBodyBGSelTag
      ColBlack,   // ColourHelpFG1
      ColMaroon,   // ColourHelpFG2
      ColGray,   // ColourHelpBG
      ColYellow,   // ColourQueryFG1
      ColWhite,   // ColourQueryFG2
      ColMaroon,   // ColourQueryBG
      //ColWhite,   // ColourProgressFG
      //ColGreenDark,   // ColourProgressBG
      ColWhite,   // ColourErrorFG1
      ColYellow,   // ColourErrorFG2
      ColMaroon,   // ColourErrorBG
      //
      ColGreen,
      ColCyan,
      ColPurple,
      ColRed,
      ColYellow | ColItalic,
      ColBlue | ColItalic  //ColBrown,
    },
    {
      // ctWhite:
      ColBlue,   // ColourTitleFG1
      ColWhite,   // ColourTitleFG2
      //ColRed,   // ColourTitleFG3
      ColBlueDark,   // ColourTitleBG
      ColBlack,   // ColourBodyFG
      ColBlue,   // ColourBodyFGDir
      ColGreenDark,   // ColourBodyFGExe
      ColWhite,   // ColourBodyBG
      ColGray,   // ColourBodyBGSel
      ColGreen,   // ColourBodyBGTag
      ColCyan,   // ColourBodyBGSelTag
      ColBlack,   // ColourHelpFG1
      ColMaroon,   // ColourHelpFG2
      ColYellow,   // ColourHelpBG
      ColYellow,   // ColourQueryFG1
      ColWhite,   // ColourQueryFG2
      ColGreenDark,   // ColourQueryBG
      //ColWhite,   // ColourProgressFG
      //ColGreenDark,   // ColourProgressBG
      ColWhite,   // ColourErrorFG1
      ColYellow,   // ColourErrorFG2
      ColMaroon,   // ColourErrorBG
      //
      ColGreenDark,
      ColCyanDark,
      ColPurple,
      ColMaroon,
      ColBlueDark | ColItalic,
      ColBlue | ColItalic  //ColBrown,
    },
    {
      // ctBlue:
      6, 0, 14, 15, 14, 10, 4, 12, 0, 8, 0, 1, 7, 11, 15, 1, /*15, 2,*/ 15, 11, 1, 10, 14, 13, 14, 43, 43
      /*
      ColBlack,   // ColourTitleFG1
      ColMaroon,   // ColourTitleFG2
      //ColBlueDark,   // ColourTitleFG3
      ColYellow,   // ColourTitleBG
      ColWhite,   // ColourBodyFG
      ColBlue,   // ColourBodyFGDir
      ColGreen,   // ColourBodyFGExe
      ColBlueDark,   // ColourBodyBG
      ColBlack,   // ColourBodyBGSel
      ColGray,   // ColourBodyBGTag
      ColBrown,   // ColourBodyBGSelTag
      ColBlack,   // ColourHelpFG1
      ColMaroon,   // ColourHelpFG2
      ColGray,   // ColourHelpBG
      ColYellow,   // ColourQueryFG1
      ColWhite,   // ColourQueryFG2
      ColMaroon,   // ColourQueryBG
      //ColWhite,   // ColourProgressFG
      //ColGreenDark,   // ColourProgressBG
      ColWhite,   // ColourErrorFG1
      ColYellow,   // ColourErrorFG2
      ColMaroon,   // ColourErrorBG
      //
      ColGreen,
      ColBlue,
      ColPurple,
      ColCyan,
      ColYellow | ColItalic, //ColBlue,
      ColYellow | ColItalic
      */
    }
  };

bool ColoursCustom;

// Editor: Syntax highlight colours: see edit.c

void ColourThemeSet (void)
  {
    if (!ColoursCustom)
      MemMove (Colours, ColourThemes [Setup.ColourTheme], sizeof (Colours));
  }

_Colours ColoursCopy;

const char *ColoursFieldNames [] =
  {
    "Title FG1",
    "Title FG2",
    //"Title FG3",
    "Title BG",
    "Body FG",
    "Body FG Dir",
    "Body FG Exe",
    "Body BG",
    "Body BG Sel",
    "Body BG Tag",
    "Body BG Sel+Tag",
    "Help FG1",
    "Help FG2",
    "Help BG",
    "Query FG1",
    "Query FG2",
    "Query BG",
    //"Progress FG",
    //"Progress BG",
    "Error FG1",
    "Error FG2",
    "Error BG",
    //
    "Syntax: Types",
    "Syntax: Reserved",
    "Syntax: Preprocess",
    "Syntax: Symbols",
    "Syntax: String",
    "Syntax: Comment",
    NULL
  };

const char *ColoursFGBG = "FFBFFFBBBBFFBFFBFFBFFFFFF";

int ColoursFieldEdit (int Field)
  {
    bool Fore;
    int iColBG;
    //
    Fore = (ColoursFGBG [Field] == 'F');
    if (Fore)
      {
        ConsoleColourFG (Colours [ColQueryFG1]);
        PutStringHighlight ("[|0|-|8| |B|old |I|talic |U|nderline] ", Colours [ColQueryFG2]);
        iColBG = StrPosChFrom ((char *) ColoursFGBG, Field, 'B');
        if (iColBG >= 0)
          ConsoleColourBG (ColoursCopy [iColBG]);
        else
          ConsoleColourBG (ColoursCopy [ColBodyBG]);
      }
    return EditColours (&ColoursCopy [Field], Fore);
  }

