#include <list>
#include <pthread.h>
#include <iostream>

template <class V>
class ConcurrentList
{
public:
  ConcurrentList()
  {
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&emptywait, NULL);
    pthread_cond_init(&fullwait, NULL);
  }
  
  ~ConcurrentList()
  {
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&emptywait);
    pthread_cond_destroy(&fullwait);
  }
  
  ConcurrentList& operator=(const ConcurrentList& op)
  {
    if(this == &op)
      return *this;
    mList = op.mList;
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&fullwait, NULL);
    pthread_cond_init(&emptywait, NULL);
  }
  
  bool operator==(const ConcurrentList& op1) const
  {
    return mList == op1.mList;
  }
  
  void pushBack(const V& v)
  {
    pthread_mutex_lock(&mLock);
    if(mList.size() == mList.max_size())
      pthread_cond_wait(&fullwait, &mLock);
    mList.push_back(v);
    pthread_cond_signal(&emptywait);
    pthread_mutex_unlock(&mLock);
  }
  
  void pushFront(const V& v)
  {
    pthread_mutex_lock(&mLock);
    if(mList.size() == mList.max_size())
      pthread_cond_wait(&fullwait, &mLock);
    mList.push_front(v);
    pthread_cond_signal(&emptywait);
    pthread_mutex_unlock(&mLock);
  }
  
  V popBack(void)
  {
    pthread_mutex_lock(&mLock);
    if(mList.empty())
      pthread_cond_wait(&emptywait, &mLock);
    V temp = mList.back();
    mList.pop_back();
    pthread_cond_signal(&fullwait);
    pthread_mutex_unlock(&mLock);
    return temp;
  }
  
  V popFront(void)
  {
    pthread_mutex_lock(&mLock);
    if(mList.empty())
      pthread_cond_wait(&emptywait, &mLock);
    V temp = mList.front();
    mList.pop_front();
    pthread_cond_signal(&fullwait);
    pthread_mutex_unlock(&mLock);
    return temp;
  }
  
  V getFront(void) const
  {
    pthread_mutex_lock(&mLock);
    if(mList.empty())
      pthread_cond_wait(&emptywait, &mLock);
    V temp = mList.front();
    pthread_cond_signal(&fullwait);
    pthread_mutex_unlock(&mLock);
  }
   
  V getBack(void) const
  {
    pthread_mutex_lock(&mLock);
    if(mList.empty())
    {
      pthread_cond_wait(&emptywait, &mLock);
      std::cout << "In Attesa\n";
    }
    V temp = mList.back();
    pthread_cond_signal(&fullwait);
    pthread_mutex_unlock(&mLock);
  }
  
  size_t size(void) const
  {
    pthread_mutex_lock(&mLock);
    size_t s = mList.size();
    pthread_mutex_unlock(&mLock);
    return s;
  }
  
  bool empty(void) const
  {
    pthread_mutex_lock(&mLock);
    bool e = mList.empty();
    pthread_mutex_unlock(&mLock);
    return e;
  }
  
  std::_List_iterator<V> begin(void)
  {
    pthread_mutex_lock(&mLock);
    std::_List_iterator<V> iter = mList.begin();
    pthread_mutex_unlock(&mLock);
    return iter;
  }

  const std::_List_const_iterator<V> begin(void) const
  {
    pthread_mutex_lock(&mLock);
    const std::_List_const_iterator<V> iter = mList.begin();
    pthread_mutex_unlock(&mLock);
    return iter;
  }
  
  std::_List_iterator<V> end(void)
  {
    pthread_mutex_lock(&mLock);
    std::_List_iterator<V> iter = mList.end();
    pthread_mutex_unlock(&mLock);
    return iter;
  }
  
  const std::_List_iterator<V> end(void) const
  {
    pthread_mutex_lock(&mLock);
    const std::_List_iterator<V> iter = mList.end();
    pthread_mutex_unlock(&mLock);
    return iter;
  }
  
  size_t max_size() const
  {
    return mList.max_size();
  }
  
  std::_List_iterator<V> erase(std::_List_iterator<V> position)
  {
    pthread_mutex_lock(&mLock);
    std::_List_iterator<V> iter = mList.erase(position);
    pthread_mutex_unlock(&mLock);
    return iter;
  }
  
  std::_List_iterator<V> erase(std::_List_iterator<V> first, std::_List_iterator<V> last)
  {
    pthread_mutex_lock(&mLock);
    std::_List_iterator<V> iter = mList.erase(first, last);
    pthread_mutex_unlock(&mLock);
    return iter;
  }
  
  typedef std::_List_iterator<V> cIterator;
  typedef std::_List_const_iterator<V> const_cIterator;

private:
  std::list<V> mList;
  
  pthread_mutex_t mLock;
  pthread_cond_t emptywait;
  pthread_cond_t fullwait;
};