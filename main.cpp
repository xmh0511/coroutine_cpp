/*
 * co_vs_callback.cpp
 *
 *  Created on: Mar 13, 2019
 *      Author: frank
 */
#include <iostream>
#include <thread>
#include <experimental/coroutine>
#include <chrono>
#include <functional>

// clang++ -std=c++2a -fcoroutines-ts -lstdc++ co_vs_callback.cpp

using call_back = std::function<void(int)>;
void Add100ByCallback(int init, call_back f) // 异步调用
{
	std::thread t([init, f]() {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		f(init + 100);
	});
	t.detach();
}

struct Add100AWaitable
{
	Add100AWaitable(int init):init_(init) {}
	bool await_ready() const { return false; }
	int await_resume() { return result_; }
	void await_suspend(std::experimental::coroutine_handle<> handle)
	{
		auto f = [handle, this](int value) mutable {
			result_ = value;
			handle.resume();
            std::cout<<"resumer\n";
		};
		Add100ByCallback(init_, f); // 调用原来的异步调用
	}
	int init_;
	int result_;
};

struct Task
{
    struct promise_type;
    using coroutine_type = std::experimental::coroutine_handle<promise_type>;
	struct promise_type {
        int current_value_;
		auto get_return_object() { return Task{coroutine_type::from_promise(*this)}; }
		auto initial_suspend() { return std::experimental::suspend_always{}; }
		auto final_suspend() { 
            std::cout<<"final_suspend\n";
            return std::experimental::suspend_never{}; 
        }
		void unhandled_exception() { std::terminate(); }
		void return_value(int v) {
            std::cout<<"return_value\n";
            current_value_ = v;
        }
	};
    coroutine_type handler_;
};

Task Add100ByCoroutine(int init)
{
    std::cout<< init<<" start invoke\n"<<std::endl;
	int ret = co_await Add100AWaitable(init);
    init = 10;
    std::cout<< init<<" ret0: "<< ret<<std::endl;
	ret = co_await Add100AWaitable(ret);
    std::cout<< init<<" ret1: "<< ret<<std::endl;
	ret = co_await Add100AWaitable(ret);
    std::cout<<init<< " ret2: "<< ret<<std::endl;
    co_return ret;
}

int main()
{
	auto r = Add100ByCoroutine(5);
    std::cout<< typeid(r).name()<<std::endl;
    r.handler_.resume();
    std::cout<<"caller 2\n";
	getchar();
}
