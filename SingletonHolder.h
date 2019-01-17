#ifndef  SINGLETONHOLDER_INC
#define  SINGLETONHOLDER_INC

template<class T>
class SingletonHolder
{
public:
    static T* Instance()
    {
        if(!pInstance_)
            pInstance_=new T;
        return pInstance_;
    }
private:
    SingletonHolder(){};
    SingletonHolder(const SingletonHolder&);
    SingletonHolder& operator=(const SingletonHolder&);
    ~SingletonHolder(){};
    static __thread T* pInstance_;
};

template<class T>
__thread T* SingletonHolder<T>::pInstance_=0;

template<class T>
class GlobalSingleton
{
public:
    static T* Instance()
    {
        if(!pInstance_)
            pInstance_=new T;
        return pInstance_;
    }
private:
    GlobalSingleton(){};
    GlobalSingleton(const GlobalSingleton&);
    GlobalSingleton& operator=(const GlobalSingleton&);
    ~GlobalSingleton(){};
    static T* pInstance_;
};

template<class T>
T* GlobalSingleton<T>::pInstance_=0;

#endif   /* ----- #ifndef SINGLETONHOLDER_INC  ----- */
