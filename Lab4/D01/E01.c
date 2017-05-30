// Davide Gallitelli - S241521 - Politecnico di Torino
// To test the code, change the name to main.c and move it to paging/src folder

// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "paging.h"

int main(struct multiboot *mboot_ptr)
{
    // Initialise all the ISRs and segmentation
    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();

    initialise_paging();
    monitor_write("Hello, paging world!\n");
		
		u32int i = 0;
		u32int *ptr = 0;
		u32int do_page_fault;
		while(1){
			do_page_fault = *ptr;
			monitor_write("Normal access at address ");
			monitor_write_hex(ptr);
			monitor_wite(" at page ");
			monitor_write_dec(i);
			monitor_write("\n");
			ptr = ptr + (u32int)1024;
			i++;
		}
    return 0;
}