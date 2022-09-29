/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "ax_base_type.h"
#include "ax_sys_api.h"

int main(int argc, char *argv[])
{
	AX_S32 ret=0,i=0;
	AX_POOL_FLOORPLAN_T PoolFloorPlan;
	AX_U64 BlkSize;
	AX_BLK BlkId;
	AX_POOL PoolId;
	AX_U64 PhysAddr,MetaPhysAddr;
	AX_VOID *blockVirAddr,*metaVirAddr;

	AX_POOL_CONFIG_T PoolConfig;
	AX_POOL UserPoolId0,UserPoolId1,UserPoolId2;

	printf("sample_pool test begin\n\n");

	AX_SYS_Init();

	//step 1:去初始化缓存池
	ret = AX_POOL_Exit();

	if(ret){
		printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
		return -1;
	}else{
		printf("AX_POOL_Exit success!\n");
	}

	//step 2:设置公共缓存池参数。如下设置了三个common pool,均在anonymous分段,每个Pool里面BlockSize不同,metasize都为0x1000字节
	memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));	
	PoolFloorPlan.CommPool[0].MetaSize   = 0x1000;
	PoolFloorPlan.CommPool[0].BlkSize   = 1*1024*1024;
	PoolFloorPlan.CommPool[0].BlkCnt    = 5;
	PoolFloorPlan.CommPool[0].CacheMode = POOL_CACHE_MODE_NONCACHE;
	PoolFloorPlan.CommPool[1].MetaSize   = 0x1000;
	PoolFloorPlan.CommPool[1].BlkSize   = 2*1024*1024;
	PoolFloorPlan.CommPool[1].BlkCnt    = 5;
	PoolFloorPlan.CommPool[1].CacheMode = POOL_CACHE_MODE_NONCACHE;
	PoolFloorPlan.CommPool[2].MetaSize   = 0x1000;
	PoolFloorPlan.CommPool[2].BlkSize   = 3*1024*1024;
	PoolFloorPlan.CommPool[2].BlkCnt    = 5;
	PoolFloorPlan.CommPool[2].CacheMode = POOL_CACHE_MODE_NONCACHE;

	//PartitionName默认是anonymous分段,分段名必须是cmm模块加载时存在的
	memset(PoolFloorPlan.CommPool[0].PartitionName,0,sizeof(PoolFloorPlan.CommPool[0].PartitionName));
	memset(PoolFloorPlan.CommPool[1].PartitionName,0,sizeof(PoolFloorPlan.CommPool[1].PartitionName));
	memset(PoolFloorPlan.CommPool[2].PartitionName,0,sizeof(PoolFloorPlan.CommPool[2].PartitionName));
	strcpy((char *)PoolFloorPlan.CommPool[0].PartitionName,"anonymous");
	strcpy((char *)PoolFloorPlan.CommPool[1].PartitionName,"anonymous");
	strcpy((char *)PoolFloorPlan.CommPool[2].PartitionName,"anonymous");

	ret = AX_POOL_SetConfig(&PoolFloorPlan);

	if(ret){
		printf("AX_POOL_SetConfig fail!Error Code:0x%X\n",ret);
		return -1;
	}else{
		printf("AX_POOL_SetConfig success!\n");
	}

	//step 3:初始化公共缓存池
	ret = AX_POOL_Init();
	if(ret){
		printf("AX_POOL_Init fail!!Error Code:0x%X\n",ret);
		return -1;
	}else{
		printf("AX_POOL_Init success!\n");
	}


	/*
	经过上述配置后，就创建了公共缓存池，通过另一个窗口cat /proc/mem_cmm_info应当可以看到三个name为comm_pool_0、comm_pool_1、comm_pool_2的pool。

	注：
	1、AX_POOL_CreatePool的调用不受限于SetConfig->init流程,可直接调用。
	2、AX_POOL_CreatePool创建的缓存池name为user_pool_0、user_pool_1、、这种缓存池只能通过对应的poolid访问！！！

	3、AX_POOL_CreatePool创建的缓存池，要按照create的先后顺序遵循“先进后出”原则调用MarkDestroyPool去销毁pool
	   例如：如果创建的顺序是user_pool_0、user_pool_1、user_pool_2，那么Destroy顺序应该为user_pool_2、user_pool_1、user_pool_0

	4、MarkDestroyPool用于标记和回收AX_POOL_CreatePool接口创建的缓存池.其规则如下:
	   4.1、如果目标pool右侧还有活跃的pool，则只做标记，不真正回收内存
	   4.2、如果目标pool右侧已经没有任何pool，则回收该pool内存，并向左迭代检查带有mark标记的pool，如有则一并回收

	5、AX_POOL_CreatePool和MarkDestroyPool接口不去操作公共缓存池，因此不影响公共缓存池配置.
	*/

	//step 4:用户通过AX_POOL_CreatePool依次创建三个缓存池,poolid分别为UserPoolId0、UserPoolId1、UserPoolId2

	//创建user_pool_0 :blocksize=1*1024*1024,metasize=0x1000,block count =2 ,noncache类型
	memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
	PoolConfig.MetaSize = 0x1000;
	PoolConfig.BlkSize = 1*1024*1024;
	PoolConfig.BlkCnt = 2;
	PoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
	memset(PoolConfig.PartitionName,0,sizeof(PoolConfig.PartitionName));
	strcpy((char *)PoolConfig.PartitionName,"anonymous");

	UserPoolId0 = AX_POOL_CreatePool(&PoolConfig);

	if (AX_INVALID_POOLID == UserPoolId0)
	{
		printf("AX_POOL_CreatePool error!!!\n");
		goto ERROR;
	}else{
		printf("AX_POOL_CreatePool[%d] success\n",UserPoolId0);
	}

	//创建user_pool_1:blocksize=2*1024*1024,metasize=0x1000,block count =3 ,noncache类型
	memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
	PoolConfig.MetaSize = 0x1000;
	PoolConfig.BlkSize = 2*1024*1024;
	PoolConfig.BlkCnt = 3;
	PoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
	memset(PoolConfig.PartitionName,0,sizeof(PoolConfig.PartitionName));
	strcpy((char *)PoolConfig.PartitionName,"anonymous");

	UserPoolId1 = AX_POOL_CreatePool(&PoolConfig);

	if (AX_INVALID_POOLID == UserPoolId1)
	{
		printf("AX_POOL_CreatePool error!!!\n");
		goto ERROR;
	}else{
		printf("AX_POOL_CreatePool[%d] success\n",UserPoolId1);
	}

	//创建user_pool_2:blocksize=3*1024*1024,metasize=0x1000,block count =2 ,cache类型
	memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
	PoolConfig.MetaSize = 0x1000;
	PoolConfig.BlkSize = 3*1024*1024;
	PoolConfig.BlkCnt = 2;
	PoolConfig.CacheMode = POOL_CACHE_MODE_CACHED;
	memset(PoolConfig.PartitionName,0,sizeof(PoolConfig.PartitionName));
	strcpy((char *)PoolConfig.PartitionName,"anonymous");

	UserPoolId2 = AX_POOL_CreatePool(&PoolConfig);

	if (AX_INVALID_POOLID == UserPoolId2)
	{
		printf("AX_POOL_CreatePool error!!!\n");
		goto ERROR;
	}else{
		printf("AX_POOL_CreatePool[%d] success\n",UserPoolId2);
	}

	/*
	经过上述配置后，通过cat /proc/mem_cmm_info应当可以看到三个name为comm_pool_0、comm_pool_1、comm_pool_2的公共缓存池,
	以及三个name为user_pool_0、user_pool_1、user_pool_2的用户缓存池。
	*/


	/*step 5:获取Block
	*注:第一个参数为指定的PoolID, 如果设置为 AX_INVALID_POOLID,则表示从任意公共缓存池获取，也就是SetConfig->Init创建的公共缓存池。
	*/
