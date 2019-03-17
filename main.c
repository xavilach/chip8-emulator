#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    struct cpu_s* p_cpu = cpu_allocate();
    
    (void) argc;
    (void) argv;
    
    if(p_cpu)
    {
        cpu_load(p_cpu, NULL, 0, 0x200);
        
        cpu_run(p_cpu);
    }
    
    return 0;
}
