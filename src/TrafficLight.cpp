#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> ulock{_mutex};
    _condition.wait(ulock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock{_mutex};
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true) {
        if(_messages.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
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
    double cycleDuration{};
    std::chrono::time_point<std::chrono::system_clock> lastUpdate{std::chrono::system_clock::now()};

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration) {
            _currentPhase = (getCurrentPhase() == TrafficLightPhase::red ? 
                TrafficLightPhase::green : TrafficLightPhase::red);

            _messages.send(std::move(getCurrentPhase()));

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<double> dis(4, 6);
            cycleDuration = dis(gen);
            lastUpdate = std::chrono::system_clock::now();
        }
    }

}

