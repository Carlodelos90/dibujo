#include "LogoManager.hpp"
#include <iostream>

namespace LogoManager
{
    bool setWindowIcon(sf::RenderWindow& window, const std::string& logoPath)
    {
        sf::Image icon;
        if (!icon.loadFromFile(logoPath))
        {
            std::cerr << "Failed to load icon from " << logoPath << std::endl;
            return false;
        }
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        return true;
    }
}
