////////////////////////////////////////////////////////////////////////////
//
// DEVICE MONITOR CONSOLE
// ======================
//
// Open a /dev item OR TCP port and
//   Display data received (hex)
//   Transmit characters typed
//   Transmit hex data blocks assigned to Numeric keys (see dmc.conf)
//
// Parameters:
//   tcpserver
//   tcpclient
//   [any /dev item]
//   -[any valid IP address]
//   -[any valiud Port number]
//   -[any valid baud rate]
//
//   exaqmple: dmc ttyUSB0 -9600
//
// dmc.conf - Configuration file. Lines of ASCII text. Each line can be -
//   any Parameter (above)
//   A Macro definition: [0-9 | A-Z] = {hex byte} OR "ASCII" OR a combination
//     eg S = "S09" 0d 0a
//
//
// HISTORY
// 08 May 2020 Tidy screen state enter / exit
// 11 May 2020 Project file command line parameter
// 06 Aug 2020 Add ASCII+HEX display. Bug fixes
//             Fix colours (subdue)
//             Add Log
// 15 Aug 2020 Add Triggered Macros: Config: <Trigger> > <Response>
//             Remove unwanted packets: Config: ! data
// 07 Oct 2020 Config files (.dm): Entries can span lines when ending in a '\'
// 07 Oct 2021 Add DeviceParity
//
// TODO
// LOG: Integrate with Console out, use common string (using ASCII)
//
////////////////////////////////////////////////////////////////////////////

const char Revision [] = "3.1";

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _Windows
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <utime.h>
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <unistd.h>
#endif
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "../Lib.c"
#include "../Console.c"
#include "../ConsoleLib.c"
#include "../About.c"


////////////////////////////////////////////////////////////////////////////
//
// LOG

bool Log = false;

_TextFile LogFile;

bool LogOpen (void)
  {
    char name [256], *n;
    //
    if (Log)
      if (!TextFileIsOpen (&LogFile))
        {
          n = name;
          StrToStr (&n, "dmc-");
          DateTimeToStrLocalize (&n, time (NULL), "%Y%m%d-%H%M");
          StrToStr (&n, ".log");
          *n = 0;
          TextFileOpen (&LogFile, name, foAppend);
        }
    return (TextFileIsOpen (&LogFile));
  }

void LogClose (void)
  {
    TextFileClose (&LogFile);
  }

//void LogWrite (const byte *Data, int Size)
//      if (fwrite (Data, 1, Size, LogFile) != Size)

void LogWrite (char *St, bool NewLine)
  {
    bool OK;
    //
    OK = false;
    if (LogOpen ())
      if (NewLine)
        OK = TextFileWriteln (&LogFile, St);
      else
        OK = TextFileWrite (&LogFile, St);
    if (!OK)
      LogClose ();
  }


////////////////////////////////////////////////////////////////////////////
//
// SUPPORT

void PutNewLine_ ()
  {
    //ConsoleClearEOL ();
    //while (ConsoleX < ConsoleSizeX)
    //  PutChar (' ');
    PutNewLine ();
  }

bool StrValidIP (char *IP)
  {
    if (StrGetNum (&IP) >= 0)
      if (*IP++ == '.')
        if (StrGetNum (&IP) >= 0)
          if (*IP++ == '.')
            if (StrGetNum (&IP) >= 0)
              return true;
    return false;
  }


////////////////////////////////////////////////////////////////////////////
//
// Key Variables

typedef enum {dDev, dTCPServer, dTCPClient} _DeviceType;

_DeviceType DeviceType = dDev;   // dDev => /dev/[device] else TCP socket
char DeviceName [128];
int DeviceFile = -1;
int DeviceSocket = -1;
int DeviceParam = -1;   // Baud rate or Port number
char DeviceParity = 'N';
char DeviceIP [128];
bool ASCII = false;

// Macros and Filters

typedef struct
  {
    int Size;
    byte *Data;
  } _Packet;

_Array Macros;
_Array Triggers;
_Array Responces;
_Array Filters;

int CycleTime = 1000;
//char CyclePattern [128] = {0};
int CycleState = -1;
int CycleTick;

void PacketFree (_Packet *Macro)
  {
    free (Macro->Data);
  }

int StrGetKey (char **p)
  {
    int Res;
    char *p_;
    //
    StepSpace (p);
    p_ = *p;
    Res = StrGetHex (&p_);
    if (p_ - *p == 2)   // 2 hex digits
      *p = p_;
    else if (**p == '^')   // control character
      {
        (*p)++;
        Res = **p & 0x1F;
        (*p)++;
      }
    else if (**p > ' ' && ~**p & 0x80)   // normal printable character
      Res = *(*p)++;
    else   // invalid
      Res = -1;
    return Res;
  }

void PutKeyName (byte c)
  {
    if (c < ' ')
      {
        PutChar (' ');
        PutChar ('^');
        PutChar (c + '@');
      }
    else if (c < 0x80)
      {
        PutChar ('\"');
        PutChar (c);
        PutChar ('\"');
      }
    else
      {
        PutChar (' ');
        PutHH (c);
      }
  }

bool ReadPacket (char **p, _Packet *Packet)
  {
    bool Res;
    int Size;
    char *p_;
    //
    Res = false;
    p_ = *p;
    Size = StrGetHexAscii (p, NULL, 0);
    if (Size >= 0)
      {
        Packet->Size = Size;
        Packet->Data = malloc (Size);
        *p = p_;
        StrGetHexAscii (p, Packet->Data, Size);
        Res = true;
      }
    return Res;
  }

bool FilterAdd (char *p)
  {
    _Packet *Filter;
    //
    Filter = malloc (sizeof (_Packet));
    ReadPacket (&p, Filter);
    ArrayAdd (&Filters, Filter);
    if (*p == 0)
      return true;
    return false;
  }

int SearchPacketArray (_Array *PacketArray, byte *Data, int DataSize)
  {
    int i, a, b;
    _Packet *Packet;
    //
    i = 0;
    while (i < PacketArray->Size)
      {
        a = b = 0;
        Packet = ArrayGet (PacketArray, i);
        if (Packet)
          while (true)
            {
              if (a == Packet->Size)   // Match
                return i;
              if (b + Packet->Size > DataSize)  // no match
                break;
              if (Packet->Data [a] == Data [b + a])   // matching so far
                a++;
              else   // not matching
                {
                  b++;
                  a = 0;
                }
            }
        i++;
      }
    return -1;
  }

bool FilterInData (byte *Data, int DataSize)
  {
    return SearchPacketArray (&Filters, Data, DataSize) >= 0;
  }


////////////////////////////////////////////////////////////////////////////
//
// Device drivers

char *DevDirectory = "/dev/";

bool DeviceInit (void)
  {
#ifndef _Windows
    char Name [256];
    struct termios options;
    int BaudFlag;
    int Tick;
    struct sockaddr_in serv_addr;
    const int True = 1;
    //
    if (DeviceType == dDev)
      {
        if (DeviceParam < 0)
          BaudFlag = B9600;
        switch (DeviceParam)
          {
            case    110: BaudFlag = B110; break;
            case    300: BaudFlag = B300; break;
            case    600: BaudFlag = B600; break;
            case   1200: BaudFlag = B1200; break;
            case   2400: BaudFlag = B2400; break;
            case   4800: BaudFlag = B4800; break;
            case   9600: BaudFlag = B9600; break;
            case  19200: BaudFlag = B19200; break;
            case  38400: BaudFlag = B38400; break;
            case  57600: BaudFlag = B57600; break;
            case 115200: BaudFlag = B115200; break;
          }
        strcpy (Name, DevDirectory);
        strcat (Name, DeviceName);
        //DeviceFile = open (Name, O_RDWR | O_NOCTTY | O_SYNC);// | O_NDELAY);   // "/dev/ttyUSB0"
        DeviceFile = open (Name, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);   // "/dev/ttyUSB0"
        //PutNewLine ();
        //PutString ("open: ");
        //PutInt (DeviceFile, 0);
        if (DeviceFile >= 0)
          {
            if (tcgetattr (DeviceFile, &options) == 0)
              {
                cfmakeraw (&options);
                options.c_cflag |= CREAD | CLOCAL;   // Enable receiving / transmitting
                cfsetispeed (&options, BaudFlag);   // Set Baud rate
                cfsetospeed (&options, BaudFlag);
                options.c_cflag &= ~CRTSCTS;   // No harware control
                options.c_iflag &= ~(IXON | IXOFF | IXANY);   // No flow control (XON/XOFF)
                options.c_cflag |= PARENB;   // Assume Parity ...
                options.c_cflag &= ~CMSPAR;   //   ... no Sticky Parity
                options.c_iflag |= INPCK | PARMRK;   // ... with checks
                if (DeviceParity == 'E')   // Make Even Parity
                  options.c_cflag &= ~PARODD;
                else if (DeviceParity == 'O')   // Make Odd Parity
                  options.c_cflag |= PARODD;
                else
                  {
                    options.c_cflag &= ~(PARENB);   // Make No Parity
                    options.c_iflag &= ~(INPCK);
                  }
                options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;   // 8 data bits
                /*
                cfsetispeed (&options, BaudFlag);   // Set Baud rate
                cfsetospeed (&options, BaudFlag);
                options.c_iflag &= ~INPCK;   // No parity
                options.c_cflag &= ~PARENB;
                options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;   // 8 data bits
                cfmakeraw (&options);
                */
                if (tcsetattr (DeviceFile, TCSANOW, &options))
                  PutString ("Some parameters not available");
              }
            return true;
          }
      }
    else if (DeviceType == dTCPServer)
      {
        if (DeviceParam < 0)
          return false;
        DeviceSocket = socket (AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (DeviceSocket >= 0)
          {
            setsockopt (DeviceSocket, SOL_SOCKET, SO_REUSEADDR, &True, sizeof (True));   // Make Port available on closing
            memset (&serv_addr, '0', sizeof (serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
            serv_addr.sin_port = htons (DeviceParam);
            bind (DeviceSocket, (struct sockaddr*) &serv_addr, sizeof (serv_addr));
            listen (DeviceSocket, 10);
            return true;
        }
      }
    else if (DeviceType == dTCPClient)
      {
        if (DeviceParam < 0)
          return false;
        DeviceSocket = socket (AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (DeviceSocket >= 0)
          {
            memset (&serv_addr, '0', sizeof (serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons (DeviceParam);
            Tick = ClockMS ();
            if (inet_pton (AF_INET, DeviceIP, &serv_addr.sin_addr) >= 0)
              while (true)
                {
                  if (connect (DeviceSocket, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) >= 0)
                    {
                      DeviceFile = DeviceSocket;
                      fcntl (DeviceSocket, F_SETFL, O_NONBLOCK);
                      return true;
                    }
                  usleep (10000);   // wait 10ms
                  if (ClockMS () - Tick > 3000)   // give up in 3 seconds
                    break;
                }
              //else
              //  puts ("connect fail");
          }
      }
#endif
    return false;
  }

bool DeviceUninit (void)
  {
#ifdef _Windows
#else
    if (DeviceFile)
      return (close (DeviceFile) == 0);
#endif
    return false;
  }

bool DeviceCheck ()
  {
#ifndef _Windows
    int df;
    int flags;
    byte col;
    //
    if (DeviceType == dTCPServer)
      {
        df = accept (DeviceSocket, (struct sockaddr*) NULL, NULL);
        if (df >= 0)   // New TCP Connection
          {
            if (DeviceFile >= 0)   // Old connection must go
              close (DeviceFile);
            DeviceFile = df;
            if (flags = fcntl (DeviceFile, F_GETFL, 0) >= 0)
              if (fcntl (DeviceFile, F_SETFL, flags | O_NONBLOCK) >= 0)
                {
                  col = ConsoleFG;
                  ConsoleColourFG (ColGreen | ColBright);
                  PutString ("New Connection");
                  PutNewLine_ ();
                  ConsoleColourFG (col);
                  return true;
                }
          }
      }
#endif // _Windows
    return false;
  }

void DeviceFlush (void)
  {
    byte *buffer;
    //
    buffer = malloc (4096);
    if (DeviceFile >= 0)
      while (true)
        if (read (DeviceFile, buffer, 4096) == 0)
          break;
    free (buffer);
  }


////////////////////////////////////////////////////////////////////////////
//
// CONFIGURATION

//char *TableBaud [] = {"1200", "2400", "9600", "19200", ""};
//char *TableParity;

//int TableBaudValue [] = {1200, 2400, 9600, 19200};

int StrSearchTable (char *Table [], char *St)
  {
    int i;
    //
    i = 0;
    while (true)
      {
        if (Table [i][0] == 0)   // End of table
          return -1;
        if (StrCompareCase (Table [i], St, false) == 0)   // Found
          return i;
        i++;
      }
  }

void SetDeviceType (void)
  {
    if ((StrCompareCase (DeviceName, "tcpserver", false) == 0) || (StrCompareCase (DeviceName, "server", false) == 0))
      {
        DeviceType = dTCPServer;
        strcpy (DeviceName, "TCP Server");
      }
    else if ((StrCompareCase (DeviceName, "tcpclient", false) == 0) || (StrCompareCase (DeviceName, "client", false) == 0))
      {
        DeviceType = dTCPClient;
        strcpy (DeviceName, "TCP Client");
      }
    else
      DeviceType = dDev;
  }

bool ParamComms (char *St)
  {
    int i;
    char c;
    //
    c = UpCase (*St);
    if (c == 'N' || c == 'O' || c == 'E')
      {
        DeviceParity = c;
        return true;
      }
    i = StrGetNum (&St);
    if (*St)   // did not make it to the end of the parameter (ie not a number)
      return false;
    // Baud rate or Port number
    DeviceParam = i;
    return true;
  }

bool ParamIP (char *St)
  {
    if (!StrValidIP (St))
      return false;
    strcpy (DeviceIP, St);
    return true;
  }

bool ParamOther (char *St)
  {
    int i;
    //
    if (StrCompareCase (St, "ASCII", false) == 0)
      {
        ASCII = true;
        return true;
      }
    if (StrCompareCase (St, "Log", false) == 0)
      {
        Log = true;
        return true;
      }
    if (UpCase (*St) == 'T')
      {
        St++;
        i = StrGetNum (&St);
        if (i > 0 && *St == 0)
          {
            CycleTime = i;
            return true;
          }
      }
    return false;
  }

bool Param (char *St)
  {
    //PutString ("Param: \"");
    //PutString (St);
    //puts ("\"");
    if (!ParamIP (St))
      if (!ParamComms (St))
        if (!ParamOther (St))
          return false;
    return true;
  }

bool ReadConfig (char *Filename)   // Read Config File (Device descriptors and Macros)
  {
    _TextFile f;
    char *Line;
    char *p;
    int n;
    bool Err, OK;
    _Packet *Macro, *Trigger;
    int Col;
    //
    //ConsoleColourFG (ColYellow);
    //ConsoleColourBG (ColBrown);
    OK = true;
    if (TextFileOpen (&f, Filename, false))
      {
        PutString ("Reading ");
        PutString (Filename);
        PutNewLine_ ();
        while ((Line = TextFileReadln (&f, true)) != NULL)
          {
            StrTrim (Line);
            Err = false;
            p = &Line [0];
            StepSpace (&p);
            if ((*p) && (*p != '#') && (*p != ';'))
              if ((int) StrPosCh (Line, '=') > 0)   // Macro definition
                {
                  n = StrGetKey (&p);
                  StepSpace (&p);
                  if (*p++ != '=' || n < 0)
                    Err = true;
                  else
                    {
                      Macro = malloc (sizeof (_Packet));
                      if (ReadPacket (&p, Macro))
                        ArraySet (&Macros, n, Macro);
                      else
                        {
                          Err = true;
                          free (Macro);
                        }
                      if (*p)
                        Err = true;
                    }
                }
              else if ((int) StrPosCh (Line, '>') > 0)   // Triggered Macro definition
                {
                  Trigger = malloc (sizeof (_Packet));
                  ReadPacket (&p, Trigger);
                  StepSpace (&p);
                  if (*p++ != '>')
                    Err = true;
                  Macro = malloc (sizeof (_Packet));
                  ReadPacket (&p, Macro);
                  if (*p)
                    Err = true;
                  ArrayAdd (&Triggers, Trigger);
                  ArrayAdd (&Responces, Macro);
                }
              else if (*p == '-')   // Parameter
                Err = !Param (++p);
              else if (*p == '!')   // Filter these packets
                Err = !FilterAdd (++p);
              /*else if (*p == '\\')
                {
                  p++;
                  CycleTime = StrGetNum (&p);
                  while (true)
                    {
                      StepSpace (&p);
                      if (!*p)
                        break;
                      n = StrGetKey (&p);
                      if (n < 0)
                        Err = true;
                      else
                        StrAppend (CyclePattern, n);
                    }
                }*/
              else if (IsAlpha (*p))   // Device
                {
                  strcpy (DeviceName, p);
                  SetDeviceType ();
                }
              else
                Err = true;
            if (Err)
              {
                OK = false;
                Col = ConsoleFG;
                ConsoleColourFG (ColRed);
                PutString ("Error: ");
                //PutString (Filename);
                //PutString (": ");
                PutString (Line);
                ConsoleColourFG (Col);
                PutNewLine_ ();
              }
          }
        TextFileClose (&f);
        for (n = 0; n < Macros.Size; n++)
          if ((Macro = ArrayGet (&Macros, n)))
            {
              PutString ("Macro on ");
              PutKeyName (n);
              PutString (": ");
              PutDataHex (Macro->Data, Macro->Size, ASCII);
            }
        for (n = 0; n < Triggers.Size; n++)
          if ((Trigger = ArrayGet (&Triggers, n)))
            {
              PutString ("Trigger: ");
              PutDataHex (Trigger->Data, Trigger->Size, ASCII);
              if ((Macro = ArrayGet (&Responces, n)))
                {
                  PutString ("  Reply: ");
                  PutDataHex (Macro->Data, Macro->Size, ASCII);
                }
            }
        for (n = 0; n < Filters.Size; n++)
          if ((Macro = ArrayGet (&Filters, n)))
            {
              PutString ("Filter: ");
              PutDataHex (Macro->Data, Macro->Size, ASCII);
            }
      }
    else
      {
        PutString ("Config File Not found");
        PutNewLine_ ();
      }
    PutNewLine_ ();
    return OK;
  }


////////////////////////////////////////////////////////////////////////////
//
// DISPLAY

void PutLn (char *St)
  {
    //PutStringHighlight (St, ColBlack, ColRed);
    PutString (St);
    PutNewLine_ ();
  }

void ShowHelp (void)
  {
    //PutLn ("dmc - Device Monitor for Console");
    //PutLn ("");
    PutLn ("dmc { Device | TCPServer | TCPClient | -Parameter | Filename }");
    PutLn ("");
    PutLn ("  Device: as in /dev eg ttyUSB0");
    PutLn ("  Parameters: (preceded with a hyphen)");
    PutLn ("    IP: eg -192.168.0.3");
    PutLn ("    Port: eg -5000");
    PutLn ("    Baud Rate: eg -9600");
    PutLn ("    Parity: eg -o  -e");
    PutLn ("    ASCII: show data in ASCII if possible. -ASCII");
    PutLn ("    Gap Time (in mS): eg -T1000");
    PutLn ("  Filename (*.dm): Project file specifying Parameters and/or the following. One item per line");
    PutLn ("    ! Filters: Discard Packets containing specified Data Block (in hex / ASCII)");
    PutLn ("    = Macros: Data block assigned to a typed key");
    PutLn ("    > Triggered Macros: Data block sent after receiving a Packet containing a Trigger");
    //PutLn ("    \\ <GapTime-mS> Play through following Macros: eg \\ 100 ABC");
    PutLn ("    Project example: USB RS232 port #0 running at 1200 baud with Filters and Macros:");
    PutLn ("      ttyUSB0");
    PutLn ("      -1200");
    PutLn ("      ! FF 00");
    PutLn ("      1 = 00 11 22 33 44 \"Ascii Part\" 0d");
    PutLn ("      X = 00 02 \"TEST\" 04");
    PutLn ("      \"Fred\" > \"Hello Fred\"");
    PutLn ("");
    PutLn ("Examples:");
    PutLn ("  dmc ttyUSB0 -9600");
    PutLn ("  dmc TCPClient -192.168.0.4 -5000");
    PutLn ("");
  }

byte DataIn [0x10000];
int DataInLen;
int DataInTime;

bool DataRead (void)
  {
    int Len;
    //
    if (DeviceFile)
      {
        Len = read (DeviceFile, &DataIn [DataInLen], sizeof (DataIn) - DataInLen);
        if (Len > 0)
          {
            DataInLen += Len;
            DataInTime = ClockMS ();
          }
        if (DataInLen > 0)
          if ((ClockMS () - DataInTime >= 50) || (DataInLen == sizeof (DataIn)))
            return true;
      }
    return false;
  }

int t0;

void PutDataHexTime (byte *Data, int DataLen, int Colour, bool Write)
  {
    int t;
    char *st, *s;
    int stl;
    //
    t = ClockMS ();
    st = malloc (64);
    s = st;
    IntToStrFill (&s, (t - t0) / 1000, 6 | IntToLengthZeros);
    CharToStr (&s, '.');
    IntToStrFill (&s, (t - t0) % 1000, 3 | IntToLengthZeros);
    if (Write)
      CharToStr (&s, '*');
    else
      CharToStr (&s, ' ');
    CharToStr (&s, ' ');
    *s = 0;
    ConsoleColourFG (Colour);
    PutString (st);
    LogWrite (st, false);
    free (st);
    if (ASCII)
      st = DataToHexAscii (Data, DataLen, &stl, ConsoleSizeX - ConsoleX);
    else
      st = DataToHex (Data, DataLen, &stl, ConsoleSizeX - ConsoleX);
    StrIndent (&st, 12);
    PutString (st);
    LogWrite (st, false);
    free (st);
  }

void DataWrite (byte *Data, int Size)
  {
    write (DeviceFile, Data, Size);
    PutDataHexTime (Data, Size, ColBlueDark, true);
  }

int main (int argc, char *argv [])
  {
    //int Len;
    int i;
    int c;
    bool Err;
    _Packet *Macro;
    //
    ConsoleInit (false);
    ConsoleColourFG (ColWhite);
    ConsoleColourBG (ColBlack);
    //ConsoleCursor (0, ConsoleSizeY - 1);
    //ConsoleClearEOL ();
    About ("dmc", Revision, "Device Monitor for Console");
    PutNewLine_ ();
    Err = false;
    for (i = 1; i < argc; i ++)
      if ((int) (StrPos_ (argv [i], ".dm")) >= 0)
        ReadConfig (argv [i]);
      else if (argv [i][0] != '-')
        {
          strcpy (DeviceName, argv [i]);
          SetDeviceType ();
        }
      else
        if (!Param (&argv [i][1]))
          {
            Err = true;
            PutString ("Invalid Parameter: ");
            PutString (argv [i]);
            PutNewLine_ ();
          }
    if ((DeviceName [0] == 0) || Err)
      ShowHelp ();
    else
      {
        ConsoleColourFG (ColYellow);
        //ConsoleColourBG (ColBlack);
        //PutNewLine_ ();
        //PutString ("dmc [");
        //PutString ((char *) Revision);
        PutString ("Monitoring ");
        PutString (DeviceName);
        //PutString (" @ ");
        PutChar (' ');
        if (DeviceType == dTCPClient)
          {
            PutChar ('\"');
            PutString (DeviceIP);
            PutChar ('\"');
          }
        if (DeviceParam > 0)
          {
            PutChar (' ');
            PutInt (DeviceParam, 0);
          }
        if (DeviceParity == 'O')
          PutString (" Odd");
        else if (DeviceParity == 'E')
          PutString (" Even");
        PutString ("   'End' to exit. 'Home' to cycle thru all Macros.  'Del' to flush.");
        PutNewLine_ ();
        if (DeviceInit ())
          {
            PutNewLine_ ();
            //ConsoleColourFG (ColYellow);
            //ConsoleColourBG (ColBlack);
            t0 = ClockMS ();
            CycleTick = ClockMS ();
            while (true)
              {
                DeviceCheck ();
                if (DeviceFile >= 0)
                  if (DataRead ())
                    {
                      if (!FilterInData (DataIn, DataInLen))
                        {
                          PutDataHexTime (DataIn, DataInLen, ColBlue, false);
                          i = SearchPacketArray (&Triggers, DataIn, DataInLen);
                          if (i >= 0)   // Triggered Macro triggered
                            if (Macro = ArrayGet (&Responces, i))
                              DataWrite (Macro->Data, Macro->Size);
                        }
                      DataInLen = 0;
                    }
                ConsoleGetSize ();
                c = GetKey ();
                if (c >= 0)
                  if (c == KeyEnd) // was Cntrl ('C'))
                    break;
                  else if (c == KeyHome)
                    {
                      CycleState = CycleState >= 0 ? -1 : 0;
                      CycleTick = ClockMS ();
                    }
                  else if (Macro = ArrayGet (&Macros, c))
                    DataWrite (Macro->Data, Macro->Size);
                  else if (c == KeyDel)
                    DeviceFlush ();
                  else
                    DataWrite ((byte *) &c, 1);
                if (CycleState >= 0)
                  if ((int) (ClockMS () - CycleTick) >= 0)
                    {
                      CycleTick += CycleTime;
                      c = CycleState;
                      while (true)
                        {
                          CycleState = (CycleState + 1) & 0xFF;
                          if (Macro = ArrayGet (&Macros, CycleState))
                            {
                              DataWrite (Macro->Data, Macro->Size);
                              break;
                            }
                          if (CycleState == c)
                            break;
                        }
                    }
#ifdef _Windows
                Sleep (10);
#else
                usleep (1000);
#endif
              }
            DeviceUninit ();
          }
        else
          {
            PutNewLine_ ();
            PutString ("ERROR: Can not open device ");
            PutString (DeviceName);
            PutString(" (");
            PutInt (errno, 0);
            PutString(" - ");
            PutString (strerror (errno));
            PutChar (')');
            PutNewLine_ ();
          }
      }
    ConsoleUninit (false);
#ifdef _Windows
    GetKeyWait ();
#endif
    ArrayFree (&Macros, (_ArrayFreeElement *) PacketFree);
    ArrayFree (&Triggers, (_ArrayFreeElement *) PacketFree);
    ArrayFree (&Responces, (_ArrayFreeElement *) PacketFree);
    ArrayFree (&Filters, (_ArrayFreeElement *) PacketFree);
    LogClose ();
    return 0;
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*
          for (i = VINTR; i <= VEOL2; i++)
            options.c_cc [i] = 0;
          options.c_lflag &= ~(ICANON | ECHO | IXON | IXOFF);
    /// *
          options.c_cflag |= (CLOCAL | CREAD);
          options.c_cflag &= ~PARENB;
          options.c_cflag &= ~CSTOPB;
          options.c_cflag &= ~CSIZE;
          options.c_cflag |= CS8;
          //options.c_cflag &= ~( ICANON | ECHO | ECHOE |ISIG );
          options.c_iflag &= ~(IXON | IXOFF | IXANY );
          options.c_oflag &= ~OPOST;
    //*/

              //printf ("Com = %i\n", Com);
              //l = ioctl (Com, 0);
              //printf ("ioctl: %i\n", l);
              //strcpy (x, "TEST\n");
              //write (Com, x, strlen (x));
              //enterInputMode ();

/*
                if (IsDigit (*p))   // Macro: n = Macro(Hex And Or Ascii) eg 1 = 00 01 AB "ABC" 0d
                  {
                    n = StrGetNum (&p);
                    StepSpace (&p);
                    if ((n < 0) || (n >= MacroSize))
                      Err = true;
                    else
                      if (*p++ != '=')
                        Err = true;
                      else
                        {
                          Macros [n].Size = StrGetHexAscii (&p, Macros [n].Data, MacroDataSize);
                          if (Macros [n].Size < 0)
                            Err = true;
                        }
                  }
*/

                        //if (c >= 0x80)   // About
                        //  {
                        //    ConsoleColourFG (ColCyan);
                        //    ConsoleColourBG (ColCyanDark);
                        //    About ("dmc", Revision, "Device Monitor for Console");
                        //    ConsoleColourBG (ColBlueDark);
                        //    PutNewLine_ ();
                        //  }
                        //else


/*
          {
            PutNewLine ();
            PutString ("open fail: ");
            PutInt (errno, 0);
          }
        else
*/

/*
int MacroIndex (char c)
  {
    if (c >= '0')
      if (c - '0' < MacroSize)
        if (Macros [c - '0'].Size > 0)
          return c - '0';
    return -1;
  }

bool IsMacro (char c)
  {
    int i = MacroIndex (c);
    if (i >= 0)
      if (Macros [i].Size > 0)
        return true;
    return false;
  }


            n = strlen (Line);
            while (true)
              {
                if (n == 0)
                  break;
                if (Line [n - 1] > ' ')
                  break;
                n--;
                Line [n] = 0;
              }
*/

              /*else if ((int) StrPos (Line, ">>"') > 0)   // Triggered Macro definition
                {
                  Trigger = malloc (sizeof (_Packet));
                  ReadPacket (&p, Trigger);
                  StepSpace (&p);
                  if (*p++ != '>')
                    Err = true;
                  if (*p++ != '>')
                    Err = true;
                  Macro = malloc (sizeof (_Packet));
                  ReadPacket (&p, Macro);
                  if (*p)
                    Err = true;
                  ArrayAdd (&Triggers, Trigger);
                  ArrayAdd (&Responces, Macro);
                }*/
