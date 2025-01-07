#include <SFML/Graphics.hpp>
#include <vector>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Drawing Tool");
    window.setFramerateLimit(60);

    sf::Color brushColor = sf::Color::Blue;
    float brushSize = 5.0f;

    sf::Color backgroundColor(200, 220, 255);

    std::vector<sf::Vertex> lines;

    bool isDrawing = false;
    sf::Vector2f lastPosition;

    sf::RectangleShape button1(sf::Vector2f(50, 30));
    button1.setFillColor(sf::Color(255, 200, 200));
    button1.setPosition(10, 10);

    sf::RectangleShape button2(sf::Vector2f(50, 30));
    button2.setFillColor(sf::Color(200, 255, 200));
    button2.setPosition(70, 10);

    sf::RectangleShape button3(sf::Vector2f(50, 30));
    button3.setFillColor(sf::Color(200, 220, 255));
    button3.setPosition(130, 10);

    sf::RectangleShape button4(sf::Vector2f(50, 30));
    button4.setFillColor(sf::Color(255, 255, 200));
    button4.setPosition(190, 10);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (button1.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(255, 200, 200);
                    } else if (button2.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(200, 255, 200);
                    } else if (button3.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(200, 220, 255);
                    } else if (button4.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(255, 255, 200);
                    } else {
                        isDrawing = true;
                        lastPosition = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDrawing = false;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    lines.clear();
                }
            }
        }

        if (isDrawing) {
            sf::Vector2f currentPosition = sf::Vector2f(sf::Mouse::getPosition(window));
            lines.push_back(sf::Vertex(lastPosition, brushColor));
            lines.push_back(sf::Vertex(currentPosition, brushColor));
            lastPosition = currentPosition;
        }

        window.clear(backgroundColor);
        window.draw(button1);
        window.draw(button2);
        window.draw(button3);
        window.draw(button4);
        window.draw(&lines[0], lines.size(), sf::Lines);
        window.display();
    }

    return 0;
}
