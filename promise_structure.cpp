#include <iostream>
#include <coroutine>
struct Ret{
    struct promise_type;
    using type = std::coroutine_handle<promise_type>;
    struct promise_type{
        auto initial_suspend(){
            return std::suspend_always{};
        }
        auto final_suspend() noexcept{
            return std::suspend_always{};
        }
        auto get_return_object(){
            std::cout<< this<<std::endl;
            auto promise_ptr = (char*)this;
            auto c = type::from_promise(*this);
            auto inner = *(void**)&(c);
            auto get_promise_from_inner = &(c.promise());
            std::cout<<"_M_fr_ptr "<< inner <<std::endl;
            std::cout<<"promise ptr "<<get_promise_from_inner<<std::endl; 
            std::cout<<"diff=== "<<( (char*)inner - promise_ptr)<<std::endl;
           return Ret{type::from_promise(*this)};
        }
        void unhandled_exception(){}
        void return_void(){}
    };
    type coro_;
};
struct awaiter {
     bool await_ready() const noexcept { return true; }
     void await_suspend(std::coroutine_handle<>)  {}
     void await_resume() const noexcept {}
};

Ret fun(){
    co_await awaiter{};
}
int main(){
   auto f = fun();
   //std::cout<<*(void**)&(f.coro_)<<std::endl;
   auto _M_fr_ptr = *(void**)&(f.coro_);
   auto promise_ptr = __builtin_coro_promise (_M_fr_ptr, __alignof(Ret::promise_type), false);
   std::cout<<"promise_ptr === " << promise_ptr<<"\n";
  auto ppp =  __builtin_coro_promise((char*) promise_ptr, __alignof(Ret::promise_type), true);
  std::cout<< ppp;
}
