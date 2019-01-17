#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>
#include <string.h>

#include "DLinkedListNode.h"
#include "IntrusiveList.h"

static uint64_t _HASH_MAGIC=0xA6EE1B9999730300;

struct HashNode
{
    void* data(){return (void*)(this+1);}
    uint64_t _key;
    uint64_t _type;
    HashNode* _next;
    DLinkedListNode<HashNode> _list_hook;
};

struct HashHeader
{
    uint64_t _magic;
    void* _base_addr;
    IntrusiveList<HashNode> _used;
    size_t _bucket_size;
    HashNode* _index[0]; 
};

template<class Alloc>
class HashTable
{
    public:
        static size_t TableSize(size_t llBucketSize=3000929)
        {
            return sizeof(HashHeader)+llBucketSize*sizeof(HashNode*);
        }
        HashTable(Alloc& alloc):m_alloc(alloc){};
        HashTable(Alloc& alloc,void* pTable,size_t llBucketSize=3000929):
            m_alloc(alloc),m_llBucketSize(llBucketSize),m_pstHeader((HashHeader*)pTable)
        {
            if(m_pstHeader->_magic!=_HASH_MAGIC || 
                    m_pstHeader->_bucket_size!=llBucketSize ||
                    m_pstHeader->_base_addr!=pTable)
            {
                bzero((void*)m_pstHeader,TableSize());
                m_pstHeader->_base_addr=pTable;
                m_pstHeader->_bucket_size=llBucketSize;
                m_pstHeader->_magic=_HASH_MAGIC;
            }
        }
        ~HashTable(){};
        void* Add(uint64_t& ullKey,unsigned iType,size_t llDataSize);
        void* Set(uint64_t ullKey,unsigned iType,void* pData,size_t llDataSize);
        void* Get(uint64_t ullKey,unsigned iType);
        void* Del(uint64_t ullKey,unsigned iType);
        IntrusiveList<HashNode>& GetNodeList()
        {
            return m_pstHeader->_used; 
        }
    private:
        inline uint64_t _GetHash(uint64_t ullKey,unsigned iType)
        {
            return (ullKey+((uint64_t)iType<<32))%m_pstHeader->_bucket_size;
        }
        Alloc& m_alloc;
        size_t m_llBucketSize;
        HashHeader* m_pstHeader;
};

template<class Alloc>
void* HashTable<Alloc>::Add(uint64_t& ullKey,unsigned iType,size_t llDataSize)
{
    size_t s=sizeof(HashNode)+llDataSize;
    HashNode* n=(HashNode*)m_alloc.Alloc(s);
    if(!n)
        return NULL;
    if(ullKey==0)
        ullKey=1+((char*)n-(char*)m_alloc.Base())/s;
    n->_key=ullKey;
    n->_type=iType;
    uint64_t k=_GetHash(ullKey,iType);
    n->_next=m_pstHeader->_index[k];  
    m_pstHeader->_index[k]=n;
    m_pstHeader->_used.push_front(*n);
    n++;
    memset((char*)n,0,llDataSize);
    return n;
}
    
template<class Alloc>
void* HashTable<Alloc>::Set(uint64_t ullKey,unsigned iType,void* pData,size_t llDataSize)
{
    void* p=Get(ullKey,iType);
    if(p)
    {
        memcpy(p,pData,llDataSize);
        return p;
    }
    size_t s=sizeof(HashNode)+llDataSize;
    HashNode* n=(HashNode*)m_alloc.Alloc(s);
    if(!n)
        return NULL;
    n->_key=ullKey;
    n->_type=iType;
    uint64_t k=_GetHash(ullKey,iType);
    n->_next=m_pstHeader->_index[k];  
    m_pstHeader->_index[k]=n;
    m_pstHeader->_used.push_front(*n);
    n++;
    if(pData)
        memcpy((void*)n,pData,llDataSize);
    else
        memset((char*)n,0,llDataSize);
    return n;
}

template<class Alloc>
void* HashTable<Alloc>::Get(uint64_t ullKey,unsigned iType)
{
    uint64_t k=_GetHash(ullKey,iType);
    HashNode* n=m_pstHeader->_index[k];  
    while(n)
    {
        if(n->_key==ullKey && n->_type == iType)
            return n+1;
        else
            n=n->_next;
    }
    return NULL;
}

template<class Alloc>
void* HashTable<Alloc>::Del(uint64_t ullKey,unsigned iType)
{
    uint64_t k=_GetHash(ullKey,iType);
    HashNode* n=m_pstHeader->_index[k];  
    HashNode* p=NULL;
    while(n)
    {
        if(n->_key==ullKey && n->_type == iType)
        {
            if(p)
                p->_next=n->_next;
            else
                m_pstHeader->_index[k]=n->_next;

            m_pstHeader->_used.erase(*n);
            m_alloc.Free(n);
        }
        else
        {
            p=n;
            n=n->_next;
        }
    }
    return NULL;
}

#endif /* !HASHTABLE_H */
