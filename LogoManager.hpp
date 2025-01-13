#ifndef LOGOMANAGER_HPP
#define LOGOMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <string>

namespace LogoManager
{
    bool setWindowIcon(sf::RenderWindow& window, const std::string& logoPath);
}

#endif
