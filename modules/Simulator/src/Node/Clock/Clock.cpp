#include "Clock.hpp"
#include <barrier>
void Clock::start(){
        
        /*
        begins a background thread that repeatedly calls tick() at fixed intervals (tickInterval). 
        The loop sleeps between iterations to ensure consistent timing.
        */

        running = true;
        logicalTimeMs = 0;
        compteurTick = 0;
        lastProcessedTime = 0;
        
        clockThread= std::thread([this]() {

            while (running) {
                
            
                tick();

                //if events are happening too fast, we sleep a bit
                std::this_thread::sleep_for(std::chrono::milliseconds(tickDurationMs));
            }
        });
    }

void Clock::stop(){
        running = false;
        if (clockThread.joinable()) {
        clockThread.join();
    }
    }

void Clock::tick() {
    compteurTick++;
    logicalTimeMs += tickDurationMs;


    sf::Packet tickPacketReceiver;
    tickPacket tickPacket(compteurTick);
    tickPacketReceiver<<tickPacket;
    logger.sendTcpPacket(tickPacketReceiver);
    // -----------------------------
    // Phase 1: Execute all transitions (nodes wake up, stay asleep...)
    // -----------------------------

    executeCallbacksInRange(stateTransitions, lastProcessedTime, logicalTimeMs);
    
    // -----------------------------
    // Phase 2: Execute all communication steps (nodes handle communication)
    // -----------------------------
    executeCommunicationInRange(communicationSteps, lastProcessedTime, logicalTimeMs);

    // -----------------------------
    // Phase 3: Packets transmission (deals with physical layer)
    // -----------------------------

    executeCallbacksInRange(transmissionStartCallbacks, lastProcessedTime, logicalTimeMs);
    executeCallbacksInRange(transmissionEndCallbacks, lastProcessedTime, logicalTimeMs);


    lastProcessedTime = logicalTimeMs;
    logger.setCurrentTick(logicalTimeMs);

}

void Clock::scheduleStateTransition(int64_t activationTime, CallbackType callback){
        //put the callback in the list of events at the given time
        //for one time stamp, there can multiple events (one for each node)
        //emplace and move are used to avoid copying the callback
        stateTransitions.emplace(activationTime, std::move(callback));
    }


void Clock::scheduleCommunicationStep(int64_t time, std::shared_ptr<Node> node) {
    communicationSteps.emplace(time, std::move(node));
}



void Clock::executeCommunicationInRange(
    std::multimap<int64_t, std::shared_ptr<Node>>& map,
    int64_t start, int64_t end)
{
    auto it = map.begin();
    while (it != map.end() && it->first <= end) {
        if (it->first <= start) {
        std::cerr << "❗ Event scheduled at or before already-processed time: " << it->first << " <= " << start << "\n";
    }


        if (it->first > start) {
            it->second->handleCommunication();
            logger.logEvent(it->second->getId(), "HandleComm");
            it = map.erase(it);
        } else {
            ++it;
        }
    }
}

void Clock::executeCallbacksInRange(std::multimap<int64_t, CallbackType>& map, int64_t start, int64_t end) {
    auto it = map.begin();
    // exclusive of start, inclusive of end
    while (it != map.end() && it->first <= end) {
        if (it->first <= start) {
          std::cerr << "❗ Event scheduled at or before already-processed time: " << it->first << " <= " << start << "\n";
        }
        if (it->first > start) {
            it->second(); //execute the callback
            it = map.erase(it);
        } else {
            ++it;
        }
    }
}

int64_t Clock::currentTimeInMilliseconds(){
        
        return logicalTimeMs;
    }



void Clock::scheduleTransmissionStart(int64_t time, CallbackType callback) {
    transmissionStartCallbacks.emplace(time, std::move(callback));
}

void Clock::scheduleTransmissionEnd(int64_t time, CallbackType callback) {
    transmissionEndCallbacks.emplace(time, std::move(callback));
}

const std::multimap<int64_t, std::shared_ptr<Node>> &Clock::getCommunicationSteps() const
{
    return communicationSteps;
}
