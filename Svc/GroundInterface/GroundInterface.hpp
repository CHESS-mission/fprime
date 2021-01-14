// ====================================================================== 
// \title  GroundInterfaceImpl.hpp
// \author lestarch
// \brief  hpp file for GroundInterface component implementation class
// ====================================================================== 
#ifndef GroundInterface_HPP
#define GroundInterface_HPP

#include <Fw/Types/Serializable.hpp>
#include <Os/Mutex.hpp>

#include "Svc/GroundInterface/GroundInterfaceComponentAc.hpp"
#include "Utils/Types/CircularBuffer.hpp"

#define GND_BUFFER_SIZE 1024
#define TOKEN_TYPE U32
#define HEADER_SIZE (2 * sizeof(TOKEN_TYPE))

namespace Svc {

  class GroundInterfaceComponentImpl :
    public GroundInterfaceComponentBase
  {
    public:
      static const U32 MAX_DATA_SIZE;
      static const TOKEN_TYPE START_WORD;
      static const U32 END_WORD;
      void processBuffer(Fw::Buffer& data /*!< Data to process */);
      // ----------------------------------------------------------------------
      // Construction, initialization, and destruction
      // ----------------------------------------------------------------------

      //! Construct object GroundInterface
      //!
      GroundInterfaceComponentImpl(
          const char *const compName /*!< The component name*/
      );

      //! Initialize object GroundInterface
      //!
      void init(
          const NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
      );

      //! Destroy object GroundInterface
      //!
      ~GroundInterfaceComponentImpl(void);

    PRIVATE:

      // ----------------------------------------------------------------------
      // Handler implementations for user-defined typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for downlinkPort
      //!
      void downlinkPort_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::ComBuffer &data, /*!< Buffer containing packet data*/
          U32 context /*!< Call context value; meaning chosen by user*/
      );

      //! Handler implementation for eventReport
      //!
      void eventReport_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwEventIdType id, /*!< Log ID*/
          Fw::Time &timeTag, /*!< Time Tag*/
          Fw::LogSeverity severity, /*!< The severity argument*/
          Fw::LogBuffer &args /*!< Buffer containing serialized log entry*/
      );

      //! Handler implementation for hkReport
      //!
      void hkReport_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          FwChanIdType id, /*!< Telemetry Channel ID*/
          Fw::Time &timeTag, /*!< Time Tag*/
          Fw::TlmBuffer &val /*!< Buffer containing serialized telemetry value*/
      );

      //! Handler implementation for fileDownlinkBufferSendIn
      //!
      void fileDownlinkBufferSendIn_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::Buffer &fwBuffer 
      );

      //! Handler implementation for readCallback
      //!
      void readCallback_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          Fw::Buffer &fwBuffer 
      );

      //! Handler implementation for schedIn
      //!
      void schedIn_handler(
          const NATIVE_INT_TYPE portNum, /*!< The port number*/
          NATIVE_UINT_TYPE context /*!< The call order*/
      );
      //! Frame and send some data
      //!
      void frame_send(
          U8* data, /*!< Data to be framed and sent out */
          TOKEN_TYPE size, /*!< Size of data in typed format */
          TOKEN_TYPE packet_type = Fw::ComPacket::FW_PACKET_UNKNOWN /*!< Packet type override for anoymous data i.e. file downlink */
      );

      //! Processes the out-going data into coms order
      void routeComData();

      //! Process all the data in the ring
      void processRing();

      //! Process a data buffer containing a read from the serial port
      void processPUS(Fw::Buffer& data /*!< Data to process */);

      // Basic data movement variables
      Fw::Buffer m_ext_buffer;
      U8 m_buffer[GND_BUFFER_SIZE];

      // Input variables
      TOKEN_TYPE m_data_size; //!< Data size expected in incoming data
      U8 m_in_buffer[GND_BUFFER_SIZE];
      Types::CircularBuffer m_in_ring;

        Os::Mutex m_poStackMutex; /*!< Protect access to PUSOpen stack */
    };

} // end namespace Svc

#endif
