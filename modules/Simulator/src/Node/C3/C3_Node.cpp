#include "C3_Node.hpp"


std::string C3_Node::initMessage() const{

    std::string msg=Node::initMessage();
    std::string finalMsg= msg+ "Class: "+std::to_string(getClassId())+ " started to run";
    
    sf::Packet positionPacketReceiver;
    positionPacket positionPacket(nodeId,3,coordinates,0,0);
    positionPacketReceiver<<positionPacket;
    logger.sendTcpPacket(positionPacketReceiver);

    return finalMsg;
}

void C3_Node::handleCommunication()
{

    #if COMMUNICATION_PERIOD == RRC_UPLINK
    if (currentState == NodeState::Transmitting)
        if(shouldReplyACK){

            std::vector<uint8_t> ackPacket;

            //preallocate the space for optimization
            ackPacket.reserve(common::typeACK.size() + 
                            common::senderGlobalIdBytesSize+
                            common::receiverGlobalIdBytesSize+
                            common::localIDPacketBytesSize+
                            common::hashFunctionBytesSize);

           //prepare the fields:
               std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId,common::senderGlobalIdBytesSize); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
                std::vector<uint8_t> receiverGlobalId = decimalToBytes(lastSenderId,common::receiverGlobalIdBytesSize); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
                std::vector<uint8_t> localIDPacket = decimalToBytes(lastLocalIDPacket,common::localIDPacketBytesSize); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
                std::vector<uint8_t> hashFunction = {0x00,0x00,0x00,0x00}; //Hash Function is 4 byte long in the simulation AND in real life
    
                // Append all fields
                appendVector(ackPacket, common::typeACK);
                appendVector(ackPacket, senderGlobalId);
                appendVector(ackPacket, receiverGlobalId);
                appendVector(ackPacket, localIDPacket);
                appendVector(ackPacket, hashFunction);
    
                addMessageToTransmit(ackPacket,common::timeOnAirAckPacket);
                shouldReplyACK=false;    
                logEvent("TxACK...");
                adressedPacketTransmissionDisplay(lastSenderId,true);
        }
    #endif
}

#if COMMUNICATION_PERIOD == RRC_BEACON



bool C3_Node::receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir)
{
    Log notlisteninglog("Node " + std::to_string(nodeId) + " not listening, dropped msg " /*+detailedBytesToString( message)*/, true);
    logger.logMessage(notlisteninglog);
    return false;
}

    //---------------------------state Transition--------------------
    bool C3_Node::canTransmitFromSleeping() { 

        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Transmit");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);

        currentState=NodeState::Transmitting;
        // Log transitionLog("Node "+std::to_string(nodeId)+" is in transmission Mode", true);
        // logger.logMessage(transitionLog);  
        if(beaconSlots.size()==0 && !shouldSendBeacon){
            return true;//we finished to send our beacons, but the node stays in transmission mode to not fuck up my implementation lol, would be possible to implement this behaviour but it's just simpler like this
                        //overall this simulator is far from being close to a real behavior of a node
        }
        else if(beaconSlots.size()==0 &&shouldSendBeacon){// we compute the randomness of slots (see the thesis report)
            shouldSendBeacon=false;
            beaconSlots=selectRandomSlots(computeRandomNbBeaconPackets(common::minimumNbBeaconPackets,common::maximumNbBeaconPackets),common::nbSlotsPossibleForOneBeacon);
        }

        if(beaconSlots[0]==0){


            std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime)); 



            //-------------------------------------define Beacon Packet----------------------------
            //----------least signficant byte first (little endian) !------------
                      
            std::vector<uint8_t> timeStamp = getTimeStamp(); //look how to provision it Timestamp is 4 bytes long in the simulation AND real life
            std::vector<uint8_t> costFunction = {0x00}; //Cost Function is 1 byte long in the simulation, 12 bits in real life
            std::vector<uint8_t> hopCount = {0x00,0x00}; //Hop Count is 2 byte long in the simulation, 10 bits in real life

            std::vector<uint8_t> globalIDPacket= {0x01,0x00 }; //Global ID is 2 byte long in the simulation, 10 bits in real life
            std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId,2); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
            //dummy hash: we don't implement the hash function in this simulation
            std::vector<uint8_t> hashFunction = {0x00,0x00,0x00,0x00}; //Hash Function is 4 byte long in the simulation AND in real life



            // Concatenate fields into one vector
            std::vector<uint8_t> beaconPacket;

            //preallocate the space for optimization
            //TODO: should use the size in the common file, not the variable, source of error
            beaconPacket.reserve(common::type.size() + timeStamp.size() + costFunction.size() +
                            hopCount.size() + globalIDPacket.size() +
                            senderGlobalId.size() + hashFunction.size());

            // Append all fields
            appendVector(beaconPacket, common::type);
            appendVector(beaconPacket, timeStamp);
            appendVector(beaconPacket, costFunction);
            appendVector(beaconPacket, hopCount);
            appendVector(beaconPacket, globalIDPacket);
            appendVector(beaconPacket, senderGlobalId);
            appendVector(beaconPacket, hashFunction);



            addMessageToTransmit(beaconPacket,std::chrono::milliseconds(common::timeOnAirBeacon));

            sf::Packet broadcastPacketReceiver;
            broadcastMessagePacket broadcastPacket(nodeId);
            broadcastPacketReceiver<<broadcastPacket;
            logger.sendTcpPacket(broadcastPacketReceiver);

            //erase the first element of the list
            beaconSlots.erase(beaconSlots.begin());
        }
        //decrease every elements of the slots by one
        if(beaconSlots.size()==0){
            return true;
        }
        for(int i=0;i<beaconSlots.size();i++){
            beaconSlots[i]--;
        }
        return true;
    }

    bool C3_Node::canSleepFromTransmitting() { 
        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Sleep");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);

        currentState=NodeState::Sleeping;
        // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
        // logger.logMessage(transitionLog);  
        return true; 
        }


    //Unauthorized transition in this mode.
    bool C3_Node::canTransmitFromTransmitting() { return false; }
    bool C3_Node::canTransmitFromCommunicating(){return false;}
    bool C3_Node::canTransmitFromListening() { return false; }
    bool C3_Node::canListenFromTransmitting() { return false; }
    bool C3_Node::canListenFromSleeping() { return false; }
    bool C3_Node::canListenFromListening() { return false; }
    bool C3_Node::canListenFromCommunicating(){return true;}
    bool C3_Node::canSleepFromListening() { return false; }
    bool C3_Node::canSleepFromSleeping() { return false; }
    bool C3_Node::canSleepFromCommunicating(){return false;}
    bool C3_Node::canCommunicateFromTransmitting(){return false;}
    bool C3_Node::canCommunicateFromListening(){return false;}
    bool C3_Node::canCommunicateFromSleeping(){return false;}
    bool C3_Node::canCommunicateFromCommunicating(){return false;}

#elif COMMUNICATION_PERIOD == RRC_DOWNLINK

   bool C3_Node::receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir){
            Log notlisteninglog("Node "+std::to_string(nodeId)+" not listening, dropped msg "/*+detailedBytesToString( message)*/, true);
             logger.logMessage(notlisteninglog);
            return false;
    }

    //---------------------------state Transition--------------------
    bool C3_Node::canTransmitFromSleeping() { 

        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Transmit");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);

        currentState=NodeState::Transmitting;
        // Log transitionLog("Node "+std::to_string(nodeId)+" is in transmission Mode", true);
        // logger.logMessage(transitionLog);  
        if(beaconSlots.size()==0 && !shouldSendBeacon){
            return true;//we finished to send our beacons, but the node stays in transmission mode to not fuck up my implementation lol, would be possible to implement this behaviour but it's just simpler like this
                        //overall this simulator is far from being close to a real behavior of a node
        }
        else if(beaconSlots.size()==0 &&shouldSendBeacon){// we compute the randomness of slots (see the thesis report)
            shouldSendBeacon=false;
            beaconSlots=selectRandomSlots(computeRandomNbBeaconPackets(common::minimumNbBeaconPackets,common::maximumNbBeaconPackets),common::nbSlotsPossibleForOneBeacon);
        }

        if(beaconSlots[0]==0){


            std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime)); 



            //-------------------------------------define Beacon Packet----------------------------
            //----------least signficant byte first (little endian) !------------
            std::vector<uint8_t> globalIDPacket= {0x01,0x00 }; //Global ID is 2 byte long in the simulation, 10 bits in real life
            std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId,2); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
           
            std::vector<uint8_t> receiverGlobalId = decimalToBytes(3,2); //Sender Global ID is 2 byte long in the simulation, 10 bits in real life
            std::vector<uint8_t> payload = {0xFF,0xFF,0xFF,0xFF}; //Payload Size is 4 byte long in the simulation, 40 Bytes max in real life
           
            //dummy hash: we don't implement the hash function in this simulation
            std::vector<uint8_t> hashFunction = {0x00,0x00,0x00,0x00}; //Hash Function is 4 byte long in the simulation AND in real life



            // Concatenate fields into one vector
            std::vector<uint8_t> beaconPacket;

            //preallocate the space for optimization
            //TODO: should use the size in the common file, not the variable, source of error
            beaconPacket.reserve(common::type.size() + 
                            senderGlobalId.size() +
                            receiverGlobalId.size() +
                            globalIDPacket.size() +
                            payload.size() +                            
                            + hashFunction.size());

            // Append all fields
            appendVector(beaconPacket, common::type);
            appendVector(beaconPacket, senderGlobalId);
            appendVector(beaconPacket, receiverGlobalId);
            appendVector(beaconPacket, globalIDPacket);
            appendVector(beaconPacket, payload);
            appendVector(beaconPacket, hashFunction);


            sf::Packet broadcastPacketReceiver;
            broadcastMessagePacket broadcastPacket(nodeId);
            broadcastPacketReceiver<<broadcastPacket;
            logger.sendTcpPacket(broadcastPacketReceiver);
            
            addMessageToTransmit(beaconPacket,std::chrono::milliseconds(common::timeOnAirFlood));
            //erase the first element of the list
            beaconSlots.erase(beaconSlots.begin());
        }
        //decrease every elements of the slots by one
        if(beaconSlots.size()==0){
            return true;
        }
        for(int i=0;i<beaconSlots.size();i++){
            beaconSlots[i]--;
        }
        return true;
    }

    bool C3_Node::canSleepFromTransmitting() { 

        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Sleep");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);

        currentState=NodeState::Sleeping;
        // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
        // logger.logMessage(transitionLog);  
        return true; 
        }

    //Unauthorized transition in this mode.
    bool C3_Node::canTransmitFromTransmitting() { return false; }
    bool C3_Node::canTransmitFromCommunicating(){return false;}
    bool C3_Node::canTransmitFromListening() { return false; }
    bool C3_Node::canListenFromTransmitting() { return false; }
    bool C3_Node::canListenFromSleeping() { return false; }
    bool C3_Node::canListenFromListening() { return false; }
    bool C3_Node::canListenFromCommunicating(){return true;}
    bool C3_Node::canSleepFromListening() { return false; }
    bool C3_Node::canSleepFromSleeping() { return false; }
    bool C3_Node::canSleepFromCommunicating(){return false;}
    bool C3_Node::canCommunicateFromTransmitting(){return false;}
    bool C3_Node::canCommunicateFromListening(){return false;}
    bool C3_Node::canCommunicateFromSleeping(){return false;}
    bool C3_Node::canCommunicateFromCommunicating(){return false;}

#elif COMMUNICATION_PERIOD== RRC_UPLINK

bool C3_Node::receiveMessage(const std::vector<uint8_t> message){

        //todo: could be refactored using the display functions from base ndoe class.
        //Node must listen/communicate and not ransmit  to receive a message
        if(!canNodeReceiveMessage()){
             

            sf::Packet receptionStatePacketReceiver;
            uint16_t senderId=extractBytesFromField(message,"senderGlobalId",common::dataFieldMap);
            receiveMessagePacket receptionState(senderId,nodeId,"notListening");
            receptionStatePacketReceiver<<receptionState;
            logger.sendTcpPacket(receptionStatePacketReceiver);

            return false;
        }
        //there should be no interference // reference in PhyLayer.cpp now !
    //     else if(!Node::receiveMessage(message)){
    //         //an interference happened, we don't treat the message
            
    //         sf::Packet receptionStatePacketReceiver;
    //         uint16_t senderId=extractBytesFromField(message,"senderGlobalId",common::dataFieldMap);
    //         receiveMessagePacket receptionState(senderId,nodeId,"interference");
    //         receptionStatePacketReceiver<<receptionState;
    //         logger.sendTcpPacket(receptionStatePacketReceiver);

    //         return false;
    //    } 

    //the node is able to decrypt the packet, but is it a data packet?
    if(message[0]!=common::typeData[0]){
        //not a data packet, we don't care
        
        dropAnimationDisplay();
        return false;
    }

    //We received a data packet, if it's relevant for us, we should send an ack
    uint16_t receiverId=extractBytesFromField(message,"receiverGlobalId",common::dataFieldMap);

    if(receiverId!=nodeId){
        //not for us, we don't care
       
        dropAnimationDisplay();
        return false;
    }

    //we received a packet for us, we should send an ack no matter what happened before (ack can be lost)
    shouldReplyACK=true;
    logEvent("RcvDATA will ACK");
    //we keep a memory of the packets received with their sender ID and local ID packet (specific to a link).
    lastSenderId=extractBytesFromField(message,"senderGlobalId",common::dataFieldMap);
    lastLocalIDPacket=extractBytesFromField(message,"localIDPacket",common::dataFieldMap);
    receivedPacketsId[lastSenderId].insert(lastLocalIDPacket);//duplicates are automatically ignored

    int totalCount = 0;
    
    for (const auto& pair : receivedPacketsId) {
        totalCount += pair.second.size();
    }

    //we have only one C3 node, so we can stop the simulation if we received enough packets
    //if multiple C3, a static variable for Class C3 node should be used to count the total packets received
    if(totalCount>=common::numberPacketsReceivedByC3ToStopSimulation){
        
        sf::Packet stopPacketReceiver;
        stopSimulationPacket stopPacket(nodeId);
        stopPacketReceiver<<stopPacket;
        logger.sendTcpPacket(stopPacketReceiver);

    }

    receptionStateDisplay(lastSenderId,"received");

    //We don't really care about the payload and the hash function at this stage of development
    //uint32_t payload=extractBytesFromField(message,"payload",common::dataFieldMap);
    //uint32_t hashFunction=extractBytesFromField(message,"hashFunction",common::dataFieldMap);

    return true;
}
bool C3_Node::canNodeReceiveMessage() {
        // State Condition: node must be listening to receive a message
        if(getCurrentState()!=NodeState::Listening){
            return false;
        }
        return true;
    }

    bool C3_Node::canListenFromSleeping() { 
        // logger.logMessage(Log("C3 "+std::to_string(nodeId)+" enter canListenFromSleeping()", true));

        setCurrentState( NodeState::Listening);
        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Listen");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);
        logEvent("WakeUp:Lstn");


        return true;
         }

    bool C3_Node::canSleepFromListening() { 

        setCurrentState( NodeState::Sleeping);
        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Sleep");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);
        logEvent("FallAsleep");
        return true; 
    }

    bool C3_Node::canSleepFromTransmitting() { 
        
        setCurrentState( NodeState::Sleeping);

        sf::Packet statePacketReceiver;
        stateNodePacket statePacket(nodeId, "Sleep");
        statePacketReceiver<<statePacket;
        logger.sendTcpPacket(statePacketReceiver);
        logEvent("FallAsleep");

        return true; 
        }

    bool C3_Node::canTransmitFromSleeping(){
        // logger.logMessage(Log("C3 "+std::to_string(nodeId)+" enter canTransmitFromSleeping()", true));
        setCurrentState( NodeState::Transmitting);

        // sf::Packet statePacketReceiver;
        // stateNodePacket statePacket(nodeId, "Transmit");
        // statePacketReceiver<<statePacket;
        // logger.sendTcpPacket(statePacketReceiver);
        logEvent("WakeUp:Trsmt");
        nodeStateDisplay("Transmit",std::nullopt);
        return true;
        }

    //Unauthorized transition in this mode.
    bool C3_Node::canTransmitFromTransmitting() { 
        return false; }
    bool C3_Node::canTransmitFromCommunicating(){return false;}
    bool C3_Node::canTransmitFromListening() { return false; }
    bool C3_Node::canListenFromTransmitting() { return false; }
    bool C3_Node::canListenFromListening() { 
        
        return false; }

    bool C3_Node::canListenFromCommunicating(){return false;}
    bool C3_Node::canSleepFromSleeping() { 
        
        return false; }
    bool C3_Node::canSleepFromCommunicating(){return false;}
    bool C3_Node::canCommunicateFromTransmitting(){return false;}
    bool C3_Node::canCommunicateFromListening(){return false;}
    bool C3_Node::canCommunicateFromSleeping(){return false;}
    bool C3_Node::canCommunicateFromCommunicating(){return false;}

#else
    #error "Unknown COMMUNICATION_PERIOD mode"
#endif

