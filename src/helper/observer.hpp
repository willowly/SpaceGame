#pragma once

#include <string>

using std::string;

template<typename... Args>
class Observer {    

    public:
        virtual void onEvent(string event,Args... args) = 0;

};

template<typename... Args>
class Observable {

    std::vector<Observer<Args...>*> observers;

    public:
        void addObserver(Observer<Args...>* observer) {
            observers.push_back(observer);
        }
    
    protected:
        void notify(string event,Args... args) {
            for(auto observer : observers) {
                observer->onEvent(event,args...);
            }
        }

};