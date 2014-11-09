
#include <string.h>
#include <stdarg.h>

#include "chip.h"
#include <stdio.h>

#include "usb_serial.h"

#include <cr_section_macros.h>
#include "usb_serial/app_usbd_cfg.h"
#include "usb_serial/cdc_vcom.h"

// TODO: can this go into the namespace?
static USBD_HANDLE_T g_hUsb;

// Extern
/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc,
    uint32_t intfClass) {
  USB_COMMON_DESCRIPTOR *pD;
  USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
  uint32_t next_desc_adr;

  pD = (USB_COMMON_DESCRIPTOR *) pDesc;
  next_desc_adr = (uint32_t) pDesc;

  while (pD->bLength) {
    /* is it interface descriptor */
    if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

      pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
      /* did we find the right interface descriptor */
      if (pIntfDesc->bInterfaceClass == intfClass) {
        break;
      }
    }
    pIntfDesc = 0;
    next_desc_adr = (uint32_t) pD + pD->bLength;
    pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
  }

  return pIntfDesc;
}

// Extern,
const USBD_API_T *g_pUsbApi;


namespace usb_serial {

/* Initialize pin and clocks for USB0/USB1 port */
static void usb_pin_clk_init(void) {
  /* enable USB main clock */
  Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_PLLOUT, 1);
  /* Enable AHB clock to the USB block and USB RAM. */
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USBRAM);
  /* power UP USB Phy */
  Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_USBPAD_PD);
}

/**
 * @brief Handle interrupt from USB0
 * @return  Nothing
 */
extern "C" {
void USB_IRQHandler(void) {
  uint32_t *addr = (uint32_t *) LPC_USB->EPLISTSTART;

  /*  WORKAROUND for artf32289 ROM driver BUG:
   As part of USB specification the device should respond
   with STALL condition for any unsupported setup packet. The host will send
   new setup packet/request on seeing STALL condition for EP0 instead of sending
   a clear STALL request. Current driver in ROM doesn't clear the STALL
   condition on new setup packet which should be fixed.
   */
  if ( LPC_USB->DEVCMDSTAT & _BIT(8)) { /* if setup packet is received */
    addr[0] &= ~(_BIT(29)); /* clear EP0_OUT stall */
    addr[2] &= ~(_BIT(29)); /* clear EP0_IN stall */
  }
  USBD_API->hw->ISR(g_hUsb);
}
}


void setup() {
  // TODO(zapta): which ones can be non static?

   static USBD_API_INIT_PARAM_T usb_param;
   static USB_CORE_DESCS_T desc;

   ErrorCode_t ret = LPC_OK;

   /* enable clocks and pinmux */
   usb_pin_clk_init();

   // Get pointer to the base of the USB ROM struct.
   g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

   /* initialize call back structures */
   memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
   usb_param.usb_reg_base = LPC_USB0_BASE;
   /*  WORKAROUND for artf44835 ROM driver BUG:
    Code clearing STALL bits in endpoint reset routine corrupts memory area
    next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
    EP2_IN are used we need to specify 3 here. But as a workaround for this
    issue specify 4. So that extra EPs control structure acts as padding buffer
    to avoid data corruption. Corruption of padding memory doesn’t affect the
    stack/program behaviour.
    */
   usb_param.max_num_ep = 3 + 1;
   usb_param.mem_base = USB_STACK_MEM_BASE;
   usb_param.mem_size = USB_STACK_MEM_SIZE;

   /* Set the USB descriptors */
   desc.device_desc = (uint8_t *) &USB_DeviceDescriptor[0];
   desc.string_desc = (uint8_t *) &USB_StringDescriptor[0];
   /* Note, to pass USBCV test full-speed only devices should have both
    descriptor arrays point to same location and device_qualifier set to 0.
    */
   desc.high_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
   desc.full_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
   desc.device_qualifier = 0;

   /* USB Initialization */
   ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
   if (ret == LPC_OK) {

     /*  WORKAROUND for artf32219 ROM driver BUG:
      The mem_base parameter part of USB_param structure returned
      by Init() routine is not accurate causing memory allocation issues for
      further components.
      */
     usb_param.mem_base = USB_STACK_MEM_BASE
         + (USB_STACK_MEM_SIZE - usb_param.mem_size);

     /* Init VCOM interface */
     ret = vcom_init(g_hUsb, &desc, &usb_param);
     if (ret == LPC_OK) {
       /*  enable USB interrupts */
       NVIC_EnableIRQ(USB0_IRQn);
       /* now connect */
       USBD_API->hw->Connect(g_hUsb, 1);
     }

   }
}

// TODO(zapta): make the standard printf working for USB out.
void printf(const char *format, ...) {
  if (vcom_connected()) {

  // Assuming single thread, using static buffer.
  static char buf[80];
  va_list ap;
  va_start(ap, format);
  int n = vsnprintf(buf, sizeof(buf), (const char *) format, ap);
  // TODO: keep writing if only a portion was written. Abort if USB not connected.
  vcom_write((uint8_t*) buf, n);
  va_end(ap);
  }
}

}  // usb_serial



