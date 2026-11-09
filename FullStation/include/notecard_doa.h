/*
 * ======================================================================================================================
 *  notecard_doa.h - Notecard DO Action Definations
 * ======================================================================================================================
 */

/*
    At Notehub web interface
        |
        | Add a JSON Note to data.qi for a specific device, for example {"doaction":"INFO"}
        v
    Notehub
        |
        | Synchronizes over the Notecard's network connection
        v
    Notecard — stores the incoming Note locally
        |
        | Your firmware reads it over I²C or serial
        v
    Your microcontroller — validates the command and performs the action
*/

#include <Notecard.h>

// Extern variables
void notecard_doa_check();