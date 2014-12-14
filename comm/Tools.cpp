#include <vcl.h>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <TabNotBk.hpp>
#include <Tabs.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <ADODB.hpp>
#include <DB.hpp>
#include <msxmldom.hpp>
#include <XMLDoc.hpp>
#include <xmldom.hpp>
#include <XMLIntf.hpp>
#include <jpeg.hpp>
#include <Registry.hpp>
#include <Math.hpp>

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#pragma hdrstop

#include "Tools.h"


// ----------------------For CRC Calc---------------------------
static unsigned char auchCRCHi[] ={
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40} ;
/*********************************************/
static unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 
0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 
0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 
0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 
0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 
0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 
0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40 } ; 

//---------------------------------------------------------------------------

#pragma package(smart_init)

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

char Int4ToHex(UINT8 value)
{
        if (value > 9){
                return 'A' + (value - 10);
        }else{
                return '0' + value;
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

int AsciiToStream(AnsiString text, UINT8 *pStream, int szLen)
{
        char *pStr = text.c_str();
        if (!*pStr) return 0;
        int cpylen = text.Length();
        if (cpylen > szLen) cpylen = szLen;
        memcpy(pStream, pStr, cpylen + 1);

        return cpylen + 1;
}

AnsiString StreamToText(UINT8 *pStream, int szLen)
{
        AnsiString retstr = "";
        UINT8 *pData = pStream;
        char buff[13];
        int pos = 0;
        int idx = 0;
        while(pos < szLen)
        {
                buff[idx++] = Int4ToHex(*pData >> 4);
                buff[idx++] = Int4ToHex(*pData & 0x0F);
                buff[idx++] = ' ';
                if (idx == 12){
                        buff[idx] = '\0'; 
                        retstr += buff;
                        idx = 0;
                }
                pos++;
                pData++;
        }
        buff[idx] = '\0'; 
        retstr += buff;
        return retstr;
}

AnsiString StreamToAscii(UINT8 *pStream, int szLen)
{
        AnsiString retstr = "";
        UINT8 *pData = pStream;
        char buff[13];
        int pos = 0;
        int idx = 0;
        while(pos < szLen)
        {
                buff[idx++] = pData[pos++];
                if (idx == 12){
                        buff[idx] = '\0';
                        retstr += buff;
                        idx = 0;
                }
        }
        buff[idx] = '\0'; 
        retstr += buff;
        return retstr;        
}
UINT8  TextToUINT8(AnsiString text)
{
        UINT8 rv;
        TextToStream(text, &rv, 1);
        return rv;
}

UINT16 TextToUINT16(AnsiString text)
{
        UINT8 pStream[2];
        TextToStream(text, pStream, 2);
        return pStream[0] | (pStream[1]<< 8);
}
UINT32 TextToUINT32(AnsiString text)
{
        UINT8 pStream[4];
        TextToStream(text, pStream, 4);
        return pStream[0] | (pStream[1] << 8)
                | (pStream[2] << 16) | (pStream[3] << 24);
}

UINT8  HexToUINT8(AnsiString text)
{
        AnsiString temp = "00" + text;
        temp = temp.SubString(temp.Length() - 1,2);
        return TextToUINT8(temp);
}

UINT16 HexToUINT16(AnsiString text)
{
        AnsiString temp = "0000" + text;
        temp = temp.SubString(temp.Length() - 3,4);
	UINT8 pStream[2];
        TextToStream(temp, pStream, 2);
        return pStream[1] | (pStream[0]<< 8);
}

UINT32 HexToUINT32(AnsiString text)
{
	UINT8 pStream[4];
        TextToStream(text, pStream, 4);
        return pStream[3] | (pStream[2] << 8)
                | (pStream[1] << 16) | (pStream[0] << 24);
}

AnsiString UINT16ToHex(UINT16 value)
{
        UINT8 pStream[2];
        pStream[0] = (value & 0xFF);
        pStream[1] = (value >> 8) & 0xFF;
        return StreamToText(pStream, 2);
}

AnsiString UINT32ToHex(UINT32 value)
{
        UINT8 pStream[4];
        pStream[0] = (value & 0xFF);
        pStream[1] = (value >> 8) & 0xFF;
        pStream[2] = (value >> 16)& 0xFF;
        pStream[3] = (value >> 24)& 0xFF;
        return StreamToText(pStream, 4);
}

AnsiString UINT16ToHex4(UINT16 value)
{
        UINT8 pStream;
        char buf[3];
        AnsiString rv;

        buf[2] = '\0';
        pStream = (value >> 8) & 0xFF;
	buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        pStream = (value & 0xFF);
	buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        return rv;
}

AnsiString UINT32ToHex4(UINT32 value)
{
        UINT8 pStream;
        char buf[3];
        AnsiString rv;

        buf[2] = '\0';
        pStream = (value >> 24)& 0xFF;
        buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        pStream = (value >> 16)& 0xFF;
        buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        pStream = (value >> 8) & 0xFF;
        buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
	pStream = (value & 0xFF);
        buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        return rv;
}

AnsiString UINT8ToHex(UINT8 value)
{
	UINT8 pStream;
        char buf[3];
        AnsiString rv;

        buf[2] = '\0';
        pStream = (value & 0xFF);
        buf[0] = Int4ToHex(pStream >> 4);
        buf[1] = Int4ToHex(pStream & 0x0F);
        rv += buf;
        
        return rv;
}

UINT8 UINT8ToBCD(UINT8 value)
{
        UINT8 rv;
        if (value > 99) return 0;
        return (value % 10) | (value / 10) << 4;
}

UINT8 BCDToUINT8(UINT8 bcd)
{
        UINT8 rv;
        return (bcd & 0x0F) + ((bcd >> 4 & 0x0F) * 10);
}
UINT32 ASCIIsToHexs(UINT8 *dst, UINT32 dstSize, UINT8 *src, UINT32 srcLen)
{
        UINT32 resultLen = 0;
        srcLen = srcLen/2*2;//偶数

        while(resultLen <= (srcLen>>1))
        {
                if(resultLen == dstSize)//不能超过缓冲的大小
                        break;
                dst[resultLen] = Ascii2Hex(src[resultLen<<1]);
                dst[resultLen] = dst[resultLen]<<4;
                dst[resultLen] |= Ascii2Hex(src[(resultLen<<1)+1]);
                resultLen++;
        }

        return resultLen;
}
UINT8 Ascii2Hex(UINT8 ascii)
{
        if(ascii>='0' && ascii<='9')
                return ascii-'0';
        else if(ascii>='A' && ascii<='Z')
                return 10+ascii-'A';
        else if(ascii>='a' && ascii<='z')
                return 10+ascii-'a';
        else
                return 0;
}
UINT32 HexsToASCIIs(UINT8 *dst, UINT8 *src, UINT32 srcLen)
{
        UINT32 resultLen = 0;

        for(UINT32 i=0; i<srcLen; i++)
        {
                dst[resultLen] = Hex2Ascii(src[i]>>4);//先高4位
                dst[resultLen+1] = Hex2Ascii(src[i]);//再低4位
                resultLen += 2;
        }

        return resultLen;
}
UINT8 Hex2Ascii(UINT8 hex)
{
        char tab[] = "0123456789abcdef";

        return tab[hex&0x0F];
}


//求出StartDate和EndDate之间的时间差值，具体要求的部分由参数datepart指定。
int   __fastcall   DateDiff(TDatePart datePart,
	TDateTime StartDate, TDateTime EndDate)
{
	//变量声明
	TDateTime   SmallDate;
	TDateTime   LargeDate;

	unsigned   short   Hour,Min,Sec,MSec;
	unsigned   short   Hour1,Min1,Sec1,MSec1;

	int   Diff;
	int   i=0;   
    
	//获得具体的时间参数   
	StartDate.DecodeTime(&Hour,&Min,&Sec,&MSec);
	EndDate.DecodeTime(&Hour1,&Min1,&Sec1,&MSec1);
            
	switch(datePart)
	{
        case   dfYear://年
		if(StartDate>EndDate)
                          {
                                  SmallDate=StrToDate(EndDate.FormatString("YYYY")+"-01-01");   
                                  LargeDate=StrToDate(StartDate.FormatString("YYYY")+"-01-01");   
                          }   
                          else   
                          {   
                                  SmallDate=StrToDate(StartDate.FormatString("YYYY")+"-01-01");   
                                  LargeDate=StrToDate(EndDate.FormatString("YYYY")+"-01-01");   
                          }   
                          while(LargeDate!=IncMonth(SmallDate,i))   
                                  i+=12;   
                          if(StartDate>EndDate)   
                                  Diff=-i/12;   
                          else   
                                  Diff=i/12;   
                          break;   
	case   dfMonth://月
                          if(StartDate>EndDate)   
                          {   
                                  SmallDate=StrToDate(EndDate.FormatString("YYYY'-'MM")+"-01");   
                                  LargeDate=StrToDate(StartDate.FormatString("YYYY'-'MM")+"-01");   
                          }   
                          else   
                          {   
                                  SmallDate=StrToDate(StartDate.FormatString("YYYY'-'MM")+"-01");   
                                  LargeDate=StrToDate(EndDate.FormatString("YYYY'-'MM")+"-01");   
                          }   
                          while(LargeDate!=IncMonth(SmallDate,i))   
                                  i++;   
                          if(StartDate>EndDate)   
                                  Diff=-i;   
                          else   
                                  Diff=i;   
                          break;   
	case   dfWeek://星期
                          SmallDate=StartDate;   
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          Diff=Diff/7;   
                          break;   
	case   dfDay://日
                          SmallDate=StartDate;   
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          break;   
	case   dfHour://小时
                          SmallDate=StartDate;   
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          Diff=Diff   *   24   +Hour1-Hour;   
                          break;   
	case   dfMinute://分钟
                          SmallDate=StartDate;   
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          Diff=Diff   *   24   +Hour1-Hour;   
                          Diff=Diff   *   60   +Min1-Min;   
                          break;   
	case   dfSecond://秒
                          SmallDate=StartDate;
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          Diff=Diff   *   24   +Hour1-Hour;   
                          Diff=Diff   *   60   +Min1-Min;   
                          Diff=Diff   *   60   +Sec1-Sec;   
                          break;   
	case   dfMilliSecond://毫秒
                          SmallDate=StartDate;
                          ReplaceTime(SmallDate,0);   
                          LargeDate=EndDate;   
                          ReplaceTime(LargeDate,0);   
                          Diff=LargeDate-SmallDate;   
                          Diff=Diff   *   24   +Hour1-Hour;   
                          Diff=Diff   *   60   +Min1-Min;   
                          Diff=Diff   *   60   +Sec1-Sec;
                          Diff=Diff   *   1000   +MSec1-MSec;
                          break;
	}
	//返回计算的结果；
	return   Diff;
}

//timebuf至少需要6个空间
void EncodeBCDTime(struct tm *pstm, UINT8* timebuf){
	timebuf[0] = UINT8ToBCD(pstm->tm_year - 100); //year - 1900
        timebuf[1] = UINT8ToBCD(pstm->tm_mon + 1);
        timebuf[2] = UINT8ToBCD(pstm->tm_mday);
        timebuf[3] = UINT8ToBCD(pstm->tm_hour);
        timebuf[4] = UINT8ToBCD(pstm->tm_min);
        timebuf[5] = UINT8ToBCD(pstm->tm_sec);
}

TDateTime DecodeBCDTime(UINT8 *pstream, UINT32 length)
{

       // UINT32 pos = 0;
	AnsiString rv = "";
	//struct tm stm = {0};
	//struct tm *ptm;
	TDateTime DA,TM,dt;
        if (pstream == NULL || length == 0) return dt;
        try{
	        //time_t timex;
                /*
	        if (length >= 1) stm.tm_year = 100 + BCDToUINT8(pstream[0]);
	        if (length >= 2) stm.tm_mon = BCDToUINT8(pstream[1]) - 1;
	        if (length >= 3) stm.tm_mday = BCDToUINT8(pstream[2]);
	        if (length >= 4) stm.tm_hour = BCDToUINT8(pstream[3]);
	        if (length >= 5) stm.tm_min = BCDToUINT8(pstream[4]);
	        if (length >= 6) stm.tm_sec = BCDToUINT8(pstream[5]);
	        mktime(&stm);
                */
                if (pstream[2] > 0){ //Day start from 1
                        TryEncodeDate(2000 + BCDToUINT8(pstream[0]),
                                BCDToUINT8(pstream[1]), BCDToUINT8(pstream[2]), DA);
                        TryEncodeTime(BCDToUINT8(pstream[3]), BCDToUINT8(pstream[4]),
                                BCDToUINT8(pstream[5]), 0, TM);
                        dt = DA + TM;
                }else{
                       // dt = Now();
                       return 0;
                }
	        /*DA = EncodeDate(BCDToUINT8(pstream[0]) + 2000,
		        BCDToUINT8(pstream[1]),BCDToUINT8(pstream[2]));
	        TM = EncodeTime(stm.tm_hour,
		        BCDToUINT8(pstream[4]),
		        BCDToUINT8(pstream[5]),0);
	        dt = DA + TM;*/
        }catch(...){
                //do nothing
        }
	//rv = DateTimeToStr(dt);
	//rv = asctime(&localtime(timex));
	//rv = asctime(&stm);
	//rv.SubString(0, rv.Length()-2);

	return dt;
}

bool LoadJpegFile(TImage* oimage, AnsiString filename)
{
        TJPEGImage *jpeg = NULL;
        BOOL rv = true;
	try{
		jpeg = new TJPEGImage();
        	jpeg->LoadFromFile(filename);
                jpeg->DIBNeeded();
        	oimage->Picture->Assign(jpeg);
        	delete jpeg;
        }catch(...){
                if (jpeg != NULL) delete jpeg;
                rv = false;
        }

        return rv;
}

bool LoadTxtFile(TMemo* memo, AnsiString filename)
{
        BOOL rv = true;
        try{
	        if (memo != NULL){
			memo->Lines->LoadFromFile(filename);
	        }
        }catch(...){
        	rv = false;
        }

        return rv;
}

int getperline(char* line, void* param)
{
        TMemo* memo = (TMemo*)param;
        memo->Lines->Add(line);
	return 1;
}
bool LoadBinaryFile(TMemo* memo, AnsiString filename)
{
	BOOL rv = true;
        try{
	        if (memo != NULL){
			//打开文件，写入数据
                        FILE *fhd;
	        	struct stat st;
	        	UINT8 *pFileData;
        		stat(filename.c_str(), &st);
        		pFileData = (char*)malloc(st.st_size);
                        fhd = fopen(filename.c_str(), "rb");
                        if (fhd == NULL) return false;
                	fread(pFileData, sizeof(char), st.st_size, fhd);
                        fclose(fhd);
                        //转换成二进制,每行显示
                        //hexdump(pFileData, st.st_size, getperline, memo);
                        memo->Text = StreamToHexView(pFileData, st.st_size);
                        free(pFileData);
	        }
        }catch(...){
        	rv = false;
        }

        return rv;
}

/*
//加载一个图片到视频多帧流
bool LoadVideoToMultiFraStm(
	AnsiString filename,
	UINT32 devid,
        UINT16 segmentlen,
        CPicData* pd)
{
        FILE *fhd;
        struct stat st;
        
	fhd = fopen(OpenDialog1->FileName.c_str(), "rb");
        if (fhd != NULL){
        	stat(OpenDialog1->FileName.c_str(), &st);
                pFileData = (char*)malloc(st.st_size);
                fread(pFileData, sizeof(char), st.st_size, fhd);
                fclose(fhd);

                CDataPackage *msFileData = new CDataPackage();
                msFileData->Write(pFileData, st.st_size);
                free(pFileData);
                msFileData->DevID = devid;
                msFileData->vDevType = PicDevType;

                return LoadVideoToMultiFraStm(msFileData, PACKAGE_MAX_DATA_LEN, pd);
        }

        return false;
}
*/

void hexdump(void *pAddressIn, long  lSize,
	int (*cbperline)(char*, void* param), void* param)
{
 char szBuf[100];
 long lIndent = 1;
 long lOutLen, lIndex, lIndex2, lOutLen2;
 long lRelPos;
 struct { char *pData; unsigned long lSize; } buf;
 unsigned char *pTmp,ucTmp;
 unsigned char *pAddress = (unsigned char *)pAddressIn;

   buf.pData   = (char *)pAddress;
   buf.lSize   = lSize;

   while (buf.lSize > 0)
   {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
          lOutLen = 16;

      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
                     "                      "
                     "    %08lX", pTmp-pAddress);
      lOutLen2 = lOutLen;

      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
         )
      {
         ucTmp = *pTmp++;

         sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
         if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
         szBuf[lIndex2] = ucTmp;

         if (!(++lRelPos & 3))     // extra blank after 4 bytes
         {  lIndex++; szBuf[lIndex+2] = ' '; }
      }

      if (!(lRelPos & 3)) lIndex--;

      szBuf[lIndex  ]   = '<';
      szBuf[lIndex+1]   = ' ';

      //printf("%s\n", szBuf);
      if (!(*cbperline)(szBuf, param)){
        break;
      }

      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
   }
}


