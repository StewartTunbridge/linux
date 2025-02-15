////////////////////////////////////////////////////////////////////////////
//
// LOG / CONFIG SUPPORT
//

bool Log = true;//false;
int LogFile = -1;

void ExpandPath (char **Path)   // Replace dynamic string with ~ expanded
  {
    char *NewPath, *p;
    //
    if (*Path && **Path == '~')
      {
        NewPath = malloc (MaxPath);
        p = NewPath;
        StrPathHome (&p, NULL);
        if ((*Path) [1] != PathDelimiter)
          CharToStr (&p, PathDelimiter);
        StrToStr (&p, (*Path) + 1);
        *p = 0;
        StrAssign (Path, NewPath);
        free (NewPath);
      }
  }

/*
void fputs_ (char *St, ?FILE *file)
  {
    fputs (St, file);
    fputs ("\n", file);
  }

char *fgets_ (char *line, int size, ?FILE *file)
  {
    char *x;
    //
    x = fgets (line, size, file);
    if (x)
      StrTrim (line);
    return x;
  }
*/

char *LogName (void)   // Path to log filename. Caller must free.
  {
    char *Name, *n;
    //
    Name = malloc (MaxPath);
    n = Name;
    StrPathHome (&n, "fbc.log");
    return Name;
  }

bool LogOpen (void)
  {
    char *Name;
    //
    if (Log)
      if (LogFile < 0)
        {
          Name = LogName ();
          LogFile = FileOpen (Name, foAppend);
          free (Name);
        }
    return (LogFile >= 0);
  }

void LogClose (void)
  {
    if (LogFile >= 0)
      {
        close (LogFile);
        LogFile = -1;
      }
  }

void LogWrite (const char *St)
  {
    if (LogOpen ())
      write (LogFile, St, StrLength (St));
  }

void LogWrite_ (const char *St)
  {
    LogWrite (St);
    LogWrite ("\n");
  }


