//---------------------------------------------------------------------------

#ifndef __UICOMM_HEADER_H_
#define __UICOMM_HEADER_H_

#include <Classes.hpp>
#include "base.h"
#include "CDataPackage.h"
//�����������������ӿڶ���
//������
//    in  Դ������
//    out Ŀ��������
//����ֵ
//    �Ƿ����������
typedef bool (* FPStreamFilter)(CDataPackage* in,
        CDataPackage* out);
        
class IComm
{
public:
	virtual void Response(TMemoryStream *memstm, UINT32 devid) = 0;
        virtual bool AddCmd(DataItemCmdCode) = 0;
        virtual bool AddCmd(DataItemCmdCode, UINT32 devid) = 0;
        virtual bool AddCmd(DataItemCmdCode, UINT32 devid, PACKTRACK callback) = 0;
        virtual bool AddCmd(DataItemCmdCode, UINT32 devid, PACKTRACK callback, UINT32 timeout) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, TMemoryStream*) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT8*, UINT32 bsize) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT32 devid, TMemoryStream*) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT32 devid, UINT8*, UINT32 bsize) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT32 devid, PACKTRACK callback, TMemoryStream*) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT32 devid, PACKTRACK callback, UINT8*, UINT32 bsize) = 0;
        virtual bool AddCmdWithStream(DataItemCmdCode, UINT32 devid, PACKTRACK callback, UINT8*, UINT32 bsize, UINT32 timeout) = 0;
};

#endif //~__UICOMM_HEADER_H_
