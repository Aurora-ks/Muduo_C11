#pragma once

#include <mutex>
#include <memory>
#include "nocopyable.h"

template<class T>
class Singleton : nocopyable
{
public:
    static std::shared_ptr<T> GetInstance();
protected:
    ~Singleton() = default;
    Singleton() = default;
private:
    static std::shared_ptr<T> instance_;
};

template <class T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

template <class T>
std::shared_ptr<T> Singleton<T>::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&](){
        instance_ = std::shared_ptr<T>(new T);
    });
    return instance_;
}
