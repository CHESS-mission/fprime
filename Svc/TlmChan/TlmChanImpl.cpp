/**
 * \file
 * \author T. Canham
 * \brief Implementation file for channelized telemetry storage component
 *
 * \copyright
 * Copyright 2009-2015, by the California Institute of Technology.
 * ALL RIGHTS RESERVED.  United States Government Sponsorship
 * acknowledged.
 * <br /><br />
 */
#include <Svc/TlmChan/TlmChanImpl.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Tlm/TlmPacket.hpp>
#include <Fw/Buffer/Buffer.hpp>
#include "../../Lib/mdb/hk_param.h"

#include <cstring>
#include <stdio.h>

#ifndef _GDS        // for VS synthax highlighting
//#define _PUS        // will cause compiler warning for redifinition
#endif

#ifdef _PUS
#include <Os/Mutex.hpp>

#include "pusopen.h"

extern Os::Mutex PO_STACK_MUTEX;

extern s_PARAM PARAM;

#endif  // defined _PUS

namespace Svc {

    TlmChanImpl::TlmChanImpl(const char* name) : TlmChanComponentBase(name)
    {   
        U16 i = 0;

        // clear data
        this->m_activeBuffer = 0;
        // clear slot pointers
        for (NATIVE_UINT_TYPE entry = 0; entry < TLMCHAN_NUM_TLM_HASH_SLOTS; entry++) {
            this->m_tlmEntries[0].slots[entry] = 0;
            this->m_tlmEntries[1].slots[entry] = 0;
        }

        // clear buckets
        for (NATIVE_UINT_TYPE entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
            this->m_tlmEntries[0].buckets[entry].used = false;
            this->m_tlmEntries[0].buckets[entry].updated = false;
            this->m_tlmEntries[0].buckets[entry].bucketNo = entry;
            this->m_tlmEntries[0].buckets[entry].next = 0;
            this->m_tlmEntries[0].buckets[entry].id = 0;
            this->m_tlmEntries[1].buckets[entry].used = false;
            this->m_tlmEntries[1].buckets[entry].updated = false;
            this->m_tlmEntries[1].buckets[entry].bucketNo = entry;
            this->m_tlmEntries[1].buckets[entry].next = 0;
            this->m_tlmEntries[1].buckets[entry].id = 0;
        }
        
        // clear free index
        this->m_tlmEntries[0].free = 0;
        this->m_tlmEntries[1].free = 0;

#ifdef _PUS
        // Set global PUS variable with default value
        // @todo - Find a clean way to deal with PARAM struct - Dirty code :o
        // Works only with U32 parameters list !!
        for(i = 0; i < PO_PARAM_SIZE; i++) {
            U32* ptr = reinterpret_cast<U32*>(&PARAM);
            ptr = ptr + i;
            *ptr = (U32)0; 
        }
#endif
    }

    TlmChanImpl::~TlmChanImpl() {}

    void TlmChanImpl::init(
            NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
            NATIVE_INT_TYPE instance /*!< The instance number*/
            ) {
        TlmChanComponentBase::init(queueDepth,instance);
    }

    NATIVE_UINT_TYPE TlmChanImpl::doHash(FwChanIdType id) {
        return (id % TLMCHAN_HASH_MOD_VALUE)%TLMCHAN_NUM_TLM_HASH_SLOTS;
    }

    void TlmChanImpl::pingIn_handler(
          const NATIVE_INT_TYPE portNum,
          U32 key
    ) {
        // return key
        this->pingOut_out(0,key);
    }

    void TlmChanImpl::Run_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) {

    // Only write packets if connected
    if (not this->isConnected_PktSend_OutputPort(0)) {
        return;
    }

#ifdef _GDS 

        // lock mutex long enough to modify active telemetry buffer
        // so the data can be read without worrying about updates
        this->lock();
        this->m_activeBuffer = 1 - this->m_activeBuffer;    // activeBuffer is 0 or 1
        // set activeBuffer to not updated
        for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
            this->m_tlmEntries[this->m_activeBuffer].buckets[entry].updated = false;
        }
        this->unLock();

        // go through each entry and send a packet if it has been updated
        for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
            TlmEntry* p_entry = &this->m_tlmEntries[1-this->m_activeBuffer].buckets[entry];
            if ((p_entry->updated) && (p_entry->used)) {
                this->m_tlmPacket.setId(p_entry->id);
                this->m_tlmPacket.setTimeTag(p_entry->lastUpdate);
                this->m_tlmPacket.setTlmBuffer(p_entry->buffer);
                this->m_comBuffer.resetSer();
                Fw::SerializeStatus stat = this->m_tlmPacket.serialize(this->m_comBuffer);
                FW_ASSERT(Fw::FW_SERIALIZE_OK == stat,static_cast<NATIVE_INT_TYPE>(stat));
                p_entry->updated = false;

                this->PktSend_out(0,this->m_comBuffer,0);
            }
        }

