#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
class ReceptionIcon
{

    public:
    ReceptionIcon(const sf::Vector2f& senderPos,const sf::Vector2f& receiverPos, std::string state);
    //void update();
    void draw(tgui::CanvasSFML::Ptr canvas) const;
    bool isFinished() const;

    private:
    std::optional <sf::Sprite> icon;  // Sprite for the reception state icon
    sf::Texture* interferenceTexture;
    sf::Texture* notListeningTexture;
    sf::Texture* receivedTexture;

    sf::Vector2f iconPosition;

    sf::Clock receptionClock;  // Clock for the reception icon duration

    bool receptionComplete;    // Flag to check if the reception is complete

    sf::Vector2f getPointOnLine(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos);
    bool loadTextures(const std::string& interferencePath, const std::string& notListeningPath, const std::string& receivedPath);
};