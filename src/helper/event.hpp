#pragma once

#include <functional>
#include <vector>


class Subscription {
    friend class BaseEvent;
    size_t index;
};

class BaseEvent {

};

template <typename... Args>
class Event : public BaseEvent {

    struct Lock {
        bool& b;
        Lock(bool& b) : b(b) {
            b = true;
        }
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        ~Lock() { b = false; }
    };

    std::vector<std::function<void(Args...)>> functions;
    bool locked = false;

    public:
        Subscription subscribe(std::function<void(Args...)> func) {
            if(locked) throw std::runtime_error("cannot subscribe inside own event");
            functions.push_back(func);
            return Subscription{functions.size()};
        }

        void unSubscribe(Subscription subscription) {
            functions[subscription.index] = {};
        }

        void operator()(const Args&... args) {
            if(locked) throw std::runtime_error("cannot call events recursively");
            Lock lock(locked);
            for(auto& func : functions) {
                if(func) {
                    func(args...);
                }
            }
        }

};