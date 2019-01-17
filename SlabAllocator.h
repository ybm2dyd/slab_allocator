#ifndef SLABALLOCATOR_H
#define SLABALLOCATOR_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include "DLinkedListNode.h"
#include "IntrusiveList.h"

template<typename T> struct alignment_trick { char c; T member; };

#define _OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
#define _ALIGNOF(type) offsetof (alignment_trick<type>, member)

static uint64_t SLAB_MAGIC=0xA6DD1B9665730300;
//static uint64_t ALLOCATOR_BASE=0x200000000000;

struct MObject
{
    MObject(int idx):_slab_idx(idx)
    {
        _obj._next=NULL;
    }
    int64_t _slab_idx;
    union _OBJ
    {
        MObject* _next;    
        char _data[1];
    }_obj;
};

//static int OBJECT_OFFSET =_ALIGNOF(MObject::_OBJ);
static int OBJECT_OFFSET __attribute__((unused)) =_OFFSETOF(MObject,_obj);

struct SlabHeader;
struct SlabPage
{
    SlabPage(int iIdx)
    {
        _slab_magic=SLAB_MAGIC;
        _slab_idx=iIdx;
        _free_obj=NULL;
        _obj_size=0;
        _free_num=0;
        _total_num=0;
    }
    uint64_t _slab_magic;
    int _slab_idx;
    int _obj_size;
    int _free_num;
    int _total_num;
    DLinkedListNode<SlabPage> _list_hook;
    SlabHeader* _slab;
    MObject* _free_obj;
};

struct SlabHeader
{
    SlabHeader(int iObjSize):_obj_size(iObjSize){};
    int _obj_size;
    IntrusiveList<SlabPage> _used_slab_list;
};

struct AllocatorHeader
{
    enum{SA_STATUS_OK=0,SA_STATUS_REUSE=1,SA_STATUS_ERR=-1};
    size_t _header_size()
    {
        return sizeof(AllocatorHeader)+_slab_num*sizeof(SlabHeader);
    }

    int _free_page_num()
    {
        int n=0;
        for(int i=0;i<_slab_num;i++)
            n+=_slabs[i]._used_slab_list.size();
        return _page_num-n; 
    }

    SlabPage* _get_page(unsigned int uIdx)
    {
        if(uIdx<_page_num)
            return (SlabPage*)((char*)_page_base+uIdx*(sizeof(SlabPage)+_slab_size));
        return NULL;
    }

    void _dump_info()
    {
        printf("magic:%lx,status:%ld,mem:%uMB\n",_slab_magic,_status,_slab_size*_page_num/1024/1024);
        printf("base:%p,page:%p,(%d/%d)\n",_mem_base,_page_base,_free_slab_list.size(),_page_num);
        printf("slab_size:%d,slab_num:%d\n",_slab_size,_slab_num);
    }

    uint64_t _slab_magic;
    void* _mem_base;
    void* _page_base;
    size_t _mem_size;
    int _slab_size;
    int _slab_num;
    unsigned int _page_num;
    IntrusiveList<SlabPage> _free_slab_list;
    struct 
    {
        uint64_t _init_time;
        uint64_t _alloc;
        uint64_t _free;
    }_stat;
    int64_t _status;
    SlabHeader _slabs[0];
};

class SlabAllocator
{
    public:
        SlabAllocator(){};
        SlabAllocator(void* pMem,size_t llMemSize,
                int iSlabSize,int iMinObjectSize,int iMaxObjectSize,
                double dGrothFactor,bool bReclaimPage=true);
        void* Alloc(int iSize);
        int Free(void* pMem);
        AllocatorHeader* GetAllocatorHeader(){return m_pstAllocHeader;}
        void* Base(){return m_pstAllocHeader->_mem_base;}
    private:
        int _SlabLowerBound(int iSize);
        int _AllocPage(SlabHeader* pHeader);
        int _Free(SlabPage* pPage,MObject* pObj);
        void* _Alloc(SlabHeader* pHeader);
        AllocatorHeader* m_pstAllocHeader;
        bool m_bReclaimPage;
};

#endif /* !SLABALLOCATOR_H */
