#include <zephyr/arch/common/semihost.h>

long semihost_exec(enum semihost_instr instr, void *args)
{

    long retval;
    switch (instr)
    {
        case SEMIHOST_WRITEC:
        {
            char arg1 = *(char *)args;
            __asm__ __volatile__ ("r0 = #%1\n\t"
                                  "r1 = %2\n\t"
                                  "trap0(#0)\n\t"
                                  "%0 = r0"
                                  : "=r"(retval)
                                  : "i"(SEMIHOST_WRITEC), "r"(&arg1), "m"(arg1)
                                  : "r0", "r1");
            return retval;
        }
        default:
            return 0;
    }
}