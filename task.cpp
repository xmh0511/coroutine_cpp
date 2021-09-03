#include <iostream>
#include <experimental/coroutine>
#include <thread>
#include <atomic>
struct Task{
    struct promise_type;
    using coroutine_type = std::experimental::coroutine_handle<promise_type>;
    struct promise_type{
        auto initial_suspend(){
            return std::experimental::suspend_never{};
        }
        auto final_suspend() noexcept{
            return std::experimental::suspend_always{};
        }
        auto get_return_object(){
            return Task{coroutine_type::from_promise(*this)};
        }
        static void unhandled_exception(){

        }
        auto return_value(int value){
          value_ = value;
          ready_ = true;
          std::cout<< std::this_thread::get_id() <<" return value\n";
        }
        int value_;
        std::atomic<bool> ready_ = false;
    };
    bool await_ready(){
        return false;
    }
    void await_suspend(coroutine_type h){
      // h is enclosing coroutine handle
      // this is coroutine of task 
      auto t = std::thread([this, h]() mutable{
          std::cout<<"while thread id: "<<std::this_thread::get_id()<<"\n";
          while(!this->coro_.promise().ready_);
          std::cout<<"is ready\n";
          h.resume();
      });
      t.detach();
    }
    auto await_resume(){
       return coro_.promise().value_;
    }
public:
   ~Task(){
       std::cout<<"destroy Task\n";
       coro_.destroy();
   }
private:
    Task(coroutine_type h):coro_(h){}
    coroutine_type coro_;
};
struct Line{
    bool await_ready(){
        return false;
    }
    void await_suspend(std::experimental::coroutine_handle<> h){
      auto t1 = std::thread([this,h]() mutable{
          std::this_thread::sleep_for(std::chrono::seconds(2));
          v_ = std::time(nullptr);
          std::cout<<"this thread id: "<< std::this_thread::get_id()<<"\n";
          h.resume();
          std::cout<<"back \n";
      });
      t1.detach();
    }
    auto await_resume(){
       return v_;
    }
    int v_;
};
Task task(){
   std::cout<<"in task\n";
   int r = co_await Line();
   std::cout<<"this thread id in task: "<< std::this_thread::get_id()<<"\n";
   co_return r;
}
Task task2(){
    // o is the coroutine of task
    std::cout<<"in task2\n";
    auto r = co_await task();
    std::cout<<"thread: "<< std::this_thread::get_id()<<" result: "<< r<<"\n";
    co_return r;
}
int main(){
    auto r = task2();
    std::cout<<"main\n";
    std::cin.get();
}
