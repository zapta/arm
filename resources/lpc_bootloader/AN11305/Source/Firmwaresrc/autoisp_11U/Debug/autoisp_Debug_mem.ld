/*
 * GENERATED FILE - DO NOT EDIT
 * (C) Code Red Technologies Ltd, 2008-2012
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v4.2.4_beta [Build 299] [28/06/2012]  on Thu Nov 08 12:55:20 PST 2012)
*/

MEMORY
{
  /* Define each memory region */
  MFlash128 (rx) : ORIGIN = 0x0, LENGTH = 0x20000 /* 128k */
  RamLoc8 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x2000 /* 8k */
  RamUsb2 (rwx) : ORIGIN = 0x20004000, LENGTH = 0x800 /* 2k */

}
  /* Define a symbol for the top of each memory region */
  __top_MFlash128 = 0x0 + 0x20000;
  __top_RamLoc8 = 0x10000000 + 0x2000;
  __top_RamUsb2 = 0x20004000 + 0x800;

