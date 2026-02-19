/*
 * ======================================================================================================================
 *  lorawan.h - LoRaWAN Definations
 * ======================================================================================================================
 */
#include <lmic.h>    // MCCI_LoRaWAN_LMIC_library
#include <hal/hal.h> // Hardware Abstraction Layer - defines LMIC_UNUSED_PIN ...

/*
  LMIC_requestNetworkTime() does not work with TTN. We do not use it.
  Reason: TTN's free public community network server does not implement the LoRaWAN 
          DeviceTimeReq MAC command required by LMIC_requestNetworkTime.
          Paid The Things Stack (TTI) versions support it fully, but public 
          TTN omits it to avoid gateway/NS complexity
*/

/* 
  TTN Consoles https://console.cloud.thethings.network/
  Clusters
    EU  https://eu1.cloud.thethings.network/console
    US  https://nam1.cloud.thethings.network/console
    AS  https://au1.cloud.thethings.network/console
*/

/*
https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/eu868/
https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/us915/

LoRaWAN's Adaptive Data Rate algorithm actively maintains your device at fast data rates 
(low spreading factors like SF7/SF8)

  Your Distance	  Typical ADR Result	Max Safe Payload
  Close  (<1km)	  SF7–8 (DR4–5)	        222 bytes
  Medium (1–5km)  SF9–10 (DR2–3)	    115 bytes 
  Far    (>5km)	  SF11–12 (DR0–1)       51 bytes     
*/

/*
 * ======================================================================================================================
 *  LoRa Antenna Options
 *  https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/antenna-options
 *  
 *  Setting Country Frequencies
 *  
 *  MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h
 *  
 * // project-specific definitions
 * //#define CFG_eu868 1
 * #define CFG_us915 1
 * //#define CFG_au915 1
 * //#define CFG_as923 1
 * // #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP    // for as923-JP; also define CFG_as923
 * //#define CFG_kr920 1
 * //#define CFG_in866 1
 * #define CFG_sx1276_radio 1
 * //#define LMIC_USE_INTERRUPTS
 * 
 *  Compile for EU Frequencies 
 *    cd MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-eu lmic_project_config.h
 *  
 *  Compile for US Frequencies 
 *    cd MCCI_LoRaWAN_LMIC_library/project_config
 *    cp lmic_project_config.h-us lmic_project_config.h
 */

