/*
 * GENERATED FILE - DO NOT EDIT
 * (C) Code Red Technologies Ltd, 2008-9    
 * Generated C linker script file for LPC1343 
 * (created from nxp_lpc13_c.ld (v3.1.4 (200912230917)) on Mon Jan 11 14:36:17 PST 2010)
*/

INCLUDE "../ld/crp_enabled_lib.ld "
INCLUDE "../ld/crp_enabled_mem.ld "

ENTRY(ResetISR)

SECTIONS
{
	.text :
	{
		KEEP(*(.isr_vector))
		. = 0x000002FC;
		KEEP(*(.crp))
		*(.text*)
		*(.rodata*)

	} > MFlash32


	/* for exception handling/unwind - some Newlib functions (in common with C++ and STDC++) use this. */
	
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > MFlash32

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > MFlash32
	__exidx_end = .;

	_etext = .;
		
	.data : AT (__exidx_end)
	{
		_data = .;
		*(vtable)
		*(.data*)
		_edata = .;
	} > RamLoc8

	/* zero initialized data */
	.bss :
	{
		_bss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > RamLoc8
	
	/* Where we put the heap with cr_clib */
	.cr_heap :
	{
		end = .;
		_pvHeapStart = .;
	} > RamLoc8

/*
	Note: (ref: M0000066)
	Moving the stack down by 16 is to work around a GDB bug.
	This space can be reclaimed for Production Builds.
*/	
	_vRamTop = __top_RamLoc8 ;
	_vStackTop = _vRamTop - 16;
}
