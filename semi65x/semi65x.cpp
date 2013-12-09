#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "semi65x.h"
#include "6502core.h"

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
      else
        {
	  FILE *f;
	  
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
	  
	  fread (&WholeRam[load_addr], 1, statbuf.st_size, f);
	  
	  fclose (f);
	}

      arg++;
    }
  
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

  Init6502core ();

  /* Do the math!  */
  while (!quit)
    {
      if (trace)
        {
	  extern int ProgramCounter;
	  fprintf (stderr, "pc: %.4x\n", ProgramCounter);
	}
      Exec6502Instruction ();
    }
  
  return 0;
}
