#pragma once

#include "../Factories/INodeFactory.hpp"
#include "../../Node/Node.hpp"
#include "../../Connectivity/Logger/Logger.hpp"
#include <memory>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <unordered_set>

class DeploymentManager {
public:
    DeploymentManager(Logger& logger,
                      std::condition_variable& dispatchCv,
                      std::mutex& dispatchCvMutex,
                      uint64_t baseTime);

    std::vector<std::shared_ptr<Node>> loadDeploymentFromFile(const std::string& filename);

private:
    Logger& logger;
    std::condition_variable& dispatchCv;
    std::mutex& dispatchCvMutex;
    uint64_t baseTime;

    common::CommunicationMode parseModeLine(const std::string& line);

    // Mode-agnostic dispatcher
    void parseLine(const std::string& line,
                   INodeFactory& factory,
                   std::vector<std::shared_ptr<Node>>& nodes,
                   std::unordered_set<int>& nodeIds,
                   common::CommunicationMode mode);

    // Mode-specific parsing handlers
    void parseLineRrcUplink(const std::string& line, INodeFactory& factory,
                            std::vector<std::shared_ptr<Node>>& nodes,
                            std::unordered_set<int>& nodeIds);

    void parseLineRrcDownlink(const std::string& line, INodeFactory& factory,
                              std::vector<std::shared_ptr<Node>>& nodes,
                              std::unordered_set<int>& nodeIds);

    void parseLineEncUplink(const std::string& line, INodeFactory& factory,
                            std::vector<std::shared_ptr<Node>>& nodes,
                            std::unordered_set<int>& nodeIds);

    // etc. for other modes
};
