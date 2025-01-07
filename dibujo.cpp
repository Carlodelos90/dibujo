#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

sf::Vector2f perpendicularVector(const sf::Vector2f& v) {
    return sf::Vector2f(-v.y, v.x);
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    if (length != 0)
        return sf::Vector2f(v.x / length, v.y / length);
    return v;
}

// Function to create a thick line segment
void addThickLineSegment(std::vector<sf::Vertex>& vertices, const sf::Vector2f& start, const sf::Vector2f& end, float thickness, sf::Color color) {
    sf::Vector2f direction = normalize(end - start);
    sf::Vector2f perpendicular = perpendicularVector(direction) * (thickness / 2.0f);

    sf::Vector2f p1 = start + perpendicular;
    sf::Vector2f p2 = start - perpendicular;
    sf::Vector2f p3 = end - perpendicular;
    sf::Vector2f p4 = end + perpendicular;

    vertices.push_back(sf::Vertex(p1, color));
    vertices.push_back(sf::Vertex(p2, color));
    vertices.push_back(sf::Vertex(p3, color));
    vertices.push_back(sf::Vertex(p4, color));
}

sf::Color getColorFromPosition(int x, int y, int width, int height) {
    float r = (float)x / width * 255;
    float g = (float)y / height * 255;
    float b = (float)(width - x) / width * 255;
    return sf::Color((sf::Uint8)r, (sf::Uint8)g, (sf::Uint8)b);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Drawing Tool with Features");
    window.setFramerateLimit(60);

    sf::Color brushColor = sf::Color::Blue;
    float brushThickness = 5.0f;

    sf::Color backgroundColor(200, 220, 255);

    std::vector<sf::Vertex> vertices;
    bool isDrawing = false;
    bool showColorPicker = false;

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

    // Color picker rectangle
    sf::RectangleShape colorPicker(sf::Vector2f(150, 150));
    colorPicker.setPosition(10, 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    // Check background color buttons
                    if (button1.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(255, 200, 200);
                    } else if (button2.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(200, 255, 200);
                    } else if (button3.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(200, 220, 255);
                    } else if (button4.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        backgroundColor = sf::Color(255, 255, 200);
                    }
                    else if (showColorPicker && colorPicker.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        int x = mousePos.x - (int)colorPicker.getPosition().x;
                        int y = mousePos.y - (int)colorPicker.getPosition().y;
                        brushColor = getColorFromPosition(x, y, colorPicker.getSize().x, colorPicker.getSize().y);
                    } else {
                        lastPosition = sf::Vector2f(mousePos.x, mousePos.y);
                        isDrawing = true;
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDrawing = false;
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    vertices.clear();
                } else if (event.key.code == sf::Keyboard::P) {
                    showColorPicker = !showColorPicker;
                } else if (event.key.code == sf::Keyboard::Up) {
                    brushThickness += 1.0f;
                } else if (event.key.code == sf::Keyboard::Down) {
                    brushThickness = std::max(1.0f, brushThickness - 1.0f);
                }
            }
        }

        if (isDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f currentPosition(mousePos.x, mousePos.y);
            if (lastPosition != currentPosition) {
                addThickLineSegment(vertices, lastPosition, currentPosition, brushThickness, brushColor);
                lastPosition = currentPosition;
            }
        }

        window.clear(backgroundColor);

        // Draw background color buttons
        window.draw(button1);
        window.draw(button2);
        window.draw(button3);
        window.draw(button4);

        // Draw the color picker if visible
        if (showColorPicker) {
            sf::Image colorPickerImage;
            colorPickerImage.create((unsigned int)colorPicker.getSize().x, (unsigned int)colorPicker.getSize().y);
            for (unsigned int x = 0; x < colorPickerImage.getSize().x; x++) {
                for (unsigned int y = 0; y < colorPickerImage.getSize().y; y++) {
                    colorPickerImage.setPixel(x, y, getColorFromPosition(x, y, colorPickerImage.getSize().x, colorPickerImage.getSize().y));
                }
            }

            sf::Texture colorPickerTexture;
            colorPickerTexture.loadFromImage(colorPickerImage);
            colorPicker.setTexture(&colorPickerTexture);

            window.draw(colorPicker);
        }

        if (!vertices.empty()) {
            sf::VertexArray quads(sf::Quads, vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i) {
                quads[i] = vertices[i];
            }
            window.draw(quads);
        }

        window.display();
    }

    return 0;
}
