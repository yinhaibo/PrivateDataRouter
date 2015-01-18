#include <stdio.h>
#include "loopbuffer.h"

int loopbuff_init(ploop_buff_t _this, unsigned char *buffer, unsigned int bufsize)
{
	_this->head =0;
	_this->tail = 0;
	_this->empty = TRUE;
	_this->full = FALSE;
	_this->size = bufsize;
	_this->pbuff = buffer;

	return 0;
}

int loopbuff_push(ploop_buff_t _this, int bytevalue)
{
	if (_this->full){
		return -1;
	}
	_this->pbuff[_this->tail] = bytevalue & 0xFF;
	if ((unsigned int)++_this->tail >= _this->size){
		_this->tail = 0;
	}
	if (_this->head == _this->tail){
		_this->full = TRUE;
	}
	_this->empty = FALSE;

	return 0;
}

int loopbuff_pull(ploop_buff_t _this)
{
	int value = -1;
	if (_this->empty){
		return value;
	}

	value = _this->pbuff[_this->head];
	value &= 0xFF;
	if ((unsigned int)++_this->head >= _this->size){
		_this->head = 0;
	}
	if (_this->head == _this->tail){
		_this->empty = TRUE;
	}
	_this->full = FALSE;

	return value;
}

void loopbuff_discard(ploop_buff_t _this, int len)
{
	int value;
	while(len--){
		if ( (value = loopbuff_pull(_this)) == -1 ) break;
		printf("%c", value);
	}	
}
void loopbuff_clear(ploop_buff_t _this)
{
	_this->head =0;
	_this->tail = 0;
	_this->empty = TRUE;
	_this->full = FALSE;
}

unsigned char loopbuff_isfull(ploop_buff_t _this)
{
	return _this->full;
}

unsigned char loopbuff_isempty(ploop_buff_t _this)
{
	return _this->empty;
}

int loopbuff_size(ploop_buff_t _this)
{
	return _this->size;
}

unsigned int loopbuff_view(ploop_buff_t _this, int offset, int len)
{
	unsigned int rv = 0;
	unsigned int pos;
	pos = _this->head + offset;
	while(pos >= _this->size){
		pos -= _this->size;
	}
	switch(len){
	case 1:
		rv = _this->pbuff[pos];
		break;
	case 2:
		rv = _this->pbuff[pos];
		if (pos + 1 < _this->size){
			rv |= _this->pbuff[pos+1] << 8;
		}else rv |= (_this->pbuff[0] << 8); 
		break;
	case 4:
		rv = _this->pbuff[pos];
		if (pos + 1 < _this->size){
			rv |= _this->pbuff[pos+1] << 8;
		}else rv |= (_this->pbuff[0] << 8);
		if (pos + 2 < _this->size){
			rv |= _this->pbuff[pos+2] << 16;
		}else rv |= (_this->pbuff[pos+2-_this->size] << 16);
		if (pos + 3 < _this->size){
			rv |= _this->pbuff[pos+3] << 24;
		}else rv |= (_this->pbuff[pos+3-_this->size] << 24);
		break;
	}

	return rv;
}

unsigned int loopbuff_getlen(ploop_buff_t _this, int offset)
{
  	//if(offset == _this->tail){
		if(_this->empty)	return(0);
	  	if(_this->full)	return(_this->size - offset);
	//}

	if(_this->head > _this->tail)
          return(_this->size - (_this->head - _this->tail) - offset);
	else
          return(_this->tail - _this->head - offset);
}