#elif defined _PUS
        // F' variables
        U32 tlmVal;

        
        // PUSOpen variables
        U8 po_buf[4096];    // @todo use definition
        U16 po_len;
        po_result_t po_res = PO_ERR;

        // Trigger PUS service 3 to check is report
        // has to generated
        po_triggerPus3();

        PO_STACK_MUTEX.lock();

        po_res = po_frame(po_buf, &po_len);

        PO_STACK_MUTEX.unLock();

        if(po_len > 0) {
            // @todo Remove - Temporary
            if(po_len > FW_COM_BUFFER_MAX_SIZE) {
                printf("=== [PUS3] TLM - To long frame (%u)- Not send\n", po_len);
                return;
            }
            FW_ASSERT(po_len <= FW_COM_BUFFER_MAX_SIZE);
            printf("TLM: Send report (%u)\n", po_len);
            Fw::ComBuffer m_comBuffer(po_buf, po_len);  //!< Com buffer for sending event buffers
            this->PktSend_out(0,m_comBuffer,0);
        }
#endif   

    }
    void TlmChanImpl::TlmRecv_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time &timeTag, Fw::TlmBuffer &val) {
        // Compute index for entry
        NATIVE_UINT_TYPE index = this->doHash(id);
        TlmEntry* entryToUse = 0;
        TlmEntry* prevEntry = 0;

        // Search to see if channel has already been stored or a bucket needs to be added
        if (this->m_tlmEntries[this->m_activeBuffer].slots[index]) {
            entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
            for (NATIVE_UINT_TYPE bucket = 0; bucket < TLMCHAN_HASH_BUCKETS; bucket++) {
                if (entryToUse) {
                    if (entryToUse->id == id) { // found the matching entry
                        break;
                    } else { // try next entry
                        prevEntry = entryToUse;
                        entryToUse = entryToUse->next;
                    }
                } else {
                    // Make sure that we haven't run out of buckets
                    FW_ASSERT(this->m_tlmEntries[this->m_activeBuffer].free < TLMCHAN_HASH_BUCKETS);
                    // add new bucket from free list
                    entryToUse = &this->m_tlmEntries[this->m_activeBuffer].buckets[this->m_tlmEntries[this->m_activeBuffer].free++];
                    prevEntry->next = entryToUse;
                    // clear next pointer
                    entryToUse->next = 0;
                    break;
                }
            }
        } else {
            // Make sure that we haven't run out of buckets
            FW_ASSERT(this->m_tlmEntries[this->m_activeBuffer].free < TLMCHAN_HASH_BUCKETS);
            // create new entry at slot head
            this->m_tlmEntries[this->m_activeBuffer].slots[index] = &this->m_tlmEntries[this->m_activeBuffer].buckets[this->m_tlmEntries[this->m_activeBuffer].free++];
            entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
            entryToUse->next = 0;
        }

        // copy into entry
        FW_ASSERT(entryToUse);
        entryToUse->used = true;
        entryToUse->id = id;
        entryToUse->updated = true;
        entryToUse->lastUpdate = timeTag;
        entryToUse->buffer = val;

#ifdef _PUS
        // List ids used in report #1 for PUS3 service 
        U32 used_hk[] = {0x29, 0x79, 0x7a, 0x169, 0x1E1, 0x42f, 0x430, 0x431};

        // Check received TM and update global variable for PUSOpen lib if
        // id is used in report
        // @todo - Find a clean way to deal with hk_param - Dirty code :o
        // Works only with U32 parameters list !!
        U16 i;
        for(i = 0; i < PO_PARAM_SIZE; i++) {
            if (used_hk[i] == id) {
                U32 value;
                val.deserialize(value);
                U32* ptr = reinterpret_cast<U32*>(&PARAM);
                ptr = ptr + i;
                *ptr = value;
                // printf("Tlm %u val %u\n", id, value);
            }
        }
#endif // defined _PUS

    }

    void TlmChanImpl::TlmGet_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time &timeTag, Fw::TlmBuffer &val) {
        // Compute index for entry
        NATIVE_UINT_TYPE index = this->doHash(id);

        // Search to see if channel has been stored
        TlmEntry *entryToUse = this->m_tlmEntries[this->m_activeBuffer].slots[index];
        for (NATIVE_UINT_TYPE bucket = 0; bucket < TLMCHAN_HASH_BUCKETS; bucket++) {
            if (entryToUse) { // If bucket exists, check id
                if (entryToUse->id == id) {
                    break;
                } else { // otherwise go to next bucket
                    entryToUse = entryToUse->next;
                }
            } else { // no buckets left to search
                break;
            }
        }

        if (entryToUse) {
            val  = entryToUse->buffer;
            timeTag = entryToUse->lastUpdate;
        } else { // requested entry may not be written yet; empty buffer
            val.resetSer();
        }

    }
}
