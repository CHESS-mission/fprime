// ====================================================================== 
// \title  GroundInterface.cpp
// \author lestarch
// \brief  cpp file for GroundInterface component implementation class
// 
// Define _PUS or _GDS depending on GS/protocols needs. 
// Currently set in App/CmakeLists.txt
// ====================================================================== 

#include <string.h>

#include <Fw/Com/ComPacket.hpp>
#include <Fw/Log/LogPacket.hpp>
#include <Svc/GroundInterface/GroundInterface.hpp>
#include <Svc/GroundInterface/GroundInterfaceCfg.hpp>
#include "Fw/Types/BasicTypes.hpp"

#include <Os/Log.hpp>

#include "pusopen.h"

namespace Svc {

const U32 GroundInterfaceComponentImpl::MAX_DATA_SIZE = 2048;
const TOKEN_TYPE GroundInterfaceComponentImpl::START_WORD = static_cast<TOKEN_TYPE>(0xdeadbeef);
const U32 GroundInterfaceComponentImpl::END_WORD = static_cast<U32>(0xcafecafe);

#ifdef __cplusplus
extern "C" {
#endif
U32 PR_NumPings = 0;
#ifdef __cplusplus
}
#endif



// ------------------------------------- ---------------------------------
// Construction, initialization, and destruction 
// ----------------------------------------------------------------------

GroundInterfaceComponentImpl ::
GroundInterfaceComponentImpl(
    const char *const compName
) : GroundInterfaceComponentBase(compName),
    m_ext_buffer(m_buffer, GND_BUFFER_SIZE),
    m_data_size(0),
    m_in_ring(m_in_buffer, GND_BUFFER_SIZE) {
}

void GroundInterfaceComponentImpl::init(const NATIVE_INT_TYPE instance) {
    GroundInterfaceComponentBase::init(instance);
    po_result_t res = PO_SUCCESS;

    res = po_initPus1();    // macro for pus1_reset(PO_DEF_PUS1)
    res |= po_initPs();     // macro for ps_reset(PO_DEF_PS)
    res |= po_initFess();   // macro for fess_reset(PO_DEF_FESS)

    if (res != PO_SUCCESS) {
        // PUSOpen initialisation error ! Critical error with FSW
        FW_ASSERT(0);
    }
}

GroundInterfaceComponentImpl::~GroundInterfaceComponentImpl(void) {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void GroundInterfaceComponentImpl::downlinkPort_handler(
    const NATIVE_INT_TYPE portNum,
    Fw::ComBuffer &data,
    U32 context
) {
#if defined _GDS
    // Downlink TM disabled
    FW_ASSERT(data.getBuffLength() <= MAX_DATA_SIZE);
    frame_send(data.getBuffAddr(), data.getBuffLength());
#endif
}

void GroundInterfaceComponentImpl::fileDownlinkBufferSendIn_handler(
    const NATIVE_INT_TYPE portNum,
    Fw::Buffer &fwBuffer
) {
#if defined _GDS
    FW_ASSERT(fwBuffer.getSize() <= MAX_DATA_SIZE);
    frame_send(fwBuffer.getData(), fwBuffer.getSize(), Fw::ComPacket::FW_PACKET_FILE);
    fileDownlinkBufferSendOut_out(0, fwBuffer);
#endif
}

void GroundInterfaceComponentImpl::readCallback_handler(
    const NATIVE_INT_TYPE portNum,
    Fw::Buffer &buffer
) {
    if(isConnected_readPoll_OutputPort(0)) {
        // User should chose to use callback or poll method for uplink data
        FW_ASSERT(0);
    }

#if defined _PUS
    processPUS(buffer);
#elif defined _GDS
    processBuffer(buffer);
#endif
}

void GroundInterfaceComponentImpl::eventReport_handler(
        const NATIVE_INT_TYPE portNum,
        FwEventIdType id,
        Fw::Time &timeTag,
        Fw::LogSeverity severity,
        Fw::LogBuffer &args
) {
    // F' variables
    Fw::Buffer buffer;          //!< buffer to send frame to SocketIpDriver

    //Fw::LogPacket m_logPacket;  //!< Packet buffer for assembling log packets
    //Fw::ComBuffer m_comBuffer;  //!< Com buffer for sending event buffers

    // PUSOpen variables
    U8 po_buf[512];
    U8 po_evtData = 0;
    U16 po_len;
    po_result_t po_res = PO_ERR;
    pus5_evtId_t po_pus5_eventId = PUS5_EVT_HIGH;   // default value

    printf("[PUS] Event received : %u (0x%02X)\n", id, id);

    /**
    ---- PUS Service 5 severity level ----
    PUS5_EVT_INFO   = Information event TM[5,1]
    PUS5_EVT_LOW    = Low severity anomaly TM[5,2]
    PUS5_EVT_MEDIUM = Medium severity anomaly TM[5,3]
    PUS5_EVT_HIGH   = High severity anomaly TM[5,4]

    ---- F' Events severity level ----
    LOG_DIAGNOSTIC  → PUS5_EVT_INFO
    LOG_ACTIVITY_LO → PUS5_EVT_INFO
    LOG_COMMAND     → PUS5_EVT_INFO
    LOG_ACTIVITY_HI → PUS5_EVT_LOW
    LOG_WARNING_LO  → PUS5_EVT_MEDIUM
    LOG_ACTIVITY_HI → PUS5_EVT_HIGH
    LOG_FATAL       → PUS5_EVT_HIGH - internal handling in the future
    **/
    switch(severity) {
        case Fw::LOG_DIAGNOSTIC:
        case Fw::LOG_ACTIVITY_LO:
        case Fw::LOG_COMMAND:
            po_pus5_eventId = PUS5_EVT_INFO;
            break;
        case Fw::LOG_ACTIVITY_HI:
            po_pus5_eventId = PUS5_EVT_LOW;
            break;
        case Fw::LOG_WARNING_LO:
            po_pus5_eventId = PUS5_EVT_MEDIUM;
            break;
        case Fw::LOG_WARNING_HI:
            po_pus5_eventId = PUS5_EVT_HIGH;
            break;
        case Fw::LOG_FATAL:
            po_pus5_eventId = PUS5_EVT_HIGH;
            break;
        default:
            po_pus5_eventId = PUS5_EVT_HIGH;
    }

    /*/ Event arguments to serialize
    m_logPacket.setId(id);
    m_logPacket.setTimeTag(timeTag);
    m_logPacket.setLogBuffer(args);
    Fw::SerializeStatus stat = m_logPacket.serialize(m_comBuffer);
    FW_ASSERT(Fw::FW_SERIALIZE_OK == stat,static_cast<NATIVE_INT_TYPE>(stat));
    //*/
    // Sample - In the future serialize id and LogBuffer
    po_evtData = id;

    this->m_poStackMutex.lock();

    // Send TM[5,x] with F' event ID = x 
    po_res = po_pus5tm( po_pus5_eventId,    // event ID PUS[5, x]
                        &po_evtData,        // event data
                        GS_APID);           // destination APID (GS)

    if(po_res != PO_SUCCESS) {
        printf("[PUS] po error: %u\n", po_res);
        FW_ASSERT(0);
    }

    // Retrieve created TM[5,x] byte stream from PUSopen stack and send it
    po_res = po_frame(po_buf, &po_len);

    if(po_res != PO_SUCCESS) {
        printf("[PUS] po error: %u\n", po_res);
        FW_ASSERT(0);
    }

    this->m_poStackMutex.unLock();

    buffer.setData(po_buf);
    buffer.setSize(po_len);
    write_out(0, buffer);

}

void GroundInterfaceComponentImpl::hkReport_handler(
        const NATIVE_INT_TYPE portNum,
        FwChanIdType id,
        Fw::Time &timeTag,
        Fw::TlmBuffer &val
) {
    // F' variables
    Fw::Buffer buffer;
    U32 tlmVal;

    // PUSOpen variables
    U8 po_buf[512];
    U16 po_len;
    po_result_t po_res = PO_ERR;

    // printf("[PUS] Housekeeping received : %u (0x%02X)\n", id, id);

    this->m_poStackMutex.lock();

    // Sample - In the future serialize id and TlmBuffer
    switch(id) {
        case 0x29:  // (41) PR_NumPings
            val.deserialize(tlmVal);
            PR_NumPings = tlmVal;   
            printf("[PUS] Housekeeping PR_NumPings received : %u \n", tlmVal);
            // Trigger PUS 3 Service provider to generate TM[3,25]
            po_triggerPus3();
            break;
    }

    // Retrieve created TM[3,25] byte stream from PUSopen stack and send it
    po_res = po_frame(po_buf, &po_len);

    this->m_poStackMutex.unLock();

    // If a report has been generated, send it
    if(po_len > 0) {
        buffer.setData(po_buf);
        buffer.setSize(po_len);
        printf("[PUS] Send report\n");
        write_out(0, buffer); 
    }
}

void GroundInterfaceComponentImpl::schedIn_handler(
    const NATIVE_INT_TYPE portNum, /*!< The port number*/
    NATIVE_UINT_TYPE context /*!< The call order*/
) {
    // TODO: replace with a call to a buffer manager
    Fw::Buffer buffer = m_ext_buffer;
    // Call read poll if it is hooked up
    if (isConnected_readPoll_OutputPort(0)) {
        readPoll_out(0, buffer);
#if defined _PUS
    processPUS(buffer);
#elif defined _GDS
    processBuffer(buffer);
#endif
    }
}

void GroundInterfaceComponentImpl::frame_send(U8 *data, TOKEN_TYPE size, TOKEN_TYPE packet_type) {
    // TODO: replace with a call to a buffer manager
    Fw::Buffer buffer = m_ext_buffer;
    Fw::SerializeBufferBase& buffer_wrapper = buffer.getSerializeRepr();
    buffer_wrapper.resetSer();
    // True size is supplied size plus sizeof(TOKEN_TYPE) if a packet_type other than "UNKNOWN" was supplied.
    // This is because if not UNKNOWN, the packet_type is serialized too.  Otherwise it is assumed the PACKET_TYPE is
    // already the first token in the UNKNOWN typed buffer.
    U32 true_size = (packet_type != Fw::ComPacket::FW_PACKET_UNKNOWN) ? size + sizeof(TOKEN_TYPE) : size;
    // Frame format :  | START_WORD | data_size  | data  | END_WORD |
    U32 total_size = sizeof(TOKEN_TYPE) + sizeof(TOKEN_TYPE) + true_size + sizeof(U32);
    // Serialize data
    FW_ASSERT(GND_BUFFER_SIZE >= total_size, GND_BUFFER_SIZE, total_size);
    buffer_wrapper.serialize(START_WORD);
    buffer_wrapper.serialize(static_cast<TOKEN_TYPE>(true_size));
    // Explicitly set the packet type, if it didn't come with the data already
    if (packet_type != Fw::ComPacket::FW_PACKET_UNKNOWN) {
        buffer_wrapper.serialize(packet_type);
    }
    buffer_wrapper.serialize(data, size, true);
    buffer_wrapper.serialize(static_cast<TOKEN_TYPE>(END_WORD));

    // Setup for sending by truncating unused data
    buffer.setSize(buffer_wrapper.getBuffLength());
    FW_ASSERT(buffer.getSize() == total_size, buffer.getSize(), total_size);
    write_out(0, buffer);
}

void GroundInterfaceComponentImpl::routeComData() {
    // Read the packet type from the data buffer
    U32 packet_type = Fw::ComPacket::FW_PACKET_UNKNOWN;
    m_in_ring.peek(packet_type, HEADER_SIZE);

    // Process variable type
    switch (packet_type) {
        case Fw::ComPacket::FW_PACKET_COMMAND: {
            Fw::ComBuffer com;
            m_in_ring.peek(com.getBuffAddr(), m_data_size, HEADER_SIZE);
            // Reset com buffer for sending out data
            com.setBuffLen(m_data_size);
            uplinkPort_out(0, com, 0);      // → send to CommandDispatcher
            break;
        }
        case Fw::ComPacket::FW_PACKET_FILE: {
            // If file uplink is possible, handle files.  Otherwise ignore.
            FW_ASSERT(0);
            if (isConnected_fileUplinkBufferGet_OutputPort(0) &&
                isConnected_fileDownlinkBufferSendOut_OutputPort(0)) {
                Fw::Buffer buffer = fileUplinkBufferGet_out(0, m_data_size);
                m_in_ring.peek(buffer.getData(), m_data_size - sizeof(packet_type), HEADER_SIZE + sizeof(packet_type));
                buffer.setSize(m_data_size - sizeof(packet_type));
                fileUplinkBufferSendOut_out(0, buffer);
            }
            break;
        }
        default:
            return;
    }
}

void GroundInterfaceComponentImpl::processRing() {
    // Header items for the packet
    TOKEN_TYPE start;
    U32 checksum; //TODO: make this run a CRC32
    // Inner-loop, process ring buffer looking for at least the header
    while (m_in_ring.get_remaining_size() >= HEADER_SIZE) {
        m_data_size = 0;
        // Peek into the header and read out values
        Fw::SerializeStatus status = m_in_ring.peek(start, 0);
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        status = m_in_ring.peek(m_data_size, sizeof(TOKEN_TYPE));
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);
        // Check the header for correctness
        if (start != START_WORD || m_data_size >= MAX_DATA_SIZE) {
            m_in_ring.rotate(1);
            continue;
        }
        // Check for enough data to deserialize everything otherwise break and wait for more.
        else if (m_in_ring.get_remaining_size() < (HEADER_SIZE + m_data_size + sizeof(END_WORD))) {
            break;
        }
        // Continue with the data portion and checksum
        m_in_ring.peek(checksum, HEADER_SIZE + m_data_size);
        // Check checksum
        if (checksum == END_WORD) {
            routeComData();
            m_in_ring.rotate(HEADER_SIZE + m_data_size + sizeof(END_WORD));
        }
        // Failed checksum, keep looking for valid message
        else {
            m_in_ring.rotate(1);
        }
    }
    //*/
}

void GroundInterfaceComponentImpl::processBuffer(Fw::Buffer& buffer) {
    NATIVE_UINT_TYPE buffer_offset = 0;
    while (buffer_offset < buffer.getSize()) {
        NATIVE_UINT_TYPE ser_size = (buffer.getSize() >= m_in_ring.get_remaining_size(true)) ?
            m_in_ring.get_remaining_size(true) : static_cast<NATIVE_UINT_TYPE>(buffer.getSize());
        m_in_ring.serialize(buffer.getData() + buffer_offset, ser_size);
        buffer_offset = buffer_offset + ser_size;
        processRing();
    }
}

void GroundInterfaceComponentImpl::processPUS(Fw::Buffer& buffer) {
    Fw::Buffer extBuff = m_ext_buffer;

    // Transmission buffer
    U8 buf[128];
    U16 len;

    // printf("[PUS] Data received : %u\n", buffer.getSize());


    this->m_poStackMutex.lock();

    // Push received data byte-by-byte into PUSopen(R) stack
    for(int i = 0; i < buffer.getSize(); i++) {
        //printf("[PUS] %d data : %hhu \n",i,buffer.getData()[i]);
        po_accept(buffer.getData()[i]); // todo propre reinterpret_cast
    }

    // Unwrap TC[x,y] from received CCSDS packet
    po_triggerPs();

    // Forward TC[x,y] to PUS x
    po_triggerPus1();

    // Retrieve potentially created TM[17,x] byte stream from PUSopen(R) stack and send it
    po_frame(buf, &len);

    this->m_poStackMutex.unLock();

    if (len > 0) {
        extBuff.setSize(len);
        extBuff.setData(buf);
        write_out(0,extBuff);
    }
}


#ifdef __cplusplus
extern "C" {
#endif

/**
* User-defined function triggered by PUS 8 provider.
* This function is defined in the Mission Database
* mdb_server.xml under <UserFunctions>.
*/
po_result_t UserPus8Fn (uint8_t fid, uint8_t *data, uint16_t len)
{
    printf("[PUS] PUS8 User Function triggered.\n");
    printf("[PUS] Function ID = %u.\n", fid);
    printf("[PUS] Received data (len = %u): %u %u %u %u\n", len, data[0], data[1], data[2], data[3]);

    return PO_SUCCESS;
}

/* -- Global functions -- */
po_result_t subnet_request(
        uint8_t * const data,
        const uint16_t len,
        const po_apid_t apid,
        const uint16_t vcid) {
    po_result_t res = PO_SUCCESS;     /* Result of this function */

    (void)apid;
    (void)vcid;

    /* Forward requested data directly down to FESS */
    res = fessChannelAccess_request(PO_DEF_FESS, data, len);

    return res;
}

po_result_t subnet_indication(uint8_t * const data, uint16_t *len) {
    po_result_t res = PO_SUCCESS;  /* Result of this function */
    uint8_t quality = 0U;
    uint8_t sequence = 0U;

    /* Call FESS indication API to retrieve received data */
    res = fessChannelAccess_indication(PO_DEF_FESS, data, len, &quality, &sequence);

    return res;
}

#ifdef __cplusplus
}
#endif

} // end namespace Svc
