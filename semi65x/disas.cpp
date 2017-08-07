#include <stdio.h>
#include <string>

#include "mapfile.h"

static const char *opcodes[] =
{
  /* 0x00 */
  "brk", "ora", "$02", "$03", "tsb", "ora", "asl", "$07",
  "php", "ora", "asl", "$0b", "tsb", "ora", "asl", "$0f",
  /* 0x10 */
  "bpl", "ora", "ora", "$13", "trb", "ora", "asl", "$17",
  "clc", "ora", "inc", "$1b", "trb", "ora", "asl", "$1f",
  /* 0x20 */
  "jsr", "and", "$22", "$23", "bit", "and", "rol", "$27",
  "plp", "and", "rol", "$2b", "bit", "and", "rol", "$2f",
  /* 0x30 */
  "bmi", "and", "and", "$33", "bit", "and", "rol", "$37",
  "sec", "and", "dec", "$3b", "bit", "and", "rol", "$3f",
  /* 0x40 */
  "rti", "eor", "$42", "$43", "bit", "eor", "lsr", "$47",
  "pha", "eor", "lsr", "$4b", "jmp", "eor", "lsr", "$4f",
  /* 0x50 */
  "bvc", "eor", "eor", "$53", "$54", "eor", "lsr", "$57",
  "cli", "eor", "phy", "$5b", "jmp", "eor", "lsr", "$5f",
  /* 0x60 */
  "rts", "adc", "$62", "$63", "stz", "adc", "ror", "$67",
  "pla", "adc", "ror", "$6b", "jmp", "adc", "ror", "$6f",
  /* 0x70 */
  "bvs", "adc", "adc", "$73", "stz", "adc", "ror", "$77",
  "sei", "adc", "ply", "$7b", "jmp", "adc", "ror", "$7f",
  /* 0x80 */
  "bra", "sta", "$82", "$83", "sty", "sta", "stx", "$87",
  "dey", "bit", "txa", "$8b", "sty", "sta", "stx", "$8f",
  /* 0x90 */
  "bcc", "sta", "sta", "$93", "sty", "sta", "stx", "$97",
  "tya", "sta", "txs", "$9b", "stz", "sta", "stz", "$9f",
  /* 0xa0 */
  "ldy", "lda", "ldx", "$a3", "ldy", "lda", "ldx", "$a7",
  "tay", "lda", "tax", "$ab", "ldy", "lda", "ldx", "$af",
  /* 0xb0 */
  "bcs", "lda", "lda", "$b3", "ldy", "lda", "ldx", "$b7",
  "clv", "lda", "tsx", "$bb", "ldy", "lda", "ldx", "$bf",
  /* 0xc0 */
  "cpy", "cmp", "$c2", "$c3", "cpy", "cmp", "dec", "$c7",
  "iny", "cmp", "dex", "$cb", "cpy", "cmp", "dec", "$cf",
  /* 0xd0 */
  "bne", "cmp", "cmp", "$d3", "$d4", "cmp", "dec", "$d7",
  "cld", "cmp", "phx", "$db", "$dc", "cmp", "dec", "$df",
  /* 0xe0 */
  "cpx", "sbc", "$e2", "$e3", "cpx", "sbc", "inc", "$e7",
  "inx", "sbc", "nop", "$eb", "cpx", "sbc", "inc", "$ef",
  /* 0xf0 */
  "beq", "sbc", "sbc", "$f3", "$f4", "sbc", "inc", "$f7",
  "sed", "sbc", "plx", "$fb", "$fc", "sbc", "inc", "$ff"
};

enum addrmode
{
  UNKNOWN,
  IMPLIED,
  INDIRECT,
  INDEXED_INDIRECT,
  ZP,
  ACCUMULATOR,
  IMMEDIATE,
  ABSOLUTE,
  INDIRECT_INDEXED,
  ZP_X,
  ZP_Y,
  ABS_X,
  ABS_Y,
  IND_ABS_X,
  RELATIVE
};

static const char *softregs[] =
{
  "_sp0", "_sp1", "_fp0", "_fp1",
  "_r0", "_r1", "_r2", "_r3", "_r4", "_r5", "_r6", "_r7",
  "_s0", "_s1", "_s2", "_s3", "_s4", "_s5", "_s6", "_s7",
  "_e0", "_e1", "_e2", "_e3", "_e4", "_e5", "_e6", "_e7",
  "_e8", "_e9", "_e10", "_e11", "_e12", "_e13", "_e14", "_e15",
  "_e16", "_e17", "_e18", "_e19", "_e20", "_e21", "_e22", "_e23",
  "_e24", "_e25", "_e26", "_e27", "_e28", "_e29", "_e30", "_e31",
  "_tmp0", "_tmp1", "_sa", "_sx", "_sy"
};

