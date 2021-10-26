#include "ringbuffer.h"
#include "threadinfo.h"
#include "stdio.h"
#include "stdlib.h"



/************************ 缓存用户测试接口定义 ********************************/
#define USER_TEST_BUFFER_LEN	(1024*1024)
#define USER_TEST_HEAD_LEN		(320)
#define USER_TEST_DATA_LEN		(4608 * 48)

void user_ring_buffer_read_test(void *pArgs)
{
	RING_BUFFER_T *pStRingBuffer = (RING_BUFFER_T *)pArgs;
	unsigned char *pReadBuff = NULL;
	int iRet = RING_BUFFER_ERROR;
	
	pReadBuff = malloc(USER_TEST_BUFFER_LEN);	
	if(NULL == pArgs)
	{
		RING_BUFFER_ERR("pArgs = %p, Err!\n", pArgs);
		return ;
	}
	
	RING_BUFFER_HINT("test func start!\n");

	while(1)
	{
		if(RING_BUFFER_TRUE != pStRingBuffer->stUserTest.bRun)
		{
			continue;
		}
		
		if(RING_BUFFER_TRUE == pStRingBuffer->stUserTest.bExit)
		{
			break;
		}		
	
		iRet = pStRingBuffer->pRead(pStRingBuffer, pReadBuff, USER_TEST_HEAD_LEN);
		if(RING_BUFFER_OK != iRet)
		{
			//RING_BUFFER_HINT("buffer empty!\n");
			continue;
		}
		
		iRet = pStRingBuffer->pRead(pStRingBuffer, pReadBuff, USER_TEST_DATA_LEN);
		if(RING_BUFFER_OK != iRet)
		{
			//RING_BUFFER_HINT("buffer empty, revert head ridx!\n");
			pStRingBuffer->pRidxRevert(pStRingBuffer, USER_TEST_HEAD_LEN);	
			continue;
		}	
		
	}	
	
}

int main()
{
	int iRet = RING_BUFFER_ERROR;
	RING_BUFFER_T *pStRingBuffer = NULL;
	unsigned char *pWriteBuff = NULL;

	register_signal();
	
	pWriteBuff = malloc(USER_TEST_BUFFER_LEN);
	if(NULL == pWriteBuff)
	{
		RING_BUFFER_ERR("pWriteBuff malloc failed!\n");
		return RING_BUFFER_ERROR;
	}
	
	pStRingBuffer = ring_buffer_init(100*1024*1024);
	if(NULL == pStRingBuffer)
	{
		RING_BUFFER_SAFE_FREE(pWriteBuff);
		RING_BUFFER_ERR("ring buffer init failed!\n");
		return RING_BUFFER_ERROR;
	}
	
	RING_BUFFER_HINT("ring_buffer_init success!\n");
	
	pStRingBuffer->stUserTest.bExit = RING_BUFFER_FALSE;
	pStRingBuffer->stUserTest.bRun = RING_BUFFER_FALSE;
	pStRingBuffer->stUserTest.pCallFunc = user_ring_buffer_read_test;
	ring_buffer_user_test_thread_create(pStRingBuffer);
	
	RING_BUFFER_HINT("ring_buffer_user_test_thread_create success!\n");
	
	pStRingBuffer->stUserTest.bRun = RING_BUFFER_TRUE;
	while(1)
	{
		//usleep(1000 * 1000);
		ring_buffer_debug_log(pStRingBuffer);
		
		iRet = pStRingBuffer->pWrite(pStRingBuffer, pWriteBuff, USER_TEST_HEAD_LEN);
		if(RING_BUFFER_OK != iRet)
		{
			//RING_BUFFER_HINT("buffer full!\n");
			continue;
		}
		
		iRet = pStRingBuffer->pWrite(pStRingBuffer, pWriteBuff, USER_TEST_DATA_LEN);
		if(RING_BUFFER_OK != iRet)
		{
			//RING_BUFFER_HINT("buffer full, revert head widx!\n");
			pStRingBuffer->pWidxRevert(pStRingBuffer, USER_TEST_HEAD_LEN);
			continue;
		}
		
	}

	pStRingBuffer->stUserTest.bExit = RING_BUFFER_TRUE;

	RING_BUFFER_SAFE_FREE(pWriteBuff);
	
	pStRingBuffer->pDeinit(pStRingBuffer);
	
	return RING_BUFFER_OK;
}