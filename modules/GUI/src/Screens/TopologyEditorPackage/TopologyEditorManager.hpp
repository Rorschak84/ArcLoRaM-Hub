#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "TopologyEditorState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include "../../UI/Dropdown/Dropdown.hpp"
#include "../../UI/Button/Button.hpp"
#include "../../Visualisation/Animations/BroadcastAnimation/BroadcastAnimation.hpp"
#include "../../UI/TypeableInput/TypeableInput.hpp" 


class TopologyEditorManager {

public:
    TopologyEditorManager(TopologyEditorState& state);
    void handleInput( InputManager& input);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window,sf::View& editorView);
    void setSelectedNode(std::optional<int> nodeId);
    std::optional<int> getSelectedNode() const;

    sf::RectangleShape convertRectangleToTopologyView(
    const sf::RenderWindow& window,
    const sf::View& topologyView,
    const sf::RectangleShape& screenRect
);


bool isBoundsFullyInsideRect(
    const sf::Vector2f& position,  // top-left corner
    const sf::Vector2f& size,
    const sf::FloatRect& rect
);



private:

    std::optional<sf::Text> coordText;
    std::optional<int> selectedNodeId;
 

    Dropdown<TopologyMode> modeDropdown;
    TopologyEditorState& state;
    sf::RectangleShape topologyBounds;

    std::unique_ptr<Button> saveButton;
    std::unique_ptr<Button> addLinkButton;
    std::unique_ptr<Button> cutNodeButton;  
    std::unique_ptr<Button> cutLinkButton;
    std::unique_ptr<Button> addNodeC1Button;
    std::unique_ptr<Button> addNodeC2Button;
    std::unique_ptr<Button> addNodeC3Button;
    std::unique_ptr<Button> moveNodeButton;

    std::unique_ptr<TypeableInput> typeableInput;

    //broadcast animations
    void startBroadcast(const sf::Vector2f& startPosition, float duration);
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations; 
    void drawRootings(sf::RenderWindow& window);




    
};



