/*
	Author	: Tang Chengwu
	Date	: 2021/09/20
	Version	: 2021/09/20 1.0 -> 创建循环缓存，验证完正常的功能，添加打印机制；
			  2021/09/21 2.0 -> 使用两种锁方式，信号量和互斥锁
*/
#include "ringbuffer.h"
#include <pthread.h>
#include <string.h>

#include <sys/time.h>
/*
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <execinfo.h>
*/

static inline int ring_buffer_lock_init(RING_BUFFER_T *pRingBuf)
{
	int iRet = RING_BUFFER_ERROR;
	
#ifdef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK		
	iRet = pthread_mutex_init(&pRingBuf->stMutex, NULL);
	if( RING_BUFFER_OK != iRet )
	{
		RING_BUFFER_ERR("pthread_mutex_init failed!\n");
		return RING_BUFFER_ERROR;
	}
#else
	iRet = sem_init(&pRingBuf->stSemLock, 0, 0);
	if( RING_BUFFER_OK != iRet )
	{
		RING_BUFFER_ERR("sem_init failed!\n");;
		return RING_BUFFER_ERROR;
	}
#endif

	return RING_BUFFER_OK;		
}
static inline void ring_buffer_lock_destroy(RING_BUFFER_T *pRingBuf)
{
#ifdef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK	
	pthread_mutex_destroy(&pRingBuf->stMutex);
#else
	sem_destroy(&pRingBuf->stSemLock);
#endif	
}


static inline int ring_buffer_lock(RING_BUFFER_T *pRingBuf)
{
#ifdef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK			
	pthread_mutex_destroy(&pRingBuf->stMutex);
#else
	sem_destroy(&pRingBuf->stSemLock);
#endif	
	return RING_BUFFER_OK;
}

static inline int ring_buffer_unlock(RING_BUFFER_T *pRingBuf)
{
#ifdef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK		
	pthread_mutex_unlock(&pRingBuf->stMutex);
#else
	sem_post(&pRingBuf->stSemLock);
#endif
	return RING_BUFFER_OK;		
}

int ring_buffer_register(RING_BUFFER_T *pRingBuf)
{
	pRingBuf->pInit = ring_buffer_init;
	pRingBuf->pDeinit = ring_buffer_deinit;
	RING_BUFFER_ASSERT(pRingBuf);
	
	pRingBuf->pWrite = ring_buffer_write;
	pRingBuf->pRead = ring_buffer_read;
	pRingBuf->pIsFull = ring_buffer_is_full;
	pRingBuf->pIsEmpty = ring_buffer_is_empty;
	pRingBuf->pRegister = ring_buffer_register;
	pRingBuf->pRidxRevert = ring_buffer_ridx_revert;
	pRingBuf->pWidxRevert = ring_buffer_widx_revert;
	
	return RING_BUFFER_OK;
}

RING_BUFFER_T *ring_buffer_init(unsigned int uiBuffLen)
{	
	RING_BUFFER_T *pStRingBuf = NULL;
	int iRet = RING_BUFFER_ERROR;
	
	if( 0 >= uiBuffLen)
	{
		RING_BUFFER_ERR("uiBuffLen = %d, Err!", uiBuffLen);
		return NULL;
	}
	
	pStRingBuf = malloc(sizeof(RING_BUFFER_T));
	if(NULL == pStRingBuf)
	{
		RING_BUFFER_ERR("pStRingBuf malloc failed!\n");
		return NULL;
	}
	
	iRet = ring_buffer_lock_init(pStRingBuf);;
	if(RING_BUFFER_OK != iRet)
	{
		RING_BUFFER_SAFE_FREE(pStRingBuf);
		RING_BUFFER_ERR("ring_buffer_lock_init failed!\n");
		return NULL;
	}	

	pStRingBuf->uiWidx = 0;
	pStRingBuf->uiRidx = 0;
	pStRingBuf->bEmpty = RING_BUFFER_FALSE;
	pStRingBuf->bFull = RING_BUFFER_FALSE;
	pStRingBuf->pBuff = malloc(uiBuffLen);
	if(NULL == pStRingBuf->pBuff)
	{
		RING_BUFFER_ERR("pStRingBuf->pBuff malloc failed!\n");
		ring_buffer_lock_destroy(pStRingBuf);
		RING_BUFFER_SAFE_FREE(pStRingBuf);
		return NULL;
	}
	
	pStRingBuf->uiBuffLen = uiBuffLen;
	
	ring_buffer_register(pStRingBuf);

	return pStRingBuf;
}

