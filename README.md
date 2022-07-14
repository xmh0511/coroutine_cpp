# coroutine_cpp

https://godbolt.org/z/5bzW3cnro

[Implementation](https://godbolt.org/z/jdb3sKT7n) of GCC and Clang regarding cpp coroutine is, it will create an object with dynamic storage duration, whose structure is shown as the following, the address of this object will be recorded in non-static data member `coroutine_handle<T>::_M_fr_ptr` of type `void*` 

| structure | bytes  |  
|  ---- | -------| 
|  coroutine-fragment-0 |  8 |
|  coroutine-fragment-1 |  8 |
| promise_type_object|  sizeof(T::promise_type)    |
| ??? |  ??? (clang has no this portion)| 
| coroutine status object|  GCC: sizeof(short), clang: sizeof(char) | 
