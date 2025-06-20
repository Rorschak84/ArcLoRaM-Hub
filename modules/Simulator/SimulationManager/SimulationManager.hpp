#pragma once



#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "../Node/Node.hpp" // Assuming Node is declared in Node.h
#include <iostream>

class SimulationManager {
public:
    SimulationManager(double distanceThreshold,Logger& logger);
    void startSimulation();
    void stopSimulation();
    void takeOwnership(std::vector<std::shared_ptr<Node>> nodes);

    int getNbNodes() const { return nodes.size(); };

    // Starts the inter-node message dispatch loop using event-based triggering
    void startTransmissionLoop();
    void stopTransmissionLoop();

    // Function to get reachable nodes for a specific node
    std::vector<std::shared_ptr<Node>> getReachableNodesForNode(const std::shared_ptr<Node>& node);

    // Function to get reachable nodes for all nodes
    std::unordered_map<int,std::vector<std::shared_ptr<Node>>> getReachableNodesForAllNodes();
    std::condition_variable dispatchCv; // Condition variable for event-based triggering
    std::mutex dispatchCvMutex;         // Mutex associated with the condition variable
    std::vector<std::shared_ptr<Node>> nodes;//heterogeneous container of nodes (C1, C2...)
private:
    void registerNode(std::shared_ptr<Node> node);

    std::unordered_map<int, std::vector<std::shared_ptr<Node>>> reachableNodesPerNode; // Map for quick access to reachable nodes by node ID
    bool checkForMessages();
    double distanceThreshold;
    // Helper function to calculate Euclidean distance between two nodes
    double calculateDistance(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);

  Logger& logger;

void ProcessMessages();
    void transmissionLoop(); // Main loop for handling transmissions
     bool dispatchRunning;
     std::thread dispatchThread;
};