#define ARRAYLEN(X) (sizeof(X) / sizeof(X[0]))

static unsigned softreg_base = 0;

static const char *fancy_name (unsigned regnum)
{
  if (regnum >= softreg_base && regnum < softreg_base + ARRAYLEN (softregs))
    return softregs[regnum - softreg_base];

  return NULL;
}

/* Print disassembled insn to OUTFILE. Return number of bytes consumed.  */

int
disassemble_insn (FILE *outfile, unsigned int pc, unsigned char *insn)
{
  int opcode = insn[0];
  addrmode am;
  const char *regname;
  
  switch (opcode & 0x3)
    {
    case 0x1:
      switch (opcode)
	{
	case 0x01: case 0x21: case 0x41: case 0x61:
	case 0x81: case 0xa1: case 0xc1: case 0xe1:
	  am = INDEXED_INDIRECT;
	  break;

	case 0x05: case 0x25: case 0x45: case 0x65:
	case 0x85: case 0xa5: case 0xc5: case 0xe5:
	  am = ZP;
	  break;

	case 0x09: case 0x29: case 0x49: case 0x69:
	case 0xa9: case 0xc9: case 0xe9:
	  am = IMMEDIATE;
	  break;

	case 0x0d: case 0x2d: case 0x4d: case 0x6d:
	case 0x8d: case 0xad: case 0xcd: case 0xed:
	  am = ABSOLUTE;
	  break;

	case 0x11: case 0x31: case 0x51: case 0x71:
	case 0x91: case 0xb1: case 0xd1: case 0xf1:
	  am = INDIRECT_INDEXED;
	  break;

	case 0x15: case 0x35: case 0x55: case 0x75:
	case 0x95: case 0xb5: case 0xd5: case 0xf5:
	  am = ZP_X;
	  break;

	case 0x19: case 0x39: case 0x59: case 0x79:
	case 0x99: case 0xb9: case 0xd9: case 0xf9:
	  am = ABS_Y;
	  break;

	case 0x1d: case 0x3d: case 0x5d: case 0x7d:
	case 0x9d: case 0xbd: case 0xdd: case 0xfd:
	  am = ABS_X;
	  break;
	
	default:
	  am = UNKNOWN;
	}
      break;

    case 0x2:
      switch (opcode)
        {
	case 0xa2:
	  am = IMMEDIATE;
	  break;

	case 0x06: case 0x26: case 0x46: case 0x66:
	case 0x86: case 0xa6: case 0xc6: case 0xe6:
	  am = ZP;
	  break;
	
	case 0x0a: case 0x2a: case 0x4a: case 0x6a:
	  am = ACCUMULATOR;
	  break;

	case 0x0e: case 0x2e: case 0x4e: case 0x6e:
	case 0x8e: case 0xae: case 0xce: case 0xee:
	  am = ABSOLUTE;
	  break;
	
	case 0x16: case 0x36: case 0x56: case 0x76:
	case 0xd6: case 0xf6:
	  am = ZP_X;
	  break;
	
	case 0x96: case 0xb6:
	  am = ZP_Y;
	  break;
	
	case 0x1e: case 0x3e: case 0x5e: case 0x7e:
	case 0xde: case 0xfe:
	  am = ABS_X;
	  break;
	
	case 0xbe:
	  am = ABS_Y;
	  break;
	
	default:
	  am = UNKNOWN;
	}
      break;

    case 0x0:
      switch (opcode)
        {
	case 0xa0: case 0xc0: case 0xe0:
	  am = IMMEDIATE;
	  break;
	
	case 0x24: case 0x84: case 0xa4: case 0xc4: case 0xe4:
	  am = ZP;
	  break;
	
	case 0x2c: case 0x4c: case 0x6c: case 0x8c: case 0xac:
	case 0xcc: case 0xec:
	  am = ABSOLUTE;
	  break;
	
	case 0x94: case 0xb4:
	  am = ZP_X;
	  break;
	
	case 0xbc:
	  am = ABS_X;
	  break;
	
	default:
	  am = UNKNOWN;
	}
      break;

    case 0x3:
      am = UNKNOWN;
    }

  /* Branch instructions.  */
  if ((opcode & 0x1f) == 0x10)
    am = RELATIVE;
  
  switch (opcode)
    {
    case 0x00:
    case 0x40:
    case 0x60:
    case 0x08: case 0x28: case 0x48: case 0x68:
    case 0x88: case 0xa8: case 0xc8: case 0xe8:
    case 0x18: case 0x38: case 0x58: case 0x78:
    case 0x98: case 0xb8: case 0xd8: case 0xf8:
    case 0x8a: case 0x9a: case 0xaa: case 0xba:
    case 0xca: case 0xea:
      am = IMPLIED;
      break;
    
    case 0x20:
      am = ABSOLUTE;
      break;
    
    /* 65C02 insns.  */
    case 0x1a: case 0x3a: case 0x5a: case 0x7a:
    case 0xda: case 0xfa:
      am = IMPLIED;
      break;
    
    case 0x12: case 0x32: case 0x52: case 0x72:
    case 0x92: case 0xb2: case 0xd2: case 0xf2:
      am = INDIRECT;
      break;
    
    case 0x7c:
      am = IND_ABS_X;
      break;
    
    case 0x89:
      am = IMMEDIATE;
      break;

    case 0x34: case 0x74:
      am = ZP_X;
      break;
    
    case 0x3c: case 0x9e:
      am = ABS_X;
      break;
    
    case 0x04:
    case 0x14: case 0x64:
    /* Some 65C02s only.  */
    case 0x07: case 0x17: case 0x27: case 0x37:
    case 0x47: case 0x57: case 0x67: case 0x77:
    case 0x87: case 0x97: case 0xa7: case 0xb7:
    case 0xc7: case 0xd7: case 0xe7: case 0xf7:
      am = ZP;
      break;
    
    case 0x0c: case 0x1c: case 0x9c:
      am = ABSOLUTE;
      break;
    
    case 0x80:
      am = RELATIVE;
      break;
    
    default:
      ;
    }

  /*fprintf (outfile, "(%.2x) ", opcode);*/

  switch (am)
    {
    case RELATIVE:
      {
        int addr = pc + (signed char) insn[1] + 2;
	const char *symbolic_name = find_symbol (addr).c_str();
	fprintf (outfile, "%s %s", opcodes[opcode], symbolic_name);
	if (symbolic_name[0] != '$')
	  fprintf (outfile, " ($%.4x)", addr);
	return 2;
      }
    
    case INDIRECT:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s (%s)", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s ($%.2x)", opcodes[opcode], insn[1]);
      return 2;
    
    case INDEXED_INDIRECT:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s (%s,x)", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s ($%.2x,x)", opcodes[opcode], insn[1]);
      return 2;
    
    case ZP:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s %s", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s $%.2x", opcodes[opcode], insn[1]);
      return 2;

    case ZP_X:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s %s,x", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s $%.2x,x", opcodes[opcode], insn[1]);
      return 2;

    case ZP_Y:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s %s,y", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s $%.2x,y", opcodes[opcode], insn[1]);
      return 2;
    
    case ACCUMULATOR:
      fprintf (outfile, "%s a", opcodes[opcode]);
      return 1;
    
    case IMPLIED:
      fprintf (outfile, "%s", opcodes[opcode]);
      return 1;

    case IMMEDIATE:
      fprintf (outfile, "%s #$%.2x", opcodes[opcode], insn[1]);
      return 2;
    
    case ABSOLUTE:
      {
        int addr = insn[1] + insn[2] * 256;
	const char *symbolic_name = find_symbol (addr).c_str();
	fprintf (outfile, "%s %s", opcodes[opcode], symbolic_name);
	if (symbolic_name[0] != '$')
	  fprintf (outfile, " ($%.4x)", addr);
	return 3;
      }

    case ABS_X:
      fprintf (outfile, "%s $%.4x,x", opcodes[opcode],
	       insn[1] + insn[2] * 256);
      return 3;

    case ABS_Y:
      fprintf (outfile, "%s $%.4x,y", opcodes[opcode],
	       insn[1] + insn[2] * 256);
      return 3;
    
    case INDIRECT_INDEXED:
      if ((regname = fancy_name (insn[1])))
	fprintf (outfile, "%s (%s),y", opcodes[opcode], regname);
      else
	fprintf (outfile, "%s ($%.2x),y", opcodes[opcode], insn[1]);
      return 2;

    case IND_ABS_X:
      fprintf (outfile, "%s $%.4x,x", opcodes[opcode],
	       insn[1] + insn[2] * 256);
      return 3;
    
    case UNKNOWN:
      fprintf (outfile, "%s ??", opcodes[opcode]);
      return 1;
    }

  return -1;
}