/*
 * ======================================================================================================================
 * Interrupt Research - We have the following that can generate interrupts: LoRa, Rain, Wind
 * ======================================================================================================================
 * The Adafruit Feather M0 uses the SAMD21 (ARM Cortex-M0+ core), where interrupts follow standard Cortex-M behavior.
 * When global interrupts are disabled, any pending interrupt—caused by an event like a pin change—latches in the 
 * NVIC's interrupt pending register. Enabling interrupts then triggers the ISR immediately if its priority allows, 
 * as the pending bit remains set until cleared in the handler.
 * 
 * MCCI_LoRaWAN_LMIC_library/src/lmic/config.h
 * // Enable this to use interrupt handler routines listening for RISING signals.
 * // Otherwise, the library polls digital input DIO pins via hal_io_check() for broader pin 
 * // compatibility but less precise timing.
 * //#define LMIC_USE_INTERRUPTS
 * 
 * Interrupts are disabled around SPI transactions to radio (e.g., in radio_irq_handler(), opcode execution) to 
 * prevent jitter from other ISRs. They're re-enabled after, allowing os_runloop_once() to process events 
 * without blocking user code.

 * In MCCI_LoRaWAN_LMIC_library/src/hal/hal.cpp
 * Finction lmic_hal_enableIRQs () has
 *
 * #if !defined(LMIC_USE_INTERRUPTS)
        // Instead of using proper interrupts (which are a bit tricky
        // and/or not available on all pins on AVR), just poll the pin
        // values. Since os_runloop disables and re-enables interrupts,
        // putting this here makes sure we check at least once every
        // loop.
        //
        // As an additional bonus, this prevents the can of worms that
        // we would otherwise get for running SPI transfers inside ISRs.
        // We merely collect the edges and timestamps here; we wait for
        // a call to lmic_hal_processPendingIRQs() before dispatching.
        lmic_hal_pollPendingIRQs_helper();
 * #endif
 * 
 * While in SD routines that use SPI bus avoid calling os_runloop() or Background() functions.
 * This might also use the SPI bus.
 * 
 * Below is not being used. I think this is an old flawed direction - rjb
 * 
 * The noInterrupts() and interrupts() calls in hal.cpp are part of the LMIC library's interrupt disable/enable functions 
 * (hal_disableIRQs() and hal_enableIRQs()). These protect brief, critical sections during LoRa radio operations like TX/RX, 
 * typically lasting microseconds on the SAMD21 (Feather M0's MCU).​ Wind speed and rain gauge interrupts are low-frequency 
 * mechanical events (e.g., <100 Hz for gusts, <1 Hz for rain tips). If one occurs during the rare disable window, 
 * the NVIC queues it automatically, ensuring execution once interrupts re-enable.
 * 
 * Modified Library - SEE https://github.com/mcci-catena/arduino-lmic?tab=readme-ov-file
 * LoRa radio is being polled, no need to mask interrupts. That would cause the loss of rain and wind speed interrupts. 
 * Comment out interupts in the code file MCCI_LoRaWAN_LMIC_library/src/hal/hal.cpp
 * 
 * void hal_disableIRQs () {
 *    //noInterrupts();    "./src/hal/hal.cpp" line 340  COMMENT OUT
 *   irqlevel++;
 * }
 *
 * void hal_enableIRQs () {
 *   if(--irqlevel == 0) {
 *   //   interrupts();    "./src/hal/hal.cpp" line 346  COMMENT OUT
 * ======================================================================================================================
 */

/*
LMIC.opmode Bit Flags - All Possible Values:

| Bit Flag      | Value    | Hex     | Meaning                    |
|---------------|----------|---------|----------------------------|
| OP_INIT       | 0x01     | 0x01    | LMIC initialized           |
| OP_PERIODIC   | 0x02     | 0x02    | Periodic timer running     |
| OP_JOINING    | 0x04     | 0x04    | Join in progress           |
| OP_SCANNING   | 0x08     | 0x08    | Scanning channels          |
| OP_TXDATA     | 0x10     | 0x10    | TX data queued             |
| OP_POLL       | 0x20     | 0x20    | Poll request pending       |
| OP_REJOIN     | 0x40     | 0x40    | Rejoin requested           |
| OP_TXRXPEND   | 0x80     | 0x80    | TX/RX transaction active   |
| OP_NEXTCHNL   | 0x100    | 0x100   | Next channel pending       |
| OP_LINKDEAD   | 0x200    | 0x200   | Link check failed          |
| OP_RXLISTEN   | 0x400    | 0x400   | RX listen mode             |
| OP_TXMODE     | 0x80000000 | 0x80000000 | TX mode active (32-bit) |

MOST IMPORTANT FOR SENDING:
if (LMIC.opmode & OP_TXRXPEND) { // Don't send }
*/


#define LORA_OTAA  0
#define LORA_ABP   1
#define LORA_WIFI_SS 8 // We need to set this pin high to disable LoRa / Wifi, 
                       // prior to accessing the SD card.
#define LORA_TRANSMIT_GAP 30 // Used to force a 30 second gap between LoRaWAN transmissions

// Extern variables
extern bool LW_valid;

// Function prototypes
const char* LW_Region();
void LW_WaitBetween(int wait_seconds);
bool LW_WaitOnTXRXPEND(int wait_seconds);
bool LW_Send(char *obs);
bool LW_isJoined();
bool LW_NeedToJoin();
void LW_initialize();
