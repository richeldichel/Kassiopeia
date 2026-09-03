#ifndef KSMUTEX_H_
#define KSMUTEX_H_

#include <pthread.h>

namespace Kassiopeia
{

class KSMutex
{
  public:
    KSMutex();
    virtual ~KSMutex();

    bool Trylock();

    void Lock();
    void Unlock();

  private:
    pthread_mutex_t fMutex;
    
    friend class KSMutexLock;
};

// RAII-style lock guard for exception-safe mutex management
class KSMutexLock
{
  public:
    explicit KSMutexLock(KSMutex& mutex) : fMutex(mutex)
    {
        fMutex.Lock();
    }
    
    ~KSMutexLock()
    {
        fMutex.Unlock();
    }
    
    // Prevent copying
    KSMutexLock(const KSMutexLock&) = delete;
    KSMutexLock& operator=(const KSMutexLock&) = delete;
    
  private:
    KSMutex& fMutex;
};

}  // namespace Kassiopeia

#endif