AnsiString GetFloatStr(const float& value)
{
        char buff[50];
        if (IsNan(value)){
                snprintf(buff, 50, "NaN");
        }else{
                snprintf(buff, 50, "%.4f", value);
        }
        return AnsiString(buff);
}

AnsiString GetFloatStrHighPrec(const float& value)
{
        char buff[50];
        if (IsNan(value)){
                snprintf(buff, 50, "NaN");
        }else{
                snprintf(buff, 50, "%.8f", value);
        }
        return AnsiString(buff);
}

float GetFloatFromStr(AnsiString floatstr)
{
        float vf32 = 0.0f;

        TryStrToFloat(floatstr, vf32);

        return vf32;
}

AnsiString StreamToHexView(UINT8 *pStream, UINT32  szLen)
{
        AnsiString retstr = "";
        UINT8 *pData = pStream;
        char buff[48 + 10 + 2]; // 10bytes --> "0000abcd: "
        UINT32 pos = 0;
        int idx = 0;
        while(pos < szLen)
        {
                if (idx == 0){
                	sprintf(buff, "%08lX: ", pData - pStream);
                        idx = 10;
                }
                buff[idx++] = Int4ToHex(*pData >> 4);
                buff[idx++] = Int4ToHex(*pData & 0x0F);
                buff[idx++] = ' ';
                if (idx == 58){
                        buff[idx-1] = '\r';
                        buff[idx] = '\n';
                        buff[idx+1] = '\0';
                        retstr += buff;
                        idx = 0;
                }
                pos++;
                pData++;
        }
        buff[idx] = '\0'; 
        retstr += buff;
        return retstr;
}

