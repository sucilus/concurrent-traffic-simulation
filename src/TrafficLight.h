#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

class Vehicle;

enum TrafficLightPhase { red, green };

template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T&& msg);
private:
    std::condition_variable _condition;
    std::mutex _mutex;
    std::deque<T> _queue; 
};

class TrafficLight final : public TrafficObject
{
public:
    TrafficLight();

    TrafficLightPhase getCurrentPhase() const;
    
    void waitForGreen() const;
    void simulate() override;

private:
    void cycleThroughPhases();
    
    std::shared_ptr<MessageQueue<TrafficLightPhase>> _msgQueue;
    TrafficLightPhase _currentPhase;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif