//---------------------------------------------------------------------------


#pragma hdrstop

#include "UComm.h"

//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#pragma package(smart_init)


device_config_t::device_config_t(){
    //sendSeq = 0;
    memset(stream, 0, MAX_MESSAGE_LEN);
    sendTick = 0;

    for (int i = 0; i < MAX_RETRANS_REC_CNT; i++){
        resendTotal[i] = 0;
    }
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

//---------------------------------------------------------------------------
static AnsiString SERIAL_WORK_MODE = "Serial port";
static AnsiString TCP_SERVER_WORK_MODE = "TCP Server";

WorkMode GetModeFromStr(AnsiString modeStr)
{
    if (modeStr == SERIAL_WORK_MODE){
        return WORK_MODE_SERIAL;
    }else if (modeStr == TCP_SERVER_WORK_MODE){
        return WORK_MODE_TCP_SERVER;
    }
    return WORK_MODE_SERIAL;
}

AnsiString GetModeStr(WorkMode mode)
{
    switch(mode){
    case WORK_MODE_SERIAL: return SERIAL_WORK_MODE;
    case WORK_MODE_TCP_SERVER: return TCP_SERVER_WORK_MODE;
    default: return TCP_SERVER_WORK_MODE;
    }
}

void GetSerialConfigFromStr(const AnsiString& confStr,
    int& portidx,
    int& baud,
    char& parity,
    int& databits,
    float& stopbits)
{
    char *configure = confStr.c_str();
    int conflen = confStr.Length();
    int seg = 1;
    int splitpos = 0;
    char portName[7] = {0}; //COM[1-999]
    for (int i = 0; i < conflen; i++){
        if (configure[i] == ','){
            switch(seg){
            case 1: //Port
                strncpy(portName, configure, i);
                break;
            case 2: // baud
                baud = atoi(configure + splitpos + 1);
                break;
            case 3: // Parity
                parity = configure[i-1];
                break;
            case 4: // Data bits
                databits = atoi(configure+i-1);
                break;
            }
            seg++;
            splitpos = i;
        }
    }
    // Stop bits
    stopbits = atof(configure+splitpos+1);
    portidx = atoi(portName+3);
}

void GetTCPClientConfigFromStr(const AnsiString& configure,
    AnsiString& ip,
    int& port)
{
    int splitpos = configure.AnsiPos(",");
    if (splitpos > 0){
        ip = configure.SubString(1, splitpos - 1);
        port = StrToInt(configure.SubString(splitpos+1, configure.Length()));
    }
}

AnsiString GetDistributionDesc(error_mode_t distri)
{
    switch(distri){
    case UNIFORM_DISTRIBUTION:
        return "Uniform";
    case POISSON_DISTRIBUTION:
        return "Poisson";
    case NO_ERROR_DISTRIBUTION:
    default:
        return "No Error";
    }
}

error_mode_t GetDistributionFromDesc(AnsiString desc)
{
    if (desc == "Uniform"){
        return UNIFORM_DISTRIBUTION;
    }else if(desc == "Poisson"){
        return POISSON_DISTRIBUTION;
    }else{
        return NO_ERROR_DISTRIBUTION;
    }
}

UINT8 HexToInt8(char hex)
{
        if (hex >= 'a' && hex <= 'f'){
        	hex = 'A' + (hex - 'a');
        }
        if (hex > '9'){
                return 10 + (hex - 'A');
        }else{
                return 9 - ('9' - hex);
        }
}

/* Trans A Hex String Into byte stream */
int TextToStream(AnsiString text, UINT8 *pStream, int szLen)
{
        int cnt = 0;
        char *pStr = text.c_str();
        bool bStart = false;
        UINT8 bValue = 0;
        if (!*pStr) return 0;
        do{
                if (*pStr == ' ' || *pStr == '\r' || *pStr == '\n'){
                        bStart = false;
                        continue;
                }
                if (bStart){
                        bValue = (bValue<<4) | HexToInt8(*pStr);
                        bStart = false;
                        *pStream++ = bValue;
                        cnt++;
                }else{
                        bValue = HexToInt8(*pStr);
                        bStart = true;
                }
        }while(*(++pStr) && cnt < szLen);

        return cnt;
}
