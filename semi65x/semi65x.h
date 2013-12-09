#ifndef SEMI65X_H
#define SEMI65X_H 1

#include <stdint.h>

typedef uint64_t CycleCountT;

typedef void CArm;

extern unsigned char WholeRam[65536];

#define BEEBWRITEMEM_DIRECT(A,V) \
  do { WholeRam[(A)] = (V); } while (0)

extern int BeebReadMem (int);
extern void BeebWriteMem (int, unsigned char);
extern void beebmem_dumpstate (void);
extern int DebugDisassembleInstruction (int addr, bool host, char *opstr);
extern void WriteLog (const char *, ...);
extern int DebugReadMem (int, bool);
extern bool DebugDisassembler (int addr, int prevAddr, int Accumulator, int XReg, int YReg, int PSR, int StackReg, bool host);
extern void Sleep (int);


extern int MachineType;
extern bool DebugEnabled;
extern uint64_t CycleCountWrap;

#endif