//---------------------------------------------------------------------------
AnsiString formatXMLDoc(TXMLDocument* doc, int indent)
{

        AnsiString sRes;
        int i;
        sRes = "<?xml version=\"" + doc->Version + "\" encoding=\"" + doc->Encoding + "\"?>\r\n";
        sRes += "<" + doc->DocumentElement->NodeName;
                for (i = 0; i < doc->DocumentElement->AttributeNodes->Count; i++)
                {
                        sRes += " " + doc->DocumentElement->AttributeNodes->Nodes[i]->NodeName
                                + "=\"" + doc->DocumentElement->AttributeNodes->Nodes[i]->NodeValue + "\"";
                }
                sRes += ">\r\n";

        for (i = 0; i < doc->DocumentElement->ChildNodes->Count; ++i){
                sRes += formatXMLNode(doc->DocumentElement->ChildNodes->Nodes[i], indent);
        }
        sRes += "</" + doc->DocumentElement->NodeName + ">\r\n";
        return  sRes;
}

AnsiString formatXMLNode(_di_IXMLNode element, int indent)
{

        AnsiString sBlank = "";
        AnsiString sRes = "";
        int i;
        for (i = 0; i < indent; ++i){
                sBlank += " ";
        }

        if (element->NodeType == ELEMENT_NODE
                && element->ChildNodes && element->ChildNodes->Count > 0
                && element->ChildNodes->Nodes[0]->NodeType != TEXT_NODE)
        {
                sRes = sBlank + '<'+element->NodeName;
                for (i = 0; i < element->AttributeNodes->Count; i++)
                {
                        sRes += " " + element->AttributeNodes->Nodes[i]->NodeName
                                + "=\"" + element->AttributeNodes->Nodes[i]->NodeValue + "\"";
                }
                sRes += ">\r\n";
                indent++;
                for (i = 0; i < element->ChildNodes->Count; i++)
                {
                        sRes += formatXMLNode(element->ChildNodes->Nodes[i], indent);
                }
                sRes += sBlank + "</" + element->NodeName + ">\r\n";
        }
        else if (element->NodeType != PROCESSING_INSTRUCTION_NODE)
        {
                sRes += sBlank + element->XML + "\r\n";
        }
        return sRes;
}