int ring_buffer_deinit(RING_BUFFER_T *pRingBuf)
{
	RING_BUFFER_ASSERT(pRingBuf);
	
	ring_buffer_lock_destroy(pRingBuf);
	
	RING_BUFFER_SAFE_FREE(pRingBuf->pBuff);
	RING_BUFFER_SAFE_FREE(pRingBuf);
	
	return RING_BUFFER_OK;
}

int ring_buffer_write(RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiWlen)
{
	unsigned int uiRidx = 0, uiWidx = 0;
	unsigned int uiShLen1 = 0, uiShLen2 = 0;
	unsigned int uiTotalLen = 0;
	RING_BUFFER_ASSERT(pRingBuf);
	RING_BUFFER_ASSERT(pBuff);

	ring_buffer_lock(pRingBuf);

	uiRidx = pRingBuf->uiRidx;
	uiWidx = pRingBuf->uiWidx;
	uiTotalLen = pRingBuf->uiBuffLen;

	if(uiWidx >= uiRidx)
	{
/*	
	--------------------
	====r          w====
		
*/				
		uiShLen1 = uiTotalLen - uiWidx;
		uiShLen2 = uiRidx;
	}
	else
	{
/*	
	--------------------
	    w==========r       
		
*/	
		uiShLen1 = uiRidx - uiWidx;
		uiShLen2 = 0;
	}
	
	if(uiWlen <= uiShLen1+uiShLen2)
	{
		
		{
			
		}
		if(uiWlen <= uiShLen1)
		{
			if(uiWidx+uiWlen-1 >= uiTotalLen)
			{
				RING_BUFFER_HINT("uiTotalLen = %d, wlen+widx = %d", uiTotalLen, uiWidx+uiWlen);
			}
			memcpy(pRingBuf->pBuff+uiWidx, pBuff, uiWlen);
		}
		else
		{
			if(uiWidx+uiShLen1-1 >= uiTotalLen)
			{
				RING_BUFFER_HINT("uiTotalLen = %d, uiShLen1+widx = %d", uiTotalLen, uiWidx+uiShLen1);
			}			
			memcpy(pRingBuf->pBuff+uiWidx, pBuff, uiShLen1);			
			memcpy(pRingBuf->pBuff+uiWidx+uiShLen1, pBuff+uiShLen1, uiWlen-uiShLen1);
		}
		pRingBuf->bFull = RING_BUFFER_FALSE;
		pRingBuf->uiWidx = (pRingBuf->uiWidx + uiWlen) % uiTotalLen;
	}
	else
	{
		pRingBuf->bFull = RING_BUFFER_TRUE;
		ring_buffer_unlock(pRingBuf);
		return RING_BUFFER_ERROR;
	}
	
	ring_buffer_unlock(pRingBuf);
	
	return RING_BUFFER_OK;
}

