/**
 *  @file     main_client.c
 *  @brief    Example of the PUSopen(R) application.
 *
 *  Example usage of PUS 17.
 *
 *  This code shall only be used in examples of PUSopen(R) usage.
 *
 *  @copyright
 *
 *  Copyright (c) 2019, 12G Flight Systems Sweden AB.
 *
 *  This program is the property of 12G Flight Systems Sweden AB.
 *  The right to copy, distribute, modify or otherwise make use
 *  of this software may be licensed only pursuant to the terms
 *  of agreement obtained from 12G Flight Systems Sweden AB.
 *
 */

/**
To compile GS mdb:
../../bin/poconfig.exe mdb_gs.xml mdb_gs.c

To compile GS:
gcc -o gc -I../../Includes -I../common -D_LINUB1804_GCC750 main_gc.c mdb_gc.c subnetwork.c ../common/clientserver.c -L../../lib -lpusopen
**/
//-- Includes -- 

#include <stdio.h>
#include "clientserver.h"

#include "pusopen.h"

// Stop flag for main application loop 
uint8_t stop = 0U;


// PUSopen(R) callback for telemetry (TM) reception 
po_result_t pususr_tm(po_tmdesc_t * tm)
{
    uint16_t i = 0U;

    printf("  TM %2u %2u Dest APID:%2u Flags:%u Count:%5u Payload (%2u bytes)  ",
           tm->service, tm->subservice, tm->destid, tm->pktSeqFlags, tm->pktSeqCount, tm->dataLen);

    for (i = 0U; i < tm->dataLen; i++) {
        printf("%02x ", tm->data[i]);
    }

    /*/ Break when [17, 4] received
    if (tm->service == 17U && tm->subservice == 4U) {
        stop = 1U;    //  Break the main application loop
    }
    //*/ 

    printf("\n");
    return PO_SUCCESS;
}

int main(int argc, const char* argv[])
{
    // Received byte 
    uint8_t data;

    // Transmission buffer 
    uint8_t buf[128];
    uint16_t len;

    // Info 
    printf("PUSopen(R) Example: PUS 17 (client)\n");
    printf("Copyright (C) 2019, 12G Flight Systems Sweden AB\n");
    printf("APID: %u\n\n", CURRENT_APID);

    // Open socket 
    server();

    // Initialize PUSopen(R) stack 
    po_initPus1();
    po_initPusUsr();
    po_initPs();
    po_initFess();

    // Send TC[17,1] to server 
    po_sendTc(17U, 1U,   // TC[17,1] 
              &data,     // TC payload 
              0U,        // TC payload length 
              1U);       // Destination APID 

    // Retrieve created TC[17,1] from stack and send it 
    (void) po_frame(buf, &len);
    server_send((char*)buf, (int)len);

    sleep_ms(100);
    //*/

    /*/
    // Send TC[17,3] to server 
    po_sendTc(17U, 3U,   // TC[17,1] 
              &data,     // TC payload 
              0U,        // TC payload length 
              1U);       // Destination APID 

    // Retrieve created TC[17,3] from stack and send it 
    (void) po_frame(buf, &len);
    server_send((char*)buf, (int)len);
    //*/

    sleep_ms(100);

    /*/ Function TC[8,1] 
    po_sendTc(8U, 1U,   // TC[8,1] 
              &data,    // TC payload 
              4U,       // TC payload length 
              1U);      // Destination APID 

    // Retrieve created TC[8,1] byte stream from PUSopen(R) stack and send it 
    (void) po_frame(buf, &len);

    server_send((char*)buf, (int)len);
    //*/

    // Main loop - wait for reception of TM[17,2] and TM[17,4] from server 
    while (stop == 0U) {

        // Push received data byte-by-byte into PUSopen(R) stack 
        while (client_recv(&data, 1) == 1) {
            po_accept(data);
        }

        // Trigger stack to process received TM[17,x] 

        // Unwrap TM[17,x] from received CCSDS packet 
        po_triggerPs();

        // Forward TM[17,x] to PUS User 
        po_triggerPus1();

        sleep_ms(100);
    }

    client_close();
    return 0;
}