/*************************crc 校验*****************************/
/* 要进行CRC校验的消息 */
/* 消息中字节数 */
unsigned short crc16_cal(unsigned char *puchMsgg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF ; /* 高CRC字节初始化 */ 
	unsigned char uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */
	unsigned short	return_result;
	unsigned uIndex ; /* CRC循环中的索引 */ 
	while (usDataLen--) /* 传输消息缓冲区 */ 
	{
		uIndex = uchCRCHi ^ *puchMsgg++ ; /* 计算CRC */
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ; 
		uchCRCLo = auchCRCLo[uIndex] ; 
	} 
	return_result =  uchCRCHi << 8;
	return_result = return_result +  uchCRCLo;
	return return_result; 
}

/**
 *  字符串分割:
 *   (abc,b) -->  a   | c
 *   (abc,ab)-->  ""  | c
 *   (abc,dd)-->  abc
 *   (abc,bc)-->  a   | ""
 *   tip: "" 表示空字符串
 */
std::vector<AnsiString> spiltStr(AnsiString src, AnsiString spilt){
        std::vector<AnsiString> items;
        int spiltLen = spilt.Length();
        AnsiString spiltStr = src;
        int idx = spiltStr.Pos(spilt);
        while(idx != 0){
                AnsiString temp = spiltStr.SubString(1,idx - 1);
                items.push_back(temp);
                spiltStr = spiltStr.SubString(idx + spiltLen,spiltStr.Length() - spiltLen);
                idx = spiltStr.Pos(spilt);
        }
        items.push_back(spiltStr);
        return items;
}

void reverse_ch(UINT8 *desc, UINT8 *src, int len){
        for(int i = 0; i < len; i++){
                desc[i] = src[len - i - 1];  
        }
}
