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
            auto to_promise = &(c.promise());
            // std::cout<<"_M_fr_ptr "<< inner <<std::endl;
            // std::cout<<"promise ptr "<<to_promise<<std::endl; 
            // std::cout<<"diff=== "<<( (char*)inner - promise_ptr)<<std::endl;
           return Ret{type::from_promise(*this)};
        }
        void unhandled_exception(){}
        void return_void(){}
        char arr[8];
    };
    type coro_;
};
struct awaiter {
     bool await_ready() const noexcept { return false; }
     void await_suspend(std::coroutine_handle<>)  {
         std::cout<<"resume\n";
     }
     void await_resume() const noexcept {}
};

struct awaiter2 {
     bool await_ready() const noexcept { return false; }
     void await_suspend(std::coroutine_handle<>)  {
         std::cout<<"resume\n";
     }
     void await_resume() const noexcept {}
};

Ret fun(){
    co_await awaiter{};
    //co_await awaiter2{};
}
int main(){
   auto f = fun();
   //std::cout<<*(void**)&(f.coro_)<<std::endl;
   auto _M_fr_ptr = *(void**)&(f.coro_);
   auto ptr = (char*)_M_fr_ptr;
   auto aptr = ptr+32;  // gcc
   //auto aptr = ptr+24; //clang
   auto nptr = (short*)aptr; // gcc
   //auto nptr = (char*)aptr; //clang
   std::cout<<(int)*nptr<<std::endl;
   *nptr = 4;  //modify the status
   f.coro_.resume();
   std::cout<<(int)*nptr<<std::endl;
   //__builtin_coro_resume(_M_fr_ptr);
}
