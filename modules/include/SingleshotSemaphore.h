#ifndef PFW_SINGLESHOT_SEMAPHORE_H
#define PFW_SINGLESHOT_SEMAPHORE_H

#include <condition_variable>
#include <mutex>



  /**
  * Это удобная абстракция вокруг std :: condition_variable, которая позволяет
  * для точки однократной синхронизации. Следовательно, у семафора нет возможности
  * сбросить его состояние.
  *
  * Не имеет значения, вызывает ли ожидающий поток `wait ()` до или после
  * сигнальный поток вызывает `signal ()`. Только в последнем случае `wait ()` не будет
  * блок.
  */
class SingleshotSemaphore
{
  public:
    SingleshotSemaphore()
        : _state(false)
    {
    }

    /**
      * Блокирует вызывающий поток, пока семафор не получит асинхронный сигнал.
      * Если для семафора уже был вызван `signal ()`, он не будет заблокирован.
     */
    void wait()
    {
        std::unique_lock<std::mutex> lk(_mutex);

        while (!_state) {
            _cond.wait(lk);
        }
    }

    /**
      * Блокирует вызывающий поток на заданный период времени и продолжает либо
      * когда `signal ()` был вызван асинхронно или когда время истекло. В
      * условие возврата обозначается возвращенным логическим значением.
      *
      * \ return true, если семафор был signal () ed; ложь по истечении времени ожидания
     */
    bool waitFor(std::chrono::milliseconds ms)
    {
        std::unique_lock<std::mutex> lk(_mutex);

        if (_state) {
            return true;
        }

        _cond.wait_for(lk, ms);
        return _state;
    }

    /**
      * Разблокирует все ожидающие потоки семафора. Обратите внимание, что потоки, достигающие
      * `wait ()` на этом семафоре после вызова `signal ()` не будет
      * заблокировать, но продолжить немедленно.
     */
    void signal()
    {
        std::unique_lock<std::mutex> lk(_mutex);
        _state = true;
        _cond.notify_all();
    }

  private:
    std::mutex              _mutex;
    std::condition_variable _cond;
    bool                    _state;
};


#endif
