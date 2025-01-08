#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

enum class DrawingMode {
    FreeDraw,
    Rectangle,
    Circle,
    Text
};

sf::Vector2f perpendicularVector(const sf::Vector2f& v) {
    return sf::Vector2f(-v.y, v.x);
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y);
    if (length != 0) return sf::Vector2f(v.x / length, v.y / length);
    return v;
}

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

sf::Color HsvToRgb(float H, float S, float V) {
    float s = S, v = V;
    float C = s * v;
    float X = C * (1 - std::fabs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r = 0, g = 0, b = 0;
    if(H >= 0 && H < 60) { r = C; g = X; b = 0; }
    else if(H >= 60 && H < 120) { r = X; g = C; b = 0; }
    else if(H >= 120 && H < 180) { r = 0; g = C; b = X; }
    else if(H >= 180 && H < 240) { r = 0; g = X; b = C; }
    else if(H >= 240 && H < 300) { r = X; g = 0; b = C; }
    else { r = C; g = 0; b = X; }
    return sf::Color(
        static_cast<sf::Uint8>((r + m) * 255),
        static_cast<sf::Uint8>((g + m) * 255),
        static_cast<sf::Uint8>((b + m) * 255)
    );
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Drawing Tool with Features");
    window.setFramerateLimit(60);

    sf::Cursor defaultCursor, textCursor;
    defaultCursor.loadFromSystem(sf::Cursor::Arrow);
    textCursor.loadFromSystem(sf::Cursor::Text);
    window.setMouseCursor(defaultCursor);

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) return -1;

    float rainbowRadius = 20.0f;
    int diameter = static_cast<int>(rainbowRadius * 2);
    sf::Image rainbowImage;
    rainbowImage.create(diameter, diameter, sf::Color::Transparent);
    for(int y = 0; y < diameter; ++y) {
        for(int x = 0; x < diameter; ++x) {
            float dx = x - rainbowRadius;
            float dy = y - rainbowRadius;
            float dist = std::sqrt(dx*dx + dy*dy);
            if(dist <= rainbowRadius) {
                float angle = std::atan2(dy, dx);
                if(angle < 0) angle += 2 * 3.14159265f;
                float hue = angle / (2 * 3.14159265f) * 360.0f;
                rainbowImage.setPixel(x, y, HsvToRgb(hue, 1.0f, 1.0f));
            }
        }
    }
    sf::Texture rainbowTexture;
    rainbowTexture.loadFromImage(rainbowImage);
    sf::CircleShape rainbowButton(rainbowRadius);
    rainbowButton.setTexture(&rainbowTexture);
    rainbowButton.setPosition(window.getSize().x - diameter - 10, 10);

    // Set default background and pen colors as specified.
    sf::Color backgroundColor(62, 63, 63);
    sf::Color brushColor(211, 211, 211);
    float brushThickness = 5.0f;

    // Background colors list includes the default as the first element.
    std::vector<sf::Color> bgColors = {
        sf::Color(62,63,63), sf::Color(255, 200, 200), sf::Color(200, 255, 200),
        sf::Color(200, 220, 255), sf::Color(255, 255, 200), sf::Color(220, 200, 255), sf::Color(255, 220, 200)
    };
    int bgColorIndex = 0;

    std::vector<sf::Vertex> vertices;
    std::vector<sf::RectangleShape> drawnRectangles;
    std::vector<sf::CircleShape> drawnCircles;
    std::vector<sf::Text> drawnTexts;
    bool isDrawing = false, showColorPicker = false;
    bool isRectangleDrawing = false, isCircleDrawing = false, isTyping = false;

    sf::Vector2f lastPosition, rectangleStart, circleCenter;
    std::string typedString;
    sf::Text currentText;
    currentText.setFont(font);
    currentText.setCharacterSize(14);
    currentText.setFillColor(brushColor);

    DrawingMode currentMode = DrawingMode::FreeDraw;

    sf::RectangleShape bgColorButton(sf::Vector2f(100, 30));
    bgColorButton.setFillColor(sf::Color(150, 150, 150));
    bgColorButton.setPosition(10, 10);
    sf::Text bgColorLabel("BG color", font, 14);
    bgColorLabel.setFillColor(sf::Color::Black);
    bgColorLabel.setPosition(
        bgColorButton.getPosition().x + (bgColorButton.getSize().x - bgColorLabel.getLocalBounds().width) / 2,
        bgColorButton.getPosition().y + (bgColorButton.getSize().y - bgColorLabel.getLocalBounds().height) / 2 - 5
    );

    sf::RectangleShape squaresButton(sf::Vector2f(70, 30));
    squaresButton.setFillColor(sf::Color(180, 180, 255));
    squaresButton.setPosition(250, 10);
    sf::RectangleShape drawButton(sf::Vector2f(70, 30));
    drawButton.setFillColor(sf::Color(180, 255, 180));
    drawButton.setPosition(330, 10);
    sf::RectangleShape circleButton(sf::Vector2f(70, 30));
    circleButton.setFillColor(sf::Color(255, 180, 180));
    circleButton.setPosition(410, 10);
    sf::RectangleShape textButton(sf::Vector2f(70, 30));
    textButton.setFillColor(sf::Color(180, 180, 180));
    textButton.setPosition(490, 10);

    sf::Text squaresLabel("squares", font, 14);
    squaresLabel.setFillColor(sf::Color::Black);
    squaresLabel.setPosition(
        squaresButton.getPosition().x + (squaresButton.getSize().x - squaresLabel.getLocalBounds().width) / 2,
        squaresButton.getPosition().y + (squaresButton.getSize().y - squaresLabel.getLocalBounds().height) / 2 - 5
    );
    sf::Text drawLabel("draw", font, 14);
    drawLabel.setFillColor(sf::Color::Black);
    drawLabel.setPosition(
        drawButton.getPosition().x + (drawButton.getSize().x - drawLabel.getLocalBounds().width) / 2,
        drawButton.getPosition().y + (drawButton.getSize().y - drawLabel.getLocalBounds().height) / 2 - 5
    );
    sf::Text circleLabel("circle", font, 14);
    circleLabel.setFillColor(sf::Color::Black);
    circleLabel.setPosition(
        circleButton.getPosition().x + (circleButton.getSize().x - circleLabel.getLocalBounds().width) / 2,
        circleButton.getPosition().y + (circleButton.getSize().y - circleLabel.getLocalBounds().height) / 2 - 5
    );
    sf::Text textLabel("text", font, 14);
    textLabel.setFillColor(sf::Color::Black);
    textLabel.setPosition(
        textButton.getPosition().x + (textButton.getSize().x - textLabel.getLocalBounds().width) / 2,
        textButton.getPosition().y + (textButton.getSize().y - textLabel.getLocalBounds().height) / 2 - 5
    );

    sf::RectangleShape colorPicker(sf::Vector2f(150, 150));
    colorPicker.setPosition(10, 50);

    sf::Clock blinkClock;

    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
            else if(event.type == sf::Event::MouseButtonPressed) {
                if(event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if(bgColorButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        bgColorIndex = (bgColorIndex + 1) % bgColors.size();
                        backgroundColor = bgColors[bgColorIndex];
                    }
                    else if(squaresButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                        currentMode = DrawingMode::Rectangle;
                    else if(drawButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                        currentMode = DrawingMode::FreeDraw;
                    else if(circleButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                        currentMode = DrawingMode::Circle;
                    else if(textButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                        currentMode = DrawingMode::Text;
                    else if(rainbowButton.getGlobalBounds().contains(mousePos.x, mousePos.y))
                        showColorPicker = !showColorPicker;
                    else if(showColorPicker && colorPicker.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        int x = mousePos.x - (int)colorPicker.getPosition().x;
                        int y = mousePos.y - (int)colorPicker.getPosition().y;
                        brushColor = getColorFromPosition(x, y, (int)colorPicker.getSize().x, (int)colorPicker.getSize().y);
                    } else {
                        if(currentMode == DrawingMode::Rectangle) {
                            rectangleStart = sf::Vector2f(mousePos.x, mousePos.y);
                            isRectangleDrawing = true;
                        } else if(currentMode == DrawingMode::Circle) {
                            circleCenter = sf::Vector2f(mousePos.x, mousePos.y);
                            isCircleDrawing = true;
                        } else if(currentMode == DrawingMode::Text && !isTyping) {
                            sf::Vector2i pos = sf::Mouse::getPosition(window);
                            currentText.setPosition(static_cast<float>(pos.x), static_cast<float>(pos.y));
                            currentText.setFillColor(brushColor);
                            typedString.clear();
                            currentText.setString("");
                            isTyping = true;
                            window.setMouseCursor(textCursor);
                        } else {
                            lastPosition = sf::Vector2f(mousePos.x, mousePos.y);
                            isDrawing = true;
                        }
                    }
                }
            }
            else if(event.type == sf::Event::MouseButtonReleased) {
                if(event.mouseButton.button == sf::Mouse::Left) {
                    if(currentMode == DrawingMode::Rectangle && isRectangleDrawing) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        sf::Vector2f endPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                        sf::RectangleShape rect;
                        sf::Vector2f size(std::abs(endPosition.x - rectangleStart.x),
                                          std::abs(endPosition.y - rectangleStart.y));
                        rect.setSize(size);
                        rect.setFillColor(brushColor);
                        rect.setPosition(std::min(rectangleStart.x, endPosition.x),
                                         std::min(rectangleStart.y, endPosition.y));
                        drawnRectangles.push_back(rect);
                        isRectangleDrawing = false;
                    } else if(currentMode == DrawingMode::Circle && isCircleDrawing) {
                        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                        sf::Vector2f currentPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                        float radius = std::sqrt((currentPosition.x - circleCenter.x) * (currentPosition.x - circleCenter.x) +
                                                 (currentPosition.y - circleCenter.y) * (currentPosition.y - circleCenter.y));
                        sf::CircleShape circle(radius);
                        circle.setFillColor(brushColor);
                        circle.setOrigin(radius, radius);
                        circle.setPosition(circleCenter);
                        drawnCircles.push_back(circle);
                        isCircleDrawing = false;
                    }
                    isDrawing = false;
                    if(currentMode == DrawingMode::Text) window.setMouseCursor(defaultCursor);
                }
            }
            else if(event.type == sf::Event::TextEntered && isTyping) {
                if(event.text.unicode == '\b') {
                    if(!typedString.empty()) typedString.pop_back();
                    currentText.setString(typedString);
                } else {
                    if(event.text.unicode == '\r') {
                        typedString += '\n';
                    } else {
                        typedString += static_cast<char>(event.text.unicode);
                    }
                    currentText.setString(typedString);
                }
            }
            else if(event.type == sf::Event::KeyPressed) {
                if(isTyping && event.key.code == sf::Keyboard::Escape) {
                    drawnTexts.push_back(currentText);
                    isTyping = false;
                    window.setMouseCursor(defaultCursor);
                }
                else if(event.key.code == sf::Keyboard::C) {
                    vertices.clear();
                    drawnRectangles.clear();
                    drawnCircles.clear();
                    drawnTexts.clear();
                } else if(event.key.code == sf::Keyboard::P) {
                    showColorPicker = !showColorPicker;
                } else if(event.key.code == sf::Keyboard::B) {
                    bgColorIndex = (bgColorIndex + 1) % bgColors.size();
                    backgroundColor = bgColors[bgColorIndex];
                } else if(event.key.code == sf::Keyboard::Up) {
                    brushThickness += 1.0f;
                } else if(event.key.code == sf::Keyboard::Down) {
                    brushThickness = std::max(1.0f, brushThickness - 1.0f);
                }
            }
        }

        if(isDrawing && currentMode == DrawingMode::FreeDraw) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f currentPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            if(lastPosition != currentPosition) {
                addThickLineSegment(vertices, lastPosition, currentPosition, brushThickness, brushColor);
                lastPosition = currentPosition;
            }
        }

        window.clear(backgroundColor);

        if(!vertices.empty()) {
            sf::VertexArray quads(sf::Quads, vertices.size());
            for(size_t i = 0; i < vertices.size(); ++i) quads[i] = vertices[i];
            window.draw(quads);
        }
        for(const auto& rect : drawnRectangles) window.draw(rect);
        for(const auto& circle : drawnCircles) window.draw(circle);
        for(const auto& text : drawnTexts) window.draw(text);

        if(showColorPicker) {
            sf::Image colorPickerImage;
            colorPickerImage.create((unsigned int)colorPicker.getSize().x, (unsigned int)colorPicker.getSize().y);
            for(unsigned int x = 0; x < colorPickerImage.getSize().x; x++) {
                for(unsigned int y = 0; y < colorPickerImage.getSize().y; y++) {
                    colorPickerImage.setPixel(x, y, getColorFromPosition(x, y, colorPickerImage.getSize().x, colorPickerImage.getSize().y));
                }
            }
            sf::Texture colorPickerTexture;
            colorPickerTexture.loadFromImage(colorPickerImage);
            colorPicker.setTexture(&colorPickerTexture);
            window.draw(colorPicker);
        }

        if(currentMode == DrawingMode::Rectangle && isRectangleDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f currentPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            sf::Vector2f topLeft(std::min(rectangleStart.x, currentPosition.x), std::min(rectangleStart.y, currentPosition.y));
            sf::Vector2f size(std::abs(currentPosition.x - rectangleStart.x), std::abs(currentPosition.y - rectangleStart.y));
            sf::RectangleShape previewRect(size);
            previewRect.setPosition(topLeft);
            previewRect.setFillColor(sf::Color::Transparent);
            previewRect.setOutlineColor(sf::Color::Black);
            previewRect.setOutlineThickness(1.0f);
            window.draw(previewRect);
        }

        if(currentMode == DrawingMode::Circle && isCircleDrawing) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f currentPosition(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            float radius = std::sqrt((currentPosition.x - circleCenter.x) * (currentPosition.x - circleCenter.x) +
                                     (currentPosition.y - circleCenter.y) * (currentPosition.y - circleCenter.y));
            sf::CircleShape previewCircle(radius);
            previewCircle.setOrigin(radius, radius);
            previewCircle.setPosition(circleCenter);
            previewCircle.setFillColor(sf::Color::Transparent);
            previewCircle.setOutlineColor(sf::Color::Black);
            previewCircle.setOutlineThickness(1.0f);
            window.draw(previewCircle);
        }

        if(isTyping) {
            window.draw(currentText);
            float blinkTime = 0.5f;
            bool cursorVisible = ((int)(blinkClock.getElapsedTime().asSeconds() / blinkTime) % 2 == 0);
            if(cursorVisible) {
                sf::RectangleShape cursor(sf::Vector2f(1.0f, currentText.getCharacterSize()));
                sf::FloatRect textBounds = currentText.getLocalBounds();
                cursor.setPosition(currentText.getPosition().x + textBounds.width + 2, currentText.getPosition().y);
                cursor.setFillColor(sf::Color::Black);
                window.draw(cursor);
            }
        }

        window.draw(bgColorButton);
        window.draw(bgColorLabel);
        window.draw(squaresButton);
        window.draw(drawButton);
        window.draw(circleButton);
        window.draw(textButton);
        window.draw(rainbowButton);
        window.draw(squaresLabel);
        window.draw(drawLabel);
        window.draw(circleLabel);
        window.draw(textLabel);

        window.display();
    }

    return 0;
}
