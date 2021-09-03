#include <iostream>
#include <coroutine>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>
template <class T>
struct Task
{
    struct promise_type;
    using coroutine_type = std::coroutine_handle<promise_type>;
    struct promise_type
    {
        auto initial_suspend()
        {
            return std::suspend_never{};
        }
        void unhandled_exception() {}
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }

        auto get_return_object()
        {
            return Task{coroutine_type::from_promise(*this)};  // contain the handle of the enclosing coroutine;
        }

        void return_value(T const& value){
            std::unique_lock<std::mutex> lock{mutex_};
            value_ = value;  // co_return from the enclosing coroutine;
            ready_ = true;
            lock.unlock();
            condition_.notify_all();
        }
        T value_;
        std::atomic<bool> ready_ = false;
        std::condition_variable condition_;
        std::mutex mutex_;
    };
public:
     // for co_await in other context
     bool await_ready(){
         return coro_.promise().ready_;
     }
     void await_suspend(std::coroutine_handle<> h){
          thread_ = std::make_unique<std::thread>([this, h]() mutable{
              auto&& promise_v = coro_.promise();
              std::unique_lock<std::mutex> lock{promise_v.mutex_};
              promise_v.condition_.wait(lock, [&promise_v]()->bool{
                  return promise_v.ready_;
              });
              h.resume();
          });  
     }
     auto await_resume(){
         return coro_.promise().value_;
     }
public:
    Task(Task const& v):coro_(v.coro_){
       
    }
    ~Task(){
        if(thread_ !=nullptr && thread_->joinable()){
            thread_->detach();
        }
        coro_.destroy();
    }
private:
    Task(coroutine_type h):coro_(h){}
    coroutine_type coro_;
    std::unique_ptr<std::thread> thread_ = nullptr;
};
