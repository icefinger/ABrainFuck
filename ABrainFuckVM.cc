#include <unistd.h>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using namespace std;


class Interpretor
{

  const string signlist[256]=
    {"NULL", "SOH", "STX", "ETX", "EOT",
     "ENQ", "ACK", "BEL", "BS", "HT",
     "LF", "VT", "FF", "CR", "SO",
     "SI", "DLE", "DC1", "DC2", "DC3",
     "DC4", "NAK", "SYN", "ETB", "CAN",
     "EM", "SUB", "ESC", "FS", "GS",
     "RS", "US", "space", "!",
     "\"", "#", "$", "%", "&",
     "\'", "(", ")", "*", "+",
     ",", "-", ".", "/", "0",
     "1", "2", "3", "4", "5",
     "6", "7", "8", "9", ":",
     ";", "<", "=", ">", "?",
     "@", "A", "B", "C", "D",
     "E", "F", "G", "H", "I",
     "J", "K", "L", "M", "N",
     "O", "P", "Q", "R", "S",
     "T", "U", "V", "W", "X",
     "Y", "Z", "[", "\\", "]",
     "^", "_", "`", "a", "b",
     "c", "d", "e", "f", "g",
     "h", "i", "j", "k", "l",
     "m", "n", "o", "p", "q",
     "r", "s", "t", "u", "v",
     "w", "x", "y", "z", "{",
     "|", "}", "~","DEL", "Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å", "É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "ø", "£", "Ø", "×", "ƒ", "á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "®", "¬", "½", "¼", "¡", "«", "»", "░", "▒", "▓", "│", "┤", "Á", "Â", "À", "©", "╣", "║", "╗", "╝", "¢", "¥", "┐", "└", "┴", "┬", "├", "─", "┼", "ã", "Ã", "╚", "╔", "╩", "╦", "╠", "═", "╬", "¤", "ð", "Ð", "Ê", "Ë", "È", "ı", "Í", "Î", "Ï", "┘", "┌", "█", "▄", "¦", "Ì", "▀", "Ó", "ß", "Ô", "Ò", "õ", "Õ", "µ", "þ", "Þ", "Ú", "Û", "Ù", "ý", "Ý", "¯", "´", "≡", "±", "‗", "¾", "¶", "§", "÷", "¸", "°", "¨", "·", "¹", "³", "²", "■", "nbsp",};

  Interpretor ()
  {
    fFile =fopen("/dev/stdout", "r+");
    if(!fFile)
      {
        throw ("initialization error: /dev/stdout cannot be open");
      }
    fMemory.resize (1024);

  }
  ~Interpretor ()
  {
    fclose(fFile);
  }

  static Interpretor* fInstance;

  string fBFData;
  vector <unsigned char> fMemory;

  int fMemoryPos=0;
  int fFilePos=0;
  int fBFDataPos=0;
  bool fSwitchPos=0;

  FILE *fFile;
  bool fDebug;

  int fFurtherstMemory=1;
  bool fDumpMem=false;
  int fDumpMemNbCol=20;

public:


  static int Interpret (ifstream &f, bool debug=false, bool dumpMem=false)
  {
    if (!fInstance)
      fInstance=new Interpretor();

    fInstance->fDebug=debug;
    fInstance->fDumpMem=dumpMem;
    fInstance->ReadBFdata (f);

    int ToReturn;
    while (fInstance->fBFDataPos < fInstance->fBFData.size ())
      {
        if (fInstance->fDebug == true)
          {
            clog << "Interpreting " << fInstance->fBFData[fInstance->fBFDataPos] << endl;
          }
        fInstance->DecideNext ();
      }

    return ToReturn;
  }

  int ReadBFdata (ifstream &f)
  {
    string currentData;
    while (f>>currentData)
      fBFData+=currentData;
  }

  int& GetMemOrFilePos ()
  {
    if (GetIsMemOrFile ())
      return fMemoryPos;
    else
      return fFilePos;
  }

  bool GetIsMemOrFile ()
  {
    if (fSwitchPos)
      return false;
    else
      return true;
  }

  int ShiftMemPos (char c)
  {
    int& toShiftPtr=GetMemOrFilePos();

    if (fDebug)
      clog << "we are shifting mode, "<<c<< " " << " at position " << toShiftPtr<<endl;
    if (c == '<')
      {
        toShiftPtr--;
        if (toShiftPtr < 0)
          toShiftPtr=0;
      }
    else if (c == '>')
      {
        toShiftPtr++;
        if (toShiftPtr >= fMemory.size () && GetIsMemOrFile ())
          fMemory.resize (toShiftPtr*1.5);
        if (toShiftPtr > fFurtherstMemory)
          fFurtherstMemory = toShiftPtr;
      }

    else return 1;

    return 0;
  }

  int Increment (char c)
  {
    int& toShiftPtr=GetMemOrFilePos();
    bool IsMemOrFile=GetIsMemOrFile ();
    if (IsMemOrFile)
      {
        if (fDebug)
          clog << "we are in memory mode, "<<c<< " " << "at position " << fMemoryPos<<endl;
        if (c == '+')
          fMemory[fMemoryPos]++;
        else
          fMemory[fMemoryPos]--;
      }
    else
      {
        if (fDebug)
          clog << "we are in file mode, "<<c<< " " << "at position " << fMemoryPos<<endl;
        char c = GetFileByte ();
        if (c == '+')
          c++;
        else
          c--;
        if (ftell (fFile) != -1)
          if (fseek(fFile,fFilePos,SEEK_SET))
            return 0;
        fputc(c,fFile);fflush(fFile);
      }

  }

  int SwitchMemFile ()
  {
    bool IsMemOrFile=GetIsMemOrFile ();
    if (!IsMemOrFile)
      {
        fSwitchPos=0;
        return 0;
      }

    string filename;
    int currentPos=fMemoryPos;
    int tmpMemPos=fMemoryPos;
    while (fMemory[tmpMemPos] != 0)
      {
        filename.push_back(fMemory[tmpMemPos++]);
      }

    if (fDebug)
      clog << "switch to file " << filename << endl;

    fclose (fFile);
    fFile=0;
    if (!access(filename.data (), W_OK) &&
        !access(filename.data (), R_OK))
      {
        if (fDebug)
          clog << "opening in rw"<< endl;
        fFile=fopen(filename.data (),"r+");
      }
    else if (!access(filename.data (), R_OK))
      {
        if (fDebug)
          clog << "opening in r only"<< endl;
        fFile=fopen(filename.data (),"r");
      }
    if (!fFile)
      {
        if (fDebug)
          clog << "create new file in rw" << endl;

        fFile=fopen(filename.data (),"w+");
        if (!fFile)
          {
            string error="Cannot open or create file ";
            throw (error+filename);
          }
      }
    fFilePos=0;
    fSwitchPos=1;
  }

  char GetFileByte ()
  {
    unsigned char c=0;
    if (ftell(fFile) != -1)
      if (fseek(fFile,fFilePos,SEEK_SET))
        return 0;
    if (!feof(fFile))
      c=fgetc (fFile);
    return c;
  }

  int PutMemToFile ()
  {
    if (ftell (fFile) != -1)
      {
        if (fseek(fFile,fFilePos,SEEK_SET))
          return 0;

        /*        if (ftell (fFile) != -1 &&
            (fFile.rdstate() & fstream::eofbit) != 0)
          {
            fFile.putback (fMemory[fMemoryPos]);
          }
        */

        fputc (fMemory[fMemoryPos],fFile);fflush (fFile);

      }
    else
      {
        fputc (fMemory[fMemoryPos],fFile); fflush (fFile);
      }

  }
  int PutFileToMem ()
  {
    char tmpc=0;
        if (ftell (fFile) != -1)
          {

            if (fseek(fFile,fFilePos,SEEK_SET))
              return 0;
            //if ((fFile.rdstate() & fstream::eofbit) != 0)
            if (!feof(fFile))
              tmpc=fgetc (fFile);
            if (fDebug)
              clog << "->value;pos " << int(fMemory[fMemoryPos])<<";"<<fMemoryPos << endl;

          }
        else
          {
            if (!feof(fFile))
              tmpc=fgetc (fFile);
            if (fDebug)
              clog << "->value;pos " << int(fMemory[fMemoryPos])<<";"<<fMemoryPos << endl;
          }
        fMemory[fMemoryPos]=(*(unsigned char*)(&tmpc));
  }

