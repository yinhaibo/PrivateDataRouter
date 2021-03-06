//---------------------------------------------------------------------------

#ifndef UCommH
#define UCommH
#include <system.hpp>

// Work mode
// Only support Serial port and tcp client mode
enum WorkMode{
    WORK_MODE_SERIAL,
    WORK_MODE_TCP_CLIENT,
};

////////////////////////////////////////////////////////////////

//Distribution Type
typedef enum _error_mode_t
{
    UNIFORM_DISTRIBUTION,
    POISSON_DISTRIBUTION,
    NO_ERROR_DISTRIBUTION,
}error_mode_t;

AnsiString GetDistributionDesc(error_mode_t distri);
error_mode_t GetDistributionFromDesc(AnsiString desc);

// Message count structure
typedef struct _device_count_t{
    unsigned int val;
    unsigned int min;
    unsigned int max;
    float        avg;
}device_count_t;
#define MIN_COUNT_VALUE 0
#define MAX_COUNT_VALUE 0xFFFFFFFF

////////////////////////////////////////////////////////////////
// Device parameter configure structure
class device_config_t{
public:
    // Message configure
    unsigned char seq;      // Device seqence, just for list
    AnsiString alias;       // Device seqence, just for reading
    WorkMode mode;          // Work mode, Can be Serial port or TCP mode
    AnsiString configure;   // Work parameter configure
    unsigned int delayFrom; // Message sending delay parameter range
    unsigned int delayTo ;  // Message sending delay parameter range

    // Message structure define
    AnsiString head;        // Message Head, Using Hex, only 2 bytes.
    unsigned char tag;      // Message tag, for indentified message which is
                            // sending by self.
    AnsiString message;     // Message content for sending.
    AnsiString eofMessage;  // EOF Message for endinag a message transmission.

    // For count
    unsigned int sendSeq;
    unsigned long sendTick;
    // Message count, one success message including send a message and eof message
    // and make sure the message have transfer correctly.
    device_count_t dcResendCnt;  // the count of resend of message, min, max, avg
    device_count_t dcRespTime; // the response time of message, min, max, avg
    unsigned long msgMsgSent; // the count of message
    unsigned long msgTxCnt;  // the count of message have sent
    unsigned long msgRxCnt;  // the count of message have received
    unsigned long msgErrCnt; // The count of error message

    device_config_t(){
        sendSeq = 0;
        sendTick = 0;
        
        dcResendCnt.min = MAX_COUNT_VALUE;
        dcResendCnt.max = MIN_COUNT_VALUE;
        dcResendCnt.avg = 0.0f;
        dcResendCnt.val = 0;

        dcRespTime.min = MAX_COUNT_VALUE;
        dcRespTime.max = MIN_COUNT_VALUE;
        dcRespTime.avg = 0.0f;
        dcRespTime.val = 0;

        msgMsgSent = 0;
        msgTxCnt = 0;
        msgRxCnt = 0;
        msgErrCnt = 0;
    }
};


//---------------------------------------------------------------------------
// Message define
#define MAX_MESSAGE_LEN 10
#define MAX_RAW_BUFFER_SIZE 1024
// USING ONE BYTE ALIGNMENT
#pragma pack(1)
typedef struct _timestamp_t{
    unsigned short year;
    unsigned char  mon;
    unsigned char  day;
    unsigned char  hour;
    unsigned char  min;
    unsigned char  second;
    unsigned short millisec;
}timestamp_t;  //sizeof(timestamp_t) => 9
typedef struct _message_t{
    unsigned short head;            // Message head
    unsigned short len;             // Message length
    unsigned char  tag;             // Message tag, for indentified message which is
                                    // sending by self.
    unsigned int   seq;             // Sequence of message
    timestamp_t   timestamp;        // Timestamp of message
    unsigned char clen;             // Message content length
    char content[MAX_MESSAGE_LEN];  // Message content
    unsigned short    crc16;        // CRC16 of the message from seq to content
}message_t;
#pragma pack()

////////////////////////////////////////////////////////////////
// Macro

// Get message length from a message type
#define MESSAGELEN(m) (offsetof(message_t, content) + (m)->clen + 2)

// m = Message, h = head, t = Tag, s = Seq, c = Content
#define CREATE_MESSAGE(m, h, t, s, c) \
    do{ \
        (m).head = TextToUINT16(h); \
        (m).tag = t; \
        (m).seq = s; \
        SYSTEMTIME systm; \
        GetLocalTime(&systm); \
        (m).timestamp.year = systm.wYear; \
        (m).timestamp.mon  = systm.wMonth; \
        (m).timestamp.day  = systm.wDay; \
        (m).timestamp.hour = systm.wHour; \
        (m).timestamp.min  = systm.wMinute; \
        (m).timestamp.second = systm.wSecond; \
        (m).timestamp.millisec = systm.wMilliseconds; \
        (m).clen = AsciiToStream(c, (m).content, MAX_MESSAGE_LEN); \
        (m).len = MESSAGELEN(&m); \
    }while(0)

//---------------------------------------------------------------------------

WorkMode GetModeFromStr(AnsiString modeStr);
AnsiString GetModeStr(WorkMode mode);
void GetSerialConfigFromStr(const AnsiString& configure,
    int& portidx,
    int& baud,
    char& parity,
    int& databits,
    float& stopbits);
void GetTCPClientConfigFromStr(const AnsiString& configure,
    AnsiString& ip,
    int& port);
int TextToStream(AnsiString text, UINT8 *pStream, int szLen);
//---------------------------------------------------------------------------
#endif
