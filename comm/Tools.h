//---------------------------------------------------------------------------

#ifndef ToolsH
#define ToolsH

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

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <vector>
#pragma hdrstop


UINT8 HexToInt8(char hex);
char Int4ToHex(UINT8 value);
int TextToStream(AnsiString text, UINT8 *pStream, int szLen);
int AsciiToStream(AnsiString text, UINT8 *pStream, int szLen);
AnsiString StreamToText(UINT8 *pStream, int szLen);
AnsiString StreamToAscii(UINT8 *pStream, int szLen);
UINT8  TextToUINT8(AnsiString text);
UINT16 TextToUINT16(AnsiString text);
UINT32 TextToUINT32(AnsiString text);
UINT8  HexToUINT8(AnsiString text);
UINT16 HexToUINT16(AnsiString text);
UINT32 HexToUINT32(AnsiString text);
AnsiString UINT16ToHex(UINT16 value);
AnsiString UINT16ToHex4(UINT16 value);
AnsiString UINT32ToHex(UINT32 value);
AnsiString UINT32ToHex4(UINT32 value);
AnsiString UINT8ToHex(UINT8 value);
UINT8 UINT8ToBCD(UINT8 value);
UINT8 BCDToUINT8(UINT8 bcd);
UINT32 ASCIIsToHexs(UINT8 *dst, UINT32 dstSize, UINT8 *src, UINT32 srcLen);//Added by zz on 2012/04/13
UINT8 Ascii2Hex(UINT8 ascii);//Added by zz on 2012/04/13
UINT32 HexsToASCIIs(UINT8 *dst, UINT8 *src, UINT32 srcLen);//Added by zz on 2012/04/16
UINT8 Hex2Ascii(UINT8 hex);//Added by zz on 2012/04/16

AnsiString GetFloatStr(const float& value);
AnsiString GetFloatStrHighPrec(const float& value);
float GetFloatFromStr(AnsiString floatstr);

void EncodeBCDTime(struct tm *pstm, UINT8* timebuf);

typedef   enum{
	dfYear, dfMonth, dfWeek, dfDay,
        dfHour, dfMinute, dfSecond, dfMilliSecond
}TDatePart;

int __fastcall DateDiff(TDatePart datePart,
	TDateTime StartDate, TDateTime EndDate);

TDateTime DecodeBCDTime(UINT8 *pstream, UINT32 length);

//加载一个JPEG文件到IMAGE控件
bool LoadJpegFile(TImage* oimage, AnsiString filename);
//加载一个文本文件到MEMO控件
bool LoadTxtFile(TMemo* memo, AnsiString filename);
//加载一个二进制文件
bool LoadBinaryFile(TMemo* memo, AnsiString filename);
//将一个电表数据状态结构转换成类
//void TransMeterStatus(pmeter_status_t, CMeterStatus*);

/*
//加载一个图片到视频多帧流
bool LoadVideoToMultiFraStm(
	AnsiString filename,
	UINT32 devid,
        UINT16 segmentlen,
        CPicData* pd);
bool LoadVideoToMultiFraStm(
	CDataPackage *package,
        UINT16 segmentlen,
        CPicData* pd);
//加载一个程序到程序多帧流
bool LoadProgToMultiFraStm(
	AnsiString filename,
	UINT32 devid,
        UINT16 segmentlen,
        CProgFile* pd);
bool LoadProgToMultiFraStm(
	CDataPackage *package,
        UINT16 segmentlen,
        CPicData* pd);
*/

void hexdump(void *pAddressIn, long  lSize,
	int (*cbperline)(char*, void* param), void* param);
AnsiString StreamToHexView(UINT8 *pStream, UINT32  szLen);
AnsiString formatXMLDoc(TXMLDocument* doc, int indent);
AnsiString formatXMLNode(_di_IXMLNode element, int indent);
AnsiString GetErrDesc(UINT32 errnum);
unsigned short crc16_cal(unsigned char *puchMsgg, unsigned short usDataLen);

std::vector<AnsiString> spiltStr(AnsiString src, AnsiString spilt);
void reverse_ch(UINT8 *desc, UINT8 *src, int len);
//---------------------------------------------------------------------------
#endif
