#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "semi65x.h"
#include "6502core.h"
#include "disas.h"
#include "mapfile.h"

#define RAMSIZE 65536
#define IOPORT 0xfff0

unsigned char WholeRam[RAMSIZE];
int MachineType;
bool DebugEnabled = false;

int
BeebReadMem (int addr)
{
  return WholeRam[addr];
}

int
DebugReadMem (int addr, bool host)
{
  return BeebReadMem (addr);
}

void
BeebWriteMem (int addr, unsigned char val)
{
  if (addr == IOPORT)
    {
      fputc (val, stdout);
      fflush (stdout);
    }

  WholeRam[addr] = val;
}

void
WriteLog (const char *fmt, ...)
{
  /* Do nothing.  */
}

int
DebugDisassembleInstruction (int addr, bool host, char *opstr)
{
  /* Do nothing.  */
  return 0;
}

bool
DebugDisassembler (int addr, int prevAddr, int Accumulator, int XReg, int Yreg,
		   int PSR, int StackReg, bool host)
{
  /* Do nothing.  */
  return true;
}

void
beebmem_dumpstate (void)
{
  /* Do nothing.  */
}

void
Sleep (int)
{
  /* Do nothing.  */
}

void
print_flags (FILE *f, int flags)
{
  fputc (flags & FlagN ? 'N' : 'n', f);
  fputc (flags & FlagV ? 'V' : 'v', f);
  fputc ('-', f);
  fputc (flags & FlagB ? 'B' : 'b', f);
  fputc (flags & FlagD ? 'D' : 'd', f);
  fputc (flags & FlagI ? 'I' : 'i', f);
  fputc (flags & FlagZ ? 'Z' : 'z', f);
  fputc (flags & FlagC ? 'C' : 'c', f);
}

class watchinfo
{
public:
  watchinfo(unsigned int size, unsigned long addr) : addr(addr), size(size)
    {
      prev = getmem(size, addr);
    }

  unsigned int getbyte(unsigned long addr)
    {
      return BeebReadMem(addr);
    }

  unsigned int getword(unsigned long addr)
    {
      return BeebReadMem(addr) | (BeebReadMem(addr + 1) << 8);
    }

  unsigned int getdword(unsigned long addr)
    {
      return BeebReadMem(addr) | (BeebReadMem(addr + 1) << 8)
	     | (BeebReadMem(addr + 2) << 16) | (BeebReadMem(addr + 3) << 24);
    }

  unsigned int getmem(unsigned int size, unsigned long addr)
    {
      switch (size)
	{
	case 1: return getbyte(addr);
	case 2: return getword(addr);
	case 4: return getdword(addr);
	}
      return -1;
    }

  void update()
    {
      prev = getmem(size, addr);
    }

  void show_diff(FILE *f)
    {
      unsigned int curr = getmem(size, addr);
      if (curr != prev)
        {
	  switch (size)
            {
	    case 1:
	      fprintf (f, " [%.4lx]=%.2x->%.2x", addr, prev, curr);
	      break;
	    case 2:
	      fprintf (f, " [%.4lx]=%.4x->%.4x", addr, prev, curr);
	      break;
	    case 4:
	      fprintf (f, " [%.4lx]=%.8x->%.8x", addr, prev, curr);
	      break;
	    }
	  prev = curr;
	}
    }

private:
  unsigned long addr;
  unsigned int prev;
  unsigned int size;
};

