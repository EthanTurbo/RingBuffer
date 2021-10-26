#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK
#include <semaphore.h> 
#endif


//#define RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK
//#define RING_BUFFER_ADOPT_SEMAPHORE


/************************ 部分变量类型宏定义 ********************************/
#define RING_BUFFER_OK 		0
#define RING_BUFFER_ERROR 	-1

#define RING_BUFFER_TRUE 	1
#define RING_BUFFER_FALSE 	0

/************************ 打印机制 ********************************/
#define RING_BUFFER_DEBUG_PRINT

#define RING_BUFFER_ERR(fmt, ...) printf("[ERR][%s][%s][%s][%s %d]"fmt,__DATE__,__TIME__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__)

#ifdef RING_BUFFER_DEBUG_PRINT
	#define RING_BUFFER_HINT(fmt, ...) printf("[HINT][%s][%s][%s][%s %d]"fmt,__DATE__,__TIME__,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__)
#else
	#define RING_BUFFER_HINT(fmt, ...)
#endif

/************************ 宏函数定义安全函数 ********************************/
#define RING_BUFFER_ASSERT(pParam)	\
	do\
	{\
		if(NULL == pParam)\
		{\
			RING_BUFFER_ERR("%s = %p, Err!\n", #pParam, pParam);\
			return RING_BUFFER_ERROR;\
		}\
	}while(0)\

#define RING_BUFFER_SAFE_FREE(pParam)	\
	do\
	{\
		if(!pParam)\
		{\
			free(pParam);\
		}\
	}while(0)\

/************************ 结构体定义 ********************************/
typedef struct
{
	volatile bool bExit;
	volatile bool bRun;
	void (*pCallFunc)(void *pArgs);
}RING_BUFFER_USER_TEST_T;

typedef struct
{

}RING_BUFFER_ABILITY_T;

typedef struct RING_BUFFER_T
{
	volatile unsigned int uiWidx;
	volatile unsigned int uiRidx;
	volatile bool bFull;
	volatile bool bEmpty;
	unsigned int uiBuffLen;
	unsigned char *pBuff;
#ifdef RING_BUFFER_ADOPT_PTHREAD_MUTEX_LOCK	
	pthread_mutex_t stMutex;
#else
	sem_t stSemLock;
#endif	
	RING_BUFFER_USER_TEST_T stUserTest;
	//RING_BUFFER_ABILITY_T stAbility;

	struct RING_BUFFER_T* (*pInit)(unsigned int uiBuffLen);
	int (*pDeinit)(struct RING_BUFFER_T *pRingBuf);
	int (*pWrite)(struct RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiWlen);
	int (*pRead)(struct RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiRlen);
	bool (*pIsFull)(struct RING_BUFFER_T *pRingBuf);
	bool (*pIsEmpty)(struct RING_BUFFER_T *pRingBuf);
	int (*pRegister)(struct RING_BUFFER_T *pRingBuf);
	int (*pWidxRevert)(struct RING_BUFFER_T *pRingBuf, unsigned int uiLen);
	int (*pRidxRevert)(struct RING_BUFFER_T *pRingBuf, unsigned int uiLen);	
	
}RING_BUFFER_T;

/************************ 函数声明 ********************************/
RING_BUFFER_T * ring_buffer_init(unsigned int uiBuffLen);

int ring_buffer_deinit(RING_BUFFER_T *pRingBuf);

int ring_buffer_write(RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiWlen);

int ring_buffer_read(RING_BUFFER_T *pRingBuf, unsigned char *pBuff, unsigned int uiRlen);

bool ring_buffer_is_full(RING_BUFFER_T *pRingBuf);

bool ring_buffer_is_empty(RING_BUFFER_T *pRingBuf);

int ring_buffer_ridx_revert(RING_BUFFER_T *pRingBuf, unsigned int uiLen);

int ring_buffer_widx_revert(RING_BUFFER_T *pRingBuf, unsigned int uiLen);

int ring_buffer_user_test_thread_create(void *pArgs);

int ring_buffer_debug_log(RING_BUFFER_T *pstRingBuf);

#endif