#include "SlabAllocator.h"
#include <strings.h>


SlabAllocator::SlabAllocator(void* pMem,size_t llMemSize,
        int iSlabSize,int iMinObjectSize,int iMaxObjectSize,
        double dGrothFactor,bool bReclaimPage):
        m_pstAllocHeader((AllocatorHeader*)pMem),m_bReclaimPage(bReclaimPage)
{
    if(m_pstAllocHeader->_slab_magic!=SLAB_MAGIC+10 ||
            m_pstAllocHeader->_mem_base!=pMem ||
            m_pstAllocHeader->_mem_size!=llMemSize ||
            m_pstAllocHeader->_slab_size!=iSlabSize)
    {
        bzero(pMem,llMemSize);
        m_pstAllocHeader->_status=AllocatorHeader::SA_STATUS_ERR;
        m_pstAllocHeader->_mem_base=pMem;
        m_pstAllocHeader->_mem_size=llMemSize;
        m_pstAllocHeader->_slab_size=iSlabSize;
        m_pstAllocHeader->_slab_num=0;

        size_t size=iMinObjectSize;
        while((int)size<iMaxObjectSize)
        {
            new(&(m_pstAllocHeader->_slabs[m_pstAllocHeader->_slab_num])) SlabHeader(size);
            m_pstAllocHeader->_slab_num++;
            size*=dGrothFactor;
        }
        m_pstAllocHeader->_page_base=(void*)((char*)pMem+m_pstAllocHeader->_header_size());

        SlabPage* pPage=NULL;
        m_pstAllocHeader->_page_num=(llMemSize-m_pstAllocHeader->_header_size())/iSlabSize;
        for(int i=0;i<int(m_pstAllocHeader->_page_num);i++)
        {
            pPage=m_pstAllocHeader->_get_page(i); 
            new(pPage) SlabPage(i);
            m_pstAllocHeader->_free_slab_list.push_front(*pPage);
        }

        m_pstAllocHeader->_slab_magic=SLAB_MAGIC+10;
        m_pstAllocHeader->_stat._init_time=time(NULL);
        m_pstAllocHeader->_status=AllocatorHeader::SA_STATUS_OK;
    }
    else
    {
        m_pstAllocHeader->_status=AllocatorHeader::SA_STATUS_REUSE;
    }
}


int SlabAllocator::_SlabLowerBound(int iSize)
{
    int first=0;
    int mid=0;
    int len=m_pstAllocHeader->_slab_num; 
    int half=0;   
    while(len > 0)
    {
        half = len >> 1;
        mid=first+half;
        if(m_pstAllocHeader->_slabs[mid]._obj_size< iSize)
        {
            first=mid+1;
            len=len-half-1;
        }
        else
            len=half;
    }
    return first; 
}

int SlabAllocator::_AllocPage(SlabHeader* pHeader)
{
    SlabPage* h=m_pstAllocHeader->_free_slab_list.pop_front();
    if(!h)
        return -1;

    h->_obj_size=pHeader->_obj_size;
    h->_slab=pHeader;
    h->_free_num=0;
    uintptr_t p=(uintptr_t)(h+1);
    int n=pHeader->_obj_size+OBJECT_OFFSET;
    MObject* pObj=NULL;
    for(int i=0;i<m_pstAllocHeader->_slab_size/n;i++)
    {
        pObj=new ((void*)p)MObject(h->_slab_idx);
        h->_free_num++;
        pObj->_obj._next=h->_free_obj;
        h->_free_obj=pObj;
        p+=n;
    }
    h->_total_num=h->_free_num;
    pHeader->_used_slab_list.push_front(*h);
    return 1;
}


void* SlabAllocator::_Alloc(SlabHeader* pHeader)
{
    if(pHeader->_used_slab_list.size()==0)
        return NULL;
    MObject* o=NULL; 
    o=pHeader->_used_slab_list.begin()->_free_obj;

    pHeader->_used_slab_list.begin()->_free_obj=pHeader->_used_slab_list.begin()->_free_obj->_obj._next;
    if(--pHeader->_used_slab_list.begin()->_free_num==0)
        pHeader->_used_slab_list.pop_front();
    m_pstAllocHeader->_stat._alloc++;
    return (void*)(&(o->_obj._data));
} 

void* SlabAllocator::Alloc(int iSize)
{
    int i=_SlabLowerBound(iSize);
    if(i>=m_pstAllocHeader->_slab_num)
        return NULL;
    SlabHeader* h=&(m_pstAllocHeader->_slabs[i]);
    if(h->_used_slab_list.size()==0 && _AllocPage(h)<=0)
        return NULL;
    return _Alloc(h);
}

int SlabAllocator::_Free(SlabPage* pPage,MObject* pObj)
{
    pObj->_obj._next=pPage->_free_obj;
    pPage->_free_obj=pObj;
    pPage->_free_num++;

    if(pPage->_free_num==1)
       pPage->_slab->_used_slab_list.push_front(*pPage);
    if(m_bReclaimPage && pPage->_free_num == pPage->_total_num)
    {
        pPage->_slab->_used_slab_list.erase(*pPage);
        m_pstAllocHeader->_free_slab_list.push_front(*pPage);
    }

    m_pstAllocHeader->_stat._free++;
    return 0;
}

int SlabAllocator::Free(void* pMem)
{
    MObject* m= (MObject*)((char*)pMem - OBJECT_OFFSET);
    SlabPage* p=m_pstAllocHeader->_get_page(m->_slab_idx); 
    if(!p || p->_slab_magic!=SLAB_MAGIC)
        return -1;
    return _Free(p,m);
}