int
main (int argc, char* argv[])
{
  bool quit = false;
  int arg = 1;
  int load_addr = 0;
  int exec_addr = -1;
  struct stat statbuf;
  bool verbose = false;
  bool trace = false;
  bool watch = false;
  const char *mapfile = NULL;
  std::vector<watchinfo> watchpoints;

  MachineType = 1;

  for (arg = 1; arg < argc;)
    {
      if (strcmp (argv[arg], "-c") == 0)
        {
	  arg++;
	  if (strcmp (argv[arg], "6502") == 0
	      || strcmp (argv[arg], "6512") == 0)
	    MachineType = 1;
	  else if (strcasecmp (argv[arg], "65C02") == 0
		   || strcasecmp (argv[arg], "65SC12") == 0)
	    MachineType = 3;
	  else
	    {
	      fprintf (stderr, "Unknown CPU '%s'\n", argv[arg]);
	      exit (1);
	    }
	}
      else if (strcmp (argv[arg], "-l") == 0)
	load_addr = strtol (argv[++arg], NULL, 0);
      else if (strcmp (argv[arg], "-e") == 0)
        exec_addr = strtol (argv[++arg], NULL, 0);
      else if (strcmp (argv[arg], "-v") == 0)
        verbose = true;
      else if (strcmp (argv[arg], "-t") == 0)
	trace = true;
      else if (strcmp (argv[arg], "-m") == 0)
        mapfile = argv[++arg];
      else if (strncmp (argv[arg], "-w", 2) == 0)
	{
	  int watchsize = 0;
	  switch (argv[arg][2])
	    {
	    case 'b':
	      watchsize = 1;
	      break;
	    case 'w':
	    case 'h':
	      watchsize = 2;
	      break;
	    case 'l':
	    case 'd':
	      watchsize = 4;
	      break;
	    default:
	      fprintf (stderr, "Watch what? (Use b/[wh]/[ld])\n");
	      exit (1);
	    }
	  unsigned long watchaddr = strtoul (argv[++arg], NULL, 0);
	  if (verbose)
	    fprintf (stderr, "Watching %s at 0x%.4lx\n",
		     watchsize == 1 ? "byte" :
		     watchsize == 2 ? "word" :
		     watchsize == 4 ? "long" : "?", watchaddr);
	  watch = true;
	  watchpoints.emplace_back(watchsize, watchaddr);
	}
      else
        {
	  FILE *f;
	  int readbytes;
	  
	  if (stat (argv[arg], &statbuf) != 0)
	    {
	      perror ("main");
	      exit (1);
	    }
	  
	  if (statbuf.st_size + load_addr >= RAMSIZE)
	    {
	      fprintf (stderr, "Program doesn't fit in RAM, exiting.\n");
	      exit (1);
	    }
	  
	  f = fopen (argv[arg], "r");
	  
	  if (!f)
	    {
	      fprintf (stderr, "Couldn't open binary '%s'\n", argv[arg]);
	      exit (1);
	    }
	  
	  readbytes = fread (&WholeRam[load_addr], 1, statbuf.st_size, f);

	  if (verbose)
	    fprintf (stderr, "Read %d bytes to 0x%.4x\n", readbytes, load_addr);
	  
	  fclose (f);
	}

      arg++;
    }
  
  if (mapfile)
    parse_map (mapfile);
  
  /* Start execution at beginning of the loaded executable if not otherwise
     specified.  */
  if (exec_addr == -1)
    exec_addr = load_addr;

  if (verbose)
    fprintf (stderr, "Load addr: 0x%.4x  Exec addr: 0x%.4x\n", load_addr,
	     exec_addr);

  /* Point reset vector to loaded binary's execution address.  */
  BeebWriteMem (0xfffc, exec_addr & 255);
  BeebWriteMem (0xfffd, (exec_addr >> 8) & 255);

  for (auto &wb : watchpoints)
    wb.update();

  Init6502core ();

  /* Do the math!  */
  while (!quit)
    {
      if (trace)
        {
	  extern int ProgramCounter, Accumulator, XReg, YReg;
	  extern unsigned char StackReg, PSR;
	  fprintf (stderr, "PC=%.4x A=%.2x X=%.2x Y=%.2x S=%.2x F=",
		   ProgramCounter, Accumulator, XReg, YReg, StackReg);
	  print_flags (stderr, PSR);
	  fputc (' ', stderr);
	  disassemble_insn (stderr, ProgramCounter, &WholeRam[ProgramCounter]);
	  if (!watch)
	    fprintf (stderr, "\n");
	}
      Exec6502Instruction ();
      if (trace && watch)
        {
	  for (auto &wp : watchpoints)
	    wp.show_diff(stderr);
	  fprintf (stderr, "\n");
	}
    }
  
  return 0;
}
