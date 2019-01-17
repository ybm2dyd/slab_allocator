// #include "MemMgr.h"
// #include "XLog.h"
// #include "Server.h"
// #include "ServerDelegate.h"
// #include <stdio.h>

// static uintptr_t _MEM_BASE =0x200000000000;
// static uintptr_t _SHM_BASE =0x600000000000;


// void* MemMgr::_AllocShm(int iShmKey,size_t llMemSize,void* pBaseAddr)
// {
    // if(llMemSize<=0)
        // return NULL;
    // int shmid=0;
    // if ((shmid = shmget(iShmKey,0,0)) < 0)
    // {
        // if ((shmid = shmget(iShmKey,llMemSize, IPC_CREAT | 0777)) < 0)
            // return NULL;
    // }
    // shmid_ds dsbuf;
    // if((shmctl(shmid,IPC_STAT,&dsbuf))<0)
        // return NULL;
    // if((size_t)dsbuf.shm_segsz!=llMemSize)
    // {
        // printf("DISMATCHED SHM EXIST:%ld,%ld\n",llMemSize,(size_t)dsbuf.shm_segsz);
        // return NULL;
    // }
    // void* pAlloc=NULL;
    // if ((pAlloc=shmat(shmid,pBaseAddr,0)) == (void *) -1)
        // return NULL;
    // printf("Shard:alloc_shm(id:%lx,addr:%llx,size:%lld)\n",(long unsigned int)iShmKey,(long long unsigned int)pBaseAddr,(long long int)llMemSize);
    // return pAlloc;
// }

// void* MemMgr::_AllocMem(size_t llMemSize,void* pBaseAddr)
// {
    // if(llMemSize<=0)
        // return NULL;
    // void* r=::mmap(pBaseAddr, llMemSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    // if (r == MAP_FAILED)
    // {
        // printf("Shard:mmap_failure!\n");
        // return NULL;
    // }
    // printf("Shard:alloc_mem(addr:%llx,size:%lld)\n",(long long unsigned int)r,(long long int)llMemSize);
// #ifdef MADV_HUGEPAGE
    // ::madvise(r,llMemSize,MADV_HUGEPAGE);
// #else
    // ::madvise(r,llMemSize,0);
// #endif
    // return r;
// }


// MemMgr::MemMgr():m_hashTable(m_shmAlloc)
// {
// }

// MemMgr::~MemMgr()
// {
// }

// int MemMgr::Init()
// {
    // size_t iMaxRoomMem=sizeof(RoomInfo)*CONF->iMaxRoom;
    // size_t iTotalMem=iMaxRoomMem+1024*1024*20;
    // size_t iTotalShm=(size_t)CONF->iShardMemoryUsage*1024*1024;
    // size_t iIndexSize=HashTable<SlabAllocator>::TableSize();

    // void* pMem=_AllocMem(iTotalMem,(void*)_MEM_BASE);
    // char* pShm=(char*)_AllocShm(CONF->iCacheShmKey,iTotalShm,(void*)_SHM_BASE);

    // new((void*)&m_memAlloc) SlabAllocator(pMem,iTotalMem,10*1024*1024,1*1024,10*1024,1.5);
    // new((void*)&m_shmAlloc) SlabAllocator(pShm+iIndexSize,iTotalShm-iIndexSize,
            // 10*1024*1024,10*1024,128*2014,1.5,false);

    // new((void*)&m_hashTable) HashTable<SlabAllocator>(m_shmAlloc,pShm);

    // printf("=========================SYSTEM MEM INFO===========================\n");
    // m_shmAlloc.GetAllocatorHeader()->_dump_info();
    // printf("=========================MEM INFO===========================\n");
    // m_memAlloc.GetAllocatorHeader()->_dump_info();

    // return 0;
// }


