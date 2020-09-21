#include <iostream>
#include <random>
#include <future>
#include <queue>

#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_queue.empty(); });
    T lightPhase = std::move(_queue.back());

    _queue.pop_back();
    return lightPhase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(msg));
   
    _cond.notify_one();
}


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _msgQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen() const
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

		auto cur = _msgQueue->receive();
		if (cur == green)
		{
			return;
		}
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase() const
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(4, 6);

    std::unique_lock<std::mutex> lck(_mutex);
	std::cout << "Traffic_Light #" << _id << "::Cycle_Through_Phases: thread id = " << std::this_thread::get_id() << std::endl;
	lck.unlock();

    int cycleDuration = distr(eng); 
	auto lastUpdate = std::chrono::system_clock::now();

    while(1) {
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if(timeSinceLastUpdate >= cycleDuration) {
            if(_currentPhase == red) _currentPhase = green;
            else _currentPhase = red;

            auto ftr = std::async(std::launch::async, &MessageQueue<TrafficLight>::send, _msgQueue, std::move(_currentPhase));
			ftr.wait();

            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = distr(eng); 
        }
    }
}

