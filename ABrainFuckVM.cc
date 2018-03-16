#include <unistd.h>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using namespace std;


class Interpretor
{
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

public:
  static int Interpret (ifstream &f, bool debug=false)
  {
    if (!fInstance)
      fInstance=new Interpretor();

    fInstance->fDebug=debug;
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
          fseek(fFile,fFilePos,SEEK_SET);
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
    fFile==0;
    if (!access(filename.data (), W_OK) &&
        !access(filename.data (), R_OK))
      {
        fFile=fopen(filename.data (),"r+");
      }
    else if (!fFile && !access(filename.data (), W_OK))
      {
        fFile=fopen(filename.data (),"w+");
      }
    else if (!access(filename.data (), R_OK))
      {
        fFile=fopen(filename.data (),"r");
      }
    if (!fFile)
      {
        string error="Cannot open file";
        throw (error+filename);
      }
    fFilePos=0;
    fSwitchPos=1;
  }

  char GetFileByte ()
  {
    unsigned char c;
    if (ftell(fFile) != -1)
      fseek(fFile,fFilePos,SEEK_SET);
    c=fgetc (fFile);
    return c;
  }

  int PutMemToFile ()
  {
    if (ftell (fFile) != -1)
      {
        fseek(fFile,fFilePos,SEEK_SET);

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
    char tmpc;
        if (ftell (fFile) != -1)
          {
            fseek(fFile,fFilePos,SEEK_SET);

            //if ((fFile.rdstate() & fstream::eofbit) != 0)

            tmpc=fgetc (fFile);

          }
        else
          {
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
  if (argc > 2)
    debug=true;

  return Interpretor::Interpret (finput,debug);
}