int ring_buffer_read(RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiRlen)
{
	unsigned int uiRidx = 0, uiWidx = 0;
	unsigned int uiShLen1 = 0, uiShLen2 = 0;
	unsigned int uiTotalLen = 0;
	
	RING_BUFFER_ASSERT(pRingBuf);
	RING_BUFFER_ASSERT(pBuff);
	
	ring_buffer_lock(pRingBuf);
	
	uiRidx = pRingBuf->uiRidx;
	uiWidx = pRingBuf->uiWidx;
	uiTotalLen = pRingBuf->uiBuffLen;

	if(uiWidx > uiRidx)
	{
/*	
	--------------------
	====w          r====
		
*/				
		uiShLen1 = uiTotalLen - uiRidx;
		uiShLen2 = uiWidx;
	}
	else
	{
/*	
	--------------------
	    r==========w       
		
*/	
		uiShLen1 = uiWidx - uiRidx;
		uiShLen2 = 0;
	}
	
	if(uiRlen <= uiShLen1+uiShLen2)
	{
		
		if(uiRlen <= uiShLen1)
		{
			memcpy(pBuff, pRingBuf->pBuff+uiRidx, uiRlen);
		}
		else
		{
			memcpy(pBuff, pRingBuf->pBuff+uiRidx, uiShLen1);
			memcpy(pBuff+uiShLen1, pRingBuf->pBuff+uiRidx+uiShLen1, uiRlen-uiShLen1);
		}
		pRingBuf->bEmpty = RING_BUFFER_FALSE;
		pRingBuf->uiRidx = (pRingBuf->uiRidx + uiRlen) % uiTotalLen;
	}
	else
	{
		pRingBuf->bEmpty = RING_BUFFER_TRUE;
		ring_buffer_unlock(pRingBuf);
		return RING_BUFFER_ERROR;
	}
	
	ring_buffer_unlock(pRingBuf);
	
	return RING_BUFFER_OK;
}

bool ring_buffer_is_full(RING_BUFFER_T *pRingBuf)
{
	RING_BUFFER_ASSERT(pRingBuf);
	
	return pRingBuf->bFull;
}

bool ring_buffer_is_empty(RING_BUFFER_T *pRingBuf)
{
	RING_BUFFER_ASSERT(pRingBuf);

	return pRingBuf->bEmpty;
}

int ring_buffer_ridx_revert(RING_BUFFER_T *pRingBuf, unsigned int uiLen)
{
	RING_BUFFER_ASSERT(pRingBuf);
	
	ring_buffer_lock(pRingBuf);
	
	pRingBuf->uiRidx = (pRingBuf->uiRidx - uiLen + pRingBuf->uiBuffLen) % pRingBuf->uiBuffLen;
	
	ring_buffer_unlock(pRingBuf);
	
	return RING_BUFFER_OK;
}

int ring_buffer_widx_revert(RING_BUFFER_T *pRingBuf, unsigned int uiLen)
{
	RING_BUFFER_ASSERT(pRingBuf);
	
	ring_buffer_lock(pRingBuf);
	
	pRingBuf->uiWidx = (pRingBuf->uiWidx - uiLen + pRingBuf->uiBuffLen) % pRingBuf->uiBuffLen;
	
	ring_buffer_unlock(pRingBuf);

	return RING_BUFFER_OK;
}

int ring_buffer_user_test_thread_create(void *pArgs)
{
	RING_BUFFER_T *pStRingBuffer = (RING_BUFFER_T *)pArgs;
	pthread_t t_Tid;
	pthread_attr_t stAttr;
	int iRet = 0;
	
	RING_BUFFER_ASSERT(pArgs);
	
	pthread_attr_init(&stAttr);
	pthread_attr_setdetachstate(&stAttr, 1);
	iRet = pthread_create(&t_Tid, &stAttr, (void *)(pStRingBuffer->stUserTest.pCallFunc), pArgs);
	if(RING_BUFFER_OK != iRet)
	{
		RING_BUFFER_ERR("pCallFunc create failed!\n");
		return RING_BUFFER_ERROR;
	}
	else
	{
		RING_BUFFER_HINT("pCallFunc Start!\n");
		return RING_BUFFER_OK;
	}
}

int ring_buffer_debug_log(RING_BUFFER_T *pstRingBuf)
{
	static struct timeval stLastTime = {0};
	struct timeval stCurrentTime = {0};
	unsigned int uiTimeTick = 0;
	
	RING_BUFFER_ASSERT(pstRingBuf);
	
	gettimeofday(&stCurrentTime,NULL);
	uiTimeTick=(stCurrentTime.tv_sec-stLastTime.tv_sec);//微秒
	if( 5 < uiTimeTick )
	{
		RING_BUFFER_HINT("widx = %d, ridx = %d!\n", pstRingBuf->uiWidx, pstRingBuf->uiRidx);
		stLastTime.tv_sec = stCurrentTime.tv_sec;
	}
	
	return RING_BUFFER_OK;
}
