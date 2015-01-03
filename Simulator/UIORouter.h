//---------------------------------------------------------------------------

#ifndef UIORouterH
#define UIORouterH
#include <system.hpp>
////////////////////////////////////////////////////////////////
// Work mode
// Only support Serial port and tcp client mode
enum WorkMode{
    WORK_MODE_SERIAL,       // Serial port
    WORK_MODE_TCP_CLIENT,   // TCP network port
};

////////////////////////////////////////////////////////////////
// Device parameter configure structure
typedef struct _device_config_t{
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

    // Message count, one success message including send a message and eof message
    // and make sure the message have transfer correctly.
    unsigned long msgReqCnt;  // Request send count
    unsigned long msgSuccCnt; // The count of success to send a whole message
    unsigned long msgRespCnt; // The count of response to another side
}device_config_t;

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
    unsigned char  seq;             // Sequence of message
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
#endif
