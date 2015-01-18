#ifndef __LOOPBUFFER_H__
#define __LOOPBUFFER_H__


/**
 * 循环缓冲数据结构
 */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// -----------------------------
//|   |--------------------|
// -----------------------------
//	  ^                    ^
//   head                 tail

typedef struct _loop_buff{
	unsigned short head;  // 接收到的有效数据位置
	unsigned short tail;  // 当前接收到的数据
	unsigned char empty;
	unsigned char full;
	unsigned int size;
	unsigned char* pbuff;
}loop_buff_t;
typedef loop_buff_t *ploop_buff_t;


int loopbuff_init(ploop_buff_t _this, unsigned char *buffer, unsigned int bufsize);
int loopbuff_push(ploop_buff_t _this, int bytevalue);
int loopbuff_pull(ploop_buff_t _this);
void loopbuff_discard(ploop_buff_t _this, int len);
void loopbuff_clear(ploop_buff_t _this);
unsigned char loopbuff_isfull(ploop_buff_t _this);
unsigned char loopbuff_isempty(ploop_buff_t _this);
int loopbuff_size(ploop_buff_t _this);
unsigned int loopbuff_view(ploop_buff_t _this, int offset, int len);
unsigned int loopbuff_getlen(ploop_buff_t _this, int offset);
#endif
