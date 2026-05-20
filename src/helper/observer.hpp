#pragma once

#include <string>

using std::string;

class EventObject {
    public:
        virtual ~EventObject();
};

template<typename... Args>
class Observer {    

    public:
        virtual void onEvent(string event,Args... args,EventObject* object = nullptr) = 0;

};

template<typename... Args>
class Observable {

    std::vector<Observer<Args...>*> observers;

    public:
        void addObserver(Observer<Args...>* observer) {
            auto iter = std::find(observers.begin(),observers.end(),observer);
            if(iter == observers.end()) {
                observers.push_back(observer);
            }
        }

        void removeObserver(Observer<Args...>* observer) {
            auto iter = std::find(observers.begin(),observers.end(),observer);
            if(iter != observers.end()) {
                observers.erase(iter);
            }
        }
    
    protected:
        void notify(string event,Args... args,EventObject* object = nullptr) {
            for(auto observer : observers) {
                observer->onEvent(event,args...);
            }
        }

};