#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

// Enumeration for drawing modes
enum class DrawingMode {
    FreeDraw,
    Rectangle
};

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
void addThickLineSegment(std::vector<sf::Vertex>& vertices, 
                         const sf::Vector2f& start, 
                         const sf::Vector2f& end, 
                         float thickness, sf::Color color) {
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

    // Load a font for button labels
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1; // Exit if font can't be loaded
    }

    sf::Color brushColor = sf::Color::Blue;
    float brushThickness = 5.0f;
    sf::Color backgroundColor(200, 220, 255);

    std::vector<sf::Vertex> vertices;
    std::vector<sf::RectangleShape> drawnRectangles;

    bool isDrawing = false;
    bool showColorPicker = false;
    bool isRectangleDrawing = false;

    sf::Vector2f lastPosition;
    sf::Vector2f rectangleStart;

    DrawingMode currentMode = DrawingMode::FreeDraw;

    // Create background color buttons
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

    // Create mode switching buttons
    sf::RectangleShape squaresButton(sf::Vector2f(70, 30));
    squaresButton.setFillColor(sf::Color(180, 180, 255));
    squaresButton.setPosition(250, 10);

    sf::RectangleShape drawButton(sf::Vector2f(70, 30));
    drawButton.setFillColor(sf::Color(180, 255, 180));
    drawButton.setPosition(330, 10);

    // Create text labels for the buttons
    sf::Text squaresLabel("squares", font, 14);
    squaresLabel.setFillColor(sf::Color::Black);
    // Center the text on the squares button
    squaresLabel.setPosition(
        squaresButton.getPosition().x + (squaresButton.getSize().x - squaresLabel.getLocalBounds().width) / 2,
        squaresButton.getPosition().y + (squaresButton.getSize().y - squaresLabel.getLocalBounds().height) / 2 - 5
    );

    sf::Text drawLabel("draw", font, 14);
    drawLabel.setFillColor(sf::Color::Black);
    // Center the text on the draw button
    drawLabel.setPosition(
        drawButton.getPosition().x + (drawButton.getSize().x - drawLabel.getLocalBounds().width) / 2,
        drawButton.getPosition().y + (drawButton.getSize().y - drawLabel.getLocalBounds().height) / 2 - 5
    );

    // Color picker rectangle
    sf::RectangleShape colorPicker(sf::Vector2f(150, 150));
    colorPicker.setPosition(10, 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } 
            else if (event.type == sf::Event::MouseButtonPressed) {
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
                    // Check mode buttons
                    else if (squaresButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        currentMode = DrawingMode::Rectangle;
                    } else if (drawButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        currentMode = DrawingMode::FreeDraw;
                    }
                    // Check color picker interaction
                    else if (showColorPicker && colorPicker.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        int x = mousePos.x - (int)colorPicker.getPosition().x;
                        int y = mousePos.y - (int)colorPicker.getPosition().y;
                        brushColor = getColorFromPosition(x, y, (int)colorPicker.getSize().x, (int)colorPicker.getSize().y);
                    } 
                    else {
                        if (currentMode == DrawingMode::Rectangle) {
                            rectangleStart = sf::Vector2f(mousePos.x, mousePos.y);
                            isRectangleDrawing = true;
                        } else {  // Free draw mode
                            lastPosition = sf::Vector2f(mousePos.x, mousePos.y);
                            isDrawing = true;
                        }
                    }
                }
            } 
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (currentMode == DrawingMode::Rectangle && isRectangleDrawing) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        sf::Vector2f endPosition(mousePos.x, mousePos.y);
                        sf::RectangleShape rect;
                        sf::Vector2f size(
                            std::abs(endPosition.x - rectangleStart.x),
                            std::abs(endPosition.y - rectangleStart.y)
                        );
                        rect.setSize(size);
                        rect.setFillColor(brushColor);
                        rect.setPosition(
                            std::min(rectangleStart.x, endPosition.x),
                            std::min(rectangleStart.y, endPosition.y)
                        );
                        drawnRectangles.push_back(rect);
                        isRectangleDrawing = false;
                    }
                    isDrawing = false;
                }
            } 
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    vertices.clear();
                    drawnRectangles.clear();
                } else if (event.key.code == sf::Keyboard::P) {
                    showColorPicker = !showColorPicker;
                } else if (event.key.code == sf::Keyboard::Up) {
                    brushThickness += 1.0f;
                } else if (event.key.code == sf::Keyboard::Down) {
                    brushThickness = std::max(1.0f, brushThickness - 1.0f);
                }
            }
        }

        if (isDrawing && currentMode == DrawingMode::FreeDraw) {
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

        // Draw mode switching buttons and labels
        window.draw(squaresButton);
        window.draw(drawButton);
        window.draw(squaresLabel);
        window.draw(drawLabel);

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

        // Draw freehand lines
        if (!vertices.empty()) {
            sf::VertexArray quads(sf::Quads, vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i) {
                quads[i] = vertices[i];
            }
            window.draw(quads);
        }

        // Draw all finalized rectangles
        for (const auto& rect : drawnRectangles) {
            window.draw(rect);
        }

        window.display();
    }

    return 0;
}