#if 1
	/*下面演示从CreatePool接口创建的pool中获取block,第一个参数设置成UserPoolId0，表示从user_pool_0获取block*/

	BlkSize = 1*1024*1024;//期望获取的buffer大小
	BlkId = AX_POOL_GetBlock(UserPoolId0,BlkSize,NULL);

	if(BlkId == AX_INVALID_BLOCKID){
		printf("AX_POOL_GetBlock fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_GetBlock success!BlkId:0x%X\n",BlkId);
	}
#else
   /*下面演示从公共缓存池中获取block,第一个参数设置成AX_INVALID_POOLID*/

	BlkSize = 3*1024*1024;//期望获取的buffer大小
	BlkId = AX_POOL_GetBlock(AX_INVALID_POOLID,BlkSize,NULL);

	if(BlkId == AX_INVALID_BLOCKID){
		printf("AX_POOL_GetBlock fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_GetBlock success!BlkId:0x%X\n",BlkId);
	}
#endif

	/*step 6:通过Block句柄获取其PoolId*/
	PoolId = AX_POOL_Handle2PoolId(BlkId);

	if(PoolId == AX_INVALID_POOLID){
		printf("AX_POOL_Handle2PoolId fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_Handle2PoolId success!(Blockid:0x%X --> PoolId=%d)\n",BlkId,PoolId);
	}

	/*step 7:通过Block句柄获取其物理地址*/
	PhysAddr = AX_POOL_Handle2PhysAddr(BlkId);

	if(!PhysAddr){
		printf("AX_POOL_Handle2PhysAddr fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_Handle2PhysAddr success!(Blockid:0x%X --> PhyAddr=0x%llx)\n",BlkId,PhysAddr);
	}

	/*step 8:通过Block句柄获取其metadata物理地址*/
	MetaPhysAddr = AX_POOL_Handle2MetaPhysAddr(BlkId);

	if(!MetaPhysAddr){
		printf("AX_POOL_Handle2MetaPhysAddr fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_Handle2MetaPhysAddr success!(Blockid:0x%X --> MetaPhyAddr=0x%llx)\n",BlkId,MetaPhysAddr);
	}

	/*step 9:映射缓存池*/
	ret = AX_POOL_MmapPool(PoolId);

	if(ret){
		printf("AX_POOL_MmapPool fail!!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_MmapPool success!\n");
	}

	/*step 10:获取指定Block的虚拟地址以及Meta的虚拟地址*/
	blockVirAddr = AX_POOL_GetBlockVirAddr(BlkId);

	if(!blockVirAddr){
		printf("AX_POOL_GetBlockVirAddr fail!\n");
		return -1;
	}else{
		printf("AX_POOL_GetBlockVirAddr success!blockVirAddr=0x%x\n",(AX_U32)blockVirAddr);
	}

	//通过block虚拟地址操作内存
	//write data
	for(i = 0;i < 20; i++){
			((AX_S32 *)blockVirAddr)[i] = i;
	}

	//read data
	for(i = 0;i < 20; i++){
			printf("%d,",((AX_S32 *)blockVirAddr)[i]);
	}
	printf("\n");

	metaVirAddr = AX_POOL_GetMetaVirAddr(BlkId);

	if(!metaVirAddr){
		printf("AX_POOL_GetMetaVirAddr fail!\n");
    	goto ERROR;
	}else{
		printf("AX_POOL_GetMetaVirAddr success!metaVirAddr=0x%x\n",(AX_U32)metaVirAddr);
	}

	//通过meta虚拟地址操作内存
	//write data
	for(i = 0;i < 20; i++){
			((AX_S32 *)metaVirAddr)[i] = i*2;
	}

	//read data
	for(i = 0;i < 20; i++){
			printf("%d,",((AX_S32 *)metaVirAddr)[i]);
	}
	printf("\n");

	/*step 11:使用完block后，释放Block句柄*/
	ret = AX_POOL_ReleaseBlock(BlkId);

	if(ret){
		printf("AX_POOL_ReleaseBlock fail!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_ReleaseBlock success!Blockid=0x%X\n",BlkId);
	}

	/*step 12:解除映射*/
	ret = AX_POOL_MunmapPool(PoolId);

	if(ret){
		printf("AX_POOL_MunmapPool fail!!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_MunmapPool success!\n");
	}

	//step 13:按照“先进后出”原则，销毁AX_POOL_CreatePool创建的缓存池
	ret = AX_POOL_MarkDestroyPool(UserPoolId2);
	if(ret){
		printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_MarkDestroyPool success!\n");
	}

	ret = AX_POOL_MarkDestroyPool(UserPoolId1);
	if(ret){
		printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_MarkDestroyPool success!\n");
	}

	ret = AX_POOL_MarkDestroyPool(UserPoolId0);
	if(ret){
		printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n",ret);
    	goto ERROR;
	}else{
		printf("AX_POOL_MarkDestroyPool success!\n");
	}

	//step 14:清除缓存池,这一步之后，通过cat /proc/mem_cmm_info应当观察到内存都被释放了。
	ret = AX_POOL_Exit();

	if(ret){
		printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
		return -1;
	}else{
		printf("AX_POOL_Exit success!\n");
	}

	printf("\nsample_pool test end\n");
	AX_SYS_Deinit();
	return 0;

ERROR:
	printf("\nsample_pool test fail!\n");
	ret = AX_POOL_Exit();
	if(ret){
		printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
		return -1;
	}else{
		printf("AX_POOL_Exit success!\n");
	}
	AX_SYS_Deinit();
	return -1;
}