  int Put (bool reverse=false)
  {
    bool IsMemOrFile= GetIsMemOrFile ();

    if (IsMemOrFile)
      {
        if (!reverse)
          {
            PutMemToFile ();
          }
        else
          {
            PutFileToMem ();
          }
        fFilePos++;
      }
    else
      {
        if (reverse)
          {
            PutMemToFile ();
          }
        else
          {
            PutFileToMem ();
          }
        fMemoryPos++;
        if (fMemoryPos > fFurtherstMemory)
          fFurtherstMemory = fMemoryPos;
      }
  }

  int rewind ()
  {
    bool MemOrFile = GetIsMemOrFile ();
    if ((MemOrFile && fMemory[fMemoryPos] == 0) ||
        (!MemOrFile && GetFileByte () == 0))
      {
        return 0;
      }

    int parCounter=1;
    fBFDataPos--;
    while (parCounter>0 && fBFDataPos >=0)
      {
        fBFDataPos--;
        if (fBFData[fBFDataPos] == '[' )
          parCounter--;
        else if (fBFData[fBFDataPos] == ']')
          parCounter++;
      }

    if (fBFDataPos < 0)
      throw ("syntax error, [ missing.");
    return 0;

  }

  int unwind ()
  {
    bool MemOrFile = GetIsMemOrFile ();
    if ((MemOrFile && fMemory[fMemoryPos] != 0) ||
        (!MemOrFile && GetFileByte () != 0))
      {
        return 0;
      }

    int parCounter=1;

    while (parCounter>0 && fBFDataPos < fBFData.size ())
      {
        if (fBFData[fBFDataPos] == '[' )
          parCounter++;
        else if (fBFData[fBFDataPos] == ']')
          parCounter--;
        fBFDataPos++;
      }

    if (fBFDataPos > fBFData.size ())
      throw ("syntax error, ] missing.");
    return 0;

  }

  int MemoryDump ()
  {
    if (!fDumpMem)
      return 0;
    clog << endl;
    for (int i=0; i<=fFurtherstMemory/fDumpMemNbCol; i++)
      {
        for (int valit=0; valit<fDumpMemNbCol; valit++)
          {
            int currentPos=valit+i*fDumpMemNbCol;
            clog.width (5);
            if (currentPos == fMemoryPos)
              clog << "\033[1m"<<int(fMemory[currentPos]) << "\033[0m ";
            else
              clog <<int(fMemory[currentPos]) << " ";
            clog.width (0);
          }
        clog << endl;
        for (int sit=0; sit<fDumpMemNbCol; sit++)
          {
            int currentPos = sit+i*fDumpMemNbCol;
            clog.width (5);
            if (currentPos == fMemoryPos)
              clog << "\033[1m"<<signlist[fMemory[currentPos]] << "\033[0m ";
            else
              clog <<signlist[fMemory[currentPos]] << " ";
            clog.width (0);
          }
        clog << endl;
        for (int sit=0; sit<fDumpMemNbCol; sit++)
          {
            clog << "------";
          }
        clog << endl;
      }
    cout << endl;
  }

  int DecideNext ()
  {
    return Decide (fBFData[fBFDataPos++]);
  }

  int Decide (char c)
  {
    switch (c)
      {
      case '<':
        return ShiftMemPos ('<');
        break;
      case '>':
        return ShiftMemPos ('>');
        break;
      case '[':
        return unwind ();
        break;
      case ']':
        return rewind ();
        break;
      case '+':
        return Increment('+');
        break;
      case '-':
        return Increment ('-');
        break;
      case '.':
        return Put ();
        break;
      case ',':
        return Put (true);
        break;
      case '~':
        SwitchMemFile ();
        break;
      case '#':
        MemoryDump ();
        break;
      case '\\':
        if (fDumpMem)
          {
            clog.flush (); cout.flush();
            throw (string("User break\n"));
          }
        break;

      }

  }
};
Interpretor* Interpretor::fInstance=0;

int main (int argc, char** argv)
{
  if (argc < 2)
    {
      cerr << "error: need input file" << endl;
      return 1;
    }

  ifstream finput (argv[1]);

  if (!finput.is_open ())
    {
      cerr << "Can't open file" << endl;
      return 1;
    }
  bool debug=false;
  bool dumpMem = false;
  for (int i=2; i<argc; i++)
    {
      string args=argv[i];
      if (args == "-d")
        debug=true;
      else if (args == "-dm")
        dumpMem=true;
    }

  int ToReturn;

  try {
    ToReturn=Interpretor::Interpret (finput,debug, dumpMem);
  }
  catch (string e)
    {
      cerr << e<<endl;
    }
  return ToReturn;
}
