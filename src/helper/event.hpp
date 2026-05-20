#pragma once

#include <functional>
#include <vector>


template <typename... Args>
class EventListener
{
    public:
        virtual void onEvent(Args... args)
        {
            
        }
        virtual ~EventListener() = default;
};

template <typename... Args>
class Event
{
public:

    struct Lock
    {
        bool &b;
        Lock(bool &b) : b(b)
        {
            b = true;
        }
        Lock(const Lock &) = delete;
        Lock &operator=(const Lock &) = delete;
        ~Lock() { b = false; }
    };

    std::vector<EventListener<Args...> *> listeners;
    bool locked = false;

public:
    void subscribe(EventListener<Args...> *listener)
    {
        if (locked)
            throw std::runtime_error("cannot subscribe inside own event");
        auto iter = std::find(listeners.begin(), listeners.end(), listener);
        if (iter == listeners.end())
        {
            listeners.push_back(listener);
        }
    }

    void unSubscribe(EventListener<Args...> *listener)
    {
        if (locked)
            throw std::runtime_error("cannot unsubscribe inside own event");
        auto iter = std::find(listeners.begin(), listeners.end(), listener);
        if (iter != listeners.end())
        {
            listeners.erase(iter);
        }
    }

    void operator()(Args... args)
    {
        if (locked)
            throw std::runtime_error("cannot call events recursively");
        Lock lock(locked);
        for (auto &listener : listeners)
        {
            if (listener != nullptr)
            {
                listener->onEvent(args...);
            }
        }
    }
};