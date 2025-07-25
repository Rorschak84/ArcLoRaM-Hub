#include "RrcUplinkNodeFactory.hpp"
#include "../../../Node/C3/C3_Node.hpp"
#include "../../../Node/C2/C2_Node.hpp"
#include "../../../Node/C1/C1_Node.hpp"
#include <stdexcept>

RrcUplinkNodeFactory::RrcUplinkNodeFactory(Logger& logger)
    : logger(logger) {}

std::shared_ptr<C3_Node> RrcUplinkNodeFactory::createC3Node(int id, std::pair<int, int> coordinates) {
    auto node = std::make_shared<C3_Node>(id, logger, coordinates);

    // TDMA setup for RRC_UPLINK mode 
    for (size_t i = 0; i < common::totalNumberOfSlots; i++) {
        node->addActivation( (i + 1) * common::durationSleepWindowMain + i * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanListen);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanSleep);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanTransmit);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + (i + 1) * common::durationACKWindow, WindowNodeState::CanSleep);
    }

    return node;
}

std::shared_ptr<C2_Node> RrcUplinkNodeFactory::createC2Node(int id, std::pair<int, int> coordinates, int nextHop, int hopCount) {
    auto node = std::make_shared<C2_Node>(id, logger, coordinates, nextHop, hopCount);

    // TDMA setup for RRC_UPLINK mode
    for (size_t i = 0; i < common::totalNumberOfSlots; i++) {
        node->addActivation( (i + 1) * common::durationSleepWindowMain + i * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanCommunicate);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + i * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanSleep);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + i * common::durationACKWindow, WindowNodeState::CanCommunicate);
        node->addActivation( (i + 1) * common::durationSleepWindowMain + (i + 1) * common::durationDataWindow + (i + 1) * common::durationSleepWindowSecondary + (i + 1) * common::durationACKWindow, WindowNodeState::CanSleep);
    }

    return node;
}

std::shared_ptr<C1_Node> RrcUplinkNodeFactory::createC1Node(int, std::pair<int, int>) {
    throw std::logic_error("C1 nodes are not supported in RRC_UPLINK mode.");
}
