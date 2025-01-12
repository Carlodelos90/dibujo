#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <iostream>
#include <filesystem>

enum class DrawingMode {
    FreeDraw,
    Rectangle,
    Circle,
    Text,
    Triangle
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

bool loadFontFromSystem(sf::Font& font, const std::string& fontName) {
    
    std::string fontPath = "/System/Library/Fonts/Supplemental/" + fontName;
    if (std::filesystem::exists(fontPath)) {
        return font.loadFromFile(fontPath);
    }
    return false;
}


std::shared_ptr<sf::VertexArray> makeThickLineSegment(
    const sf::Vector2f& start, 
    const sf::Vector2f& end, 
    float thickness, 
    sf::Color color)
{
    auto direction = normalize(end - start);
    auto perp = sf::Vector2f(-direction.y, direction.x) * (thickness * 0.5f);

    sf::Vector2f p1 = start + perp;
    sf::Vector2f p2 = start - perp;
    sf::Vector2f p3 = end - perp;
    sf::Vector2f p4 = end + perp;

    auto quad = std::make_shared<sf::VertexArray>(sf::Quads, 4);
    (*quad)[0] = sf::Vertex(p1, color);
    (*quad)[1] = sf::Vertex(p2, color);
    (*quad)[2] = sf::Vertex(p3, color);
    (*quad)[3] = sf::Vertex(p4, color);

    return quad;
}

sf::Color getColorFromPosition(int x, int y, int width, int height) {
    float r = (float)x / width * 255;
    float g = (float)y / height * 255;
    float b = (float)(width - x) / width * 255;
    return sf::Color((sf::Uint8)r, (sf::Uint8)g, (sf::Uint8)b);
}

sf::Color HsvToRgb(float H, float S, float V) {
    float C = S * V;
    float X = C * (1 - std::fabs(std::fmod(H / 60.f, 2.f) - 1.f));
    float m = V - C;
    float r=0, g=0, b=0;
    if(H>=0 && H<60) { r=C; g=X; }
    else if(H>=60 && H<120) { r=X; g=C; }
    else if(H>=120 && H<180) { g=C; b=X; }
    else if(H>=180 && H<240) { g=X; b=C; }
    else if(H>=240 && H<300) { r=X; b=C; }
    else { r=C; b=X; }

    return sf::Color(
        (sf::Uint8)((r+m)*255),
        (sf::Uint8)((g+m)*255),
        (sf::Uint8)((b+m)*255)
    );
}

std::string getAssetPath(const std::string& filename) {
    using std::filesystem::exists;
    std::string currentDir = std::filesystem::current_path().string();
    std::vector<std::string> possiblePaths = {
        currentDir + "/" + filename,
        "/usr/local/share/dibujo/" + filename,
        "/opt/homebrew/share/dibujo/" + filename
    };
    for(const auto& path : possiblePaths) {
        if (exists(path)) {
            return path;
        }
    }
    return "";
}




int main()
{
    sf::RenderWindow window(sf::VideoMode(1024,768), "Dibujo with Z-Stack");
    window.setFramerateLimit(60);

    sf::Cursor defaultCursor, textCursor;
    defaultCursor.loadFromSystem(sf::Cursor::Arrow);
    textCursor.loadFromSystem(sf::Cursor::Text);
    window.setMouseCursor(defaultCursor);

    
    sf::Font font;
    if(!font.loadFromFile("arial.ttf") &&
       !font.loadFromFile("../share/dibujo/arial.ttf") &&
       !loadFontFromSystem(font,"Arial.ttf"))
    {
        std::cerr<<"Failed to load font 'Arial.ttf'.\n";
        return -1;
    }

    
    sf::Texture squaresTexture, drawTexture, circleTexture, textTexture, triangleTexture;
    if(!squaresTexture.loadFromFile(getAssetPath("squares.png")) ||
       !drawTexture.loadFromFile(getAssetPath("draw.png")) ||
       !circleTexture.loadFromFile(getAssetPath("circle.png")) ||
       !textTexture.loadFromFile(getAssetPath("text.png")) ||
       !triangleTexture.loadFromFile(getAssetPath("triangle.png")))
    {
        std::cerr<<"Failed to load one or more texture files.\n";
        return -1;
    }

    
    float rainbowRadius = 20.f;
    int diameter = (int)(rainbowRadius*2.f);
    sf::Image rainbowImage;
    rainbowImage.create(diameter, diameter, sf::Color::Transparent);
    for(int y=0;y<diameter;++y) {
        for(int x=0;x<diameter;++x) {
            float dx = x - rainbowRadius;
            float dy = y - rainbowRadius;
            float dist = std::sqrt(dx*dx + dy*dy);
            if(dist<=rainbowRadius) {
                float angle = std::atan2(dy,dx);
                if(angle<0) angle+=2.f*3.14159265f;
                float hue = (angle/(2.f*3.14159265f))*360.f;
                rainbowImage.setPixel(x,y,HsvToRgb(hue,1.f,1.f));
            }
        }
    }
    sf::Texture rainbowTexture;
    rainbowTexture.loadFromImage(rainbowImage);
    sf::CircleShape rainbowButton(rainbowRadius);
    rainbowButton.setTexture(&rainbowTexture);
    rainbowButton.setPosition(window.getSize().x - diameter - 10,10);

    
    sf::Color backgroundColor(62,63,63);
    sf::Color brushColor(211,211,211);
    float brushThickness=5.f;

    std::vector<sf::Color> bgColors = {
        sf::Color(62,63,63), sf::Color(255,200,200), sf::Color(200,255,200),
        sf::Color(200,220,255), sf::Color(255,255,200), sf::Color(220,200,255),
        sf::Color(255,220,200)
    };
    int bgColorIndex=0;

    
    
    
    std::vector<std::shared_ptr<sf::Drawable>> drawStack;

    
    bool isDrawing=false;           
    bool showColorPicker=false;     
    bool isRectangleDrawing=false;  
    bool isCircleDrawing=false;     
    bool isTyping=false;            

    bool isTriangleDrawing=false;
    int triangleClickCount=0;
    sf::Vector2f trianglePoints[3];

    
    
    
    std::vector<std::shared_ptr<sf::Drawable>> currentStroke;

    sf::Vector2f lastPosition, rectangleStart, circleCenter;
    std::string typedString;
    sf::Text currentText;
    currentText.setFont(font);
    currentText.setCharacterSize(21);
    currentText.setFillColor(brushColor);

    enum class DrawingMode {FreeDraw, Rectangle, Circle, Text, Triangle} currentMode=DrawingMode::FreeDraw;

    
    sf::RectangleShape bgColorButton(sf::Vector2f(100.f,30.f));
    bgColorButton.setFillColor(sf::Color(150,150,150));
    bgColorButton.setPosition(10,10);
    sf::Text bgColorLabel("BG color", font,21);
    bgColorLabel.setFillColor(sf::Color::Black);
    bgColorLabel.setPosition(
        bgColorButton.getPosition().x + (bgColorButton.getSize().x - bgColorLabel.getLocalBounds().width)/2.f,
        bgColorButton.getPosition().y + (bgColorButton.getSize().y - bgColorLabel.getLocalBounds().height)/2.f -5.f
    );

    sf::RectangleShape squaresButton(sf::Vector2f(70.f,30.f)); 
    squaresButton.setTexture(&squaresTexture);
    squaresButton.setPosition(250.f,10.f);

    sf::RectangleShape drawButton(sf::Vector2f(70.f,30.f));
    drawButton.setTexture(&drawTexture);
    drawButton.setPosition(330.f,10.f);

    sf::RectangleShape circleButton(sf::Vector2f(70.f,30.f));
    circleButton.setTexture(&circleTexture);
    circleButton.setPosition(410.f,10.f);

    sf::RectangleShape textButton(sf::Vector2f(70.f,30.f));
    textButton.setTexture(&textTexture);
    textButton.setPosition(490.f,10.f);

    sf::RectangleShape triangleButton(sf::Vector2f(70.f,30.f));
    triangleButton.setTexture(&triangleTexture);
    triangleButton.setPosition(570.f,10.f);

    sf::RectangleShape colorPicker(sf::Vector2f(150.f,150.f));
    colorPicker.setPosition(10.f,50.f);

    sf::Clock blinkClock;

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type==sf::Event::Closed) {
                window.close();
            }
            else if(event.type==sf::Event::MouseButtonPressed) {
                if(event.mouseButton.button==sf::Mouse::Left) {
                    sf::Vector2i mousePos=sf::Mouse::getPosition(window);
                    
                    if(bgColorButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        bgColorIndex=(bgColorIndex+1)%bgColors.size();
                        backgroundColor=bgColors[bgColorIndex];
                    }
                    else if(squaresButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        currentMode=DrawingMode::Rectangle;
                    }
                    else if(drawButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        currentMode=DrawingMode::FreeDraw;
                    }
                    else if(circleButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        currentMode=DrawingMode::Circle;
                    }
                    else if(textButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        currentMode=DrawingMode::Text;
                    }
                    else if(triangleButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        currentMode=DrawingMode::Triangle;
                    }
                    else if(rainbowButton.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        showColorPicker=!showColorPicker;
                    }
                    else if(showColorPicker && colorPicker.getGlobalBounds().contains(mousePos.x,mousePos.y)) {
                        int x=mousePos.x-(int)colorPicker.getPosition().x;
                        int y=mousePos.y-(int)colorPicker.getPosition().y;
                        brushColor=getColorFromPosition(x,y,
                                      (int)colorPicker.getSize().x,
                                      (int)colorPicker.getSize().y);
                    }
                    else {
                        
                        if(currentMode==DrawingMode::Rectangle) {
                            rectangleStart=sf::Vector2f(mousePos.x,mousePos.y);
                            isRectangleDrawing=true;
                        }
                        else if(currentMode==DrawingMode::Circle) {
                            circleCenter=sf::Vector2f(mousePos.x,mousePos.y);
                            isCircleDrawing=true;
                        }
                        else if(currentMode==DrawingMode::Text && !isTyping) {
                            window.setMouseCursor(textCursor);
                            currentText.setPosition((float)mousePos.x,(float)mousePos.y);
                            currentText.setFillColor(brushColor);
                            typedString.clear();
                            currentText.setString("");
                            isTyping=true;
                        }
                        else if(currentMode==DrawingMode::Triangle) {
                            if(!isTriangleDrawing) {
                                isTriangleDrawing=true;
                                triangleClickCount=1;
                                trianglePoints[0]=sf::Vector2f(mousePos.x,mousePos.y);
                            }
                            else {
                                triangleClickCount++;
                                trianglePoints[triangleClickCount-1]=sf::Vector2f(mousePos.x,mousePos.y);
                                if(triangleClickCount==3) {
                                    
                                    auto tri = std::make_shared<sf::ConvexShape>(3);
                                    tri->setPoint(0,trianglePoints[0]);
                                    tri->setPoint(1,trianglePoints[1]);
                                    tri->setPoint(2,trianglePoints[2]);
                                    tri->setFillColor(brushColor);
                                    drawStack.push_back(tri);

                                    isTriangleDrawing=false;
                                    triangleClickCount=0;
                                }
                            }
                        }
                        else if(currentMode==DrawingMode::FreeDraw) {
                            isDrawing=true;
                            lastPosition=sf::Vector2f(mousePos.x,mousePos.y);
                            currentStroke.clear(); 
                            
                        }
                    }
                }
            }
            else if(event.type==sf::Event::MouseButtonReleased) {
                if(event.mouseButton.button==sf::Mouse::Left) {
                    if(currentMode==DrawingMode::Rectangle && isRectangleDrawing) {
                        sf::Vector2i mousePos=sf::Mouse::getPosition(window);
                        sf::Vector2f endPos((float)mousePos.x,(float)mousePos.y);
                        sf::Vector2f size(std::fabs(endPos.x-rectangleStart.x),
                                          std::fabs(endPos.y-rectangleStart.y));
                        auto rect = std::make_shared<sf::RectangleShape>(size);
                        rect->setFillColor(brushColor);
                        rect->setPosition(std::min(rectangleStart.x,endPos.x),
                                          std::min(rectangleStart.y,endPos.y));
                        drawStack.push_back(rect);
                        isRectangleDrawing=false;
                    }
                    else if(currentMode==DrawingMode::Circle && isCircleDrawing) {
                        sf::Vector2i mousePos=sf::Mouse::getPosition(window);
                        sf::Vector2f endPos((float)mousePos.x,(float)mousePos.y);
                        float radius=std::sqrt((endPos.x - circleCenter.x)*(endPos.x - circleCenter.x)
                                              +(endPos.y - circleCenter.y)*(endPos.y - circleCenter.y));
                        auto circle = std::make_shared<sf::CircleShape>(radius);
                        circle->setFillColor(brushColor);
                        circle->setOrigin(radius,radius);
                        circle->setPosition(circleCenter);
                        drawStack.push_back(circle);
                        isCircleDrawing=false;
                    }
                    else if(currentMode==DrawingMode::FreeDraw && isDrawing) {
                        
                        
                        
                        for(auto& quad : currentStroke){
                            drawStack.push_back(quad);
                        }
                        currentStroke.clear();
                        isDrawing=false;
                    }

                    if(currentMode==DrawingMode::Text) {
                        window.setMouseCursor(defaultCursor);
                    }
                }
            }
            else if(event.type==sf::Event::TextEntered && isTyping) {
                if(event.text.unicode=='\b') {
                    if(!typedString.empty()) typedString.pop_back();
                    currentText.setString(typedString);
                }
                else {
                    if(event.text.unicode=='\r') {
                        typedString+='\n';
                    }
                    else {
                        typedString+=(char)event.text.unicode;
                    }
                    currentText.setString(typedString);
                }
            }
            else if(event.type==sf::Event::KeyPressed) {
                if(isTyping && event.key.code==sf::Keyboard::Escape) {
                    
                    auto textPtr = std::make_shared<sf::Text>(currentText);
                    drawStack.push_back(textPtr);

                    isTyping=false;
                    window.setMouseCursor(defaultCursor);
                }
                else if(event.key.code==sf::Keyboard::C) {
                    
                    drawStack.clear();
                }
                else if(event.key.code==sf::Keyboard::P) {
                    showColorPicker=!showColorPicker;
                }
                else if(event.key.code==sf::Keyboard::B) {
                    bgColorIndex=(bgColorIndex+1)%bgColors.size();
                    backgroundColor=bgColors[bgColorIndex];
                }
                else if(event.key.code==sf::Keyboard::Up) {
                    brushThickness+=1.f;
                }
                else if(event.key.code==sf::Keyboard::Down) {
                    brushThickness=std::max(1.f, brushThickness-1.f);
                }
            }
        }

        
        if(isDrawing && currentMode==DrawingMode::FreeDraw) {
            sf::Vector2i mousePos=sf::Mouse::getPosition(window);
            sf::Vector2f currentPos((float)mousePos.x,(float)mousePos.y);
            if(currentPos!=lastPosition) {
                
                auto quad = makeThickLineSegment(lastPosition,currentPos,brushThickness,brushColor);
                
                currentStroke.push_back(quad);
                lastPosition=currentPos;
            }
        }

        
        window.clear(backgroundColor);

        
        for(const auto& shape : drawStack) {
            window.draw(*shape);
        }

        
        for(const auto& quad : currentStroke){
            window.draw(*quad);
        }

        
        if(isTyping) {
            window.draw(currentText);
            static sf::Clock blinkClock;
            bool cursorVisible=((int)(blinkClock.getElapsedTime().asSeconds()/0.5f)%2==0);
            if(cursorVisible) {
                sf::RectangleShape cursor(sf::Vector2f(1.f, currentText.getCharacterSize()));
                sf::FloatRect textBounds=currentText.getLocalBounds();
                cursor.setPosition(currentText.getPosition().x+textBounds.width+2.f,
                                   currentText.getPosition().y);
                cursor.setFillColor(sf::Color::Black);
                window.draw(cursor);
            }
        }

        
        if(isTriangleDrawing && triangleClickCount>0 && triangleClickCount<3) {
            sf::Vector2i mousePos=sf::Mouse::getPosition(window);
            sf::Vector2f mouseP((float)mousePos.x,(float)mousePos.y);
            int pointCount=triangleClickCount+1; 
            sf::ConvexShape preview(pointCount);
            for(int i=0;i<triangleClickCount;++i){
                preview.setPoint(i,trianglePoints[i]);
            }
            preview.setPoint(pointCount-1,mouseP);
            preview.setFillColor(sf::Color::Transparent);
            preview.setOutlineColor(sf::Color::Black);
            preview.setOutlineThickness(1.f);
            window.draw(preview);
        }

        
        if(isRectangleDrawing && currentMode==DrawingMode::Rectangle) {
            sf::Vector2i mousePos=sf::Mouse::getPosition(window);
            sf::Vector2f currPos((float)mousePos.x,(float)mousePos.y);
            sf::Vector2f topLeft(std::min(rectangleStart.x,currPos.x),
                                 std::min(rectangleStart.y,currPos.y));
            sf::Vector2f size(std::fabs(currPos.x-rectangleStart.x),
                              std::fabs(currPos.y-rectangleStart.y));
            sf::RectangleShape previewRect(size);
            previewRect.setPosition(topLeft);
            previewRect.setFillColor(sf::Color::Transparent);
            previewRect.setOutlineColor(sf::Color::Black);
            previewRect.setOutlineThickness(1.f);
            window.draw(previewRect);
        }
        
        if(isCircleDrawing && currentMode==DrawingMode::Circle) {
            sf::Vector2i mousePos=sf::Mouse::getPosition(window);
            sf::Vector2f currPos((float)mousePos.x,(float)mousePos.y);
            float radius=std::sqrt((currPos.x-circleCenter.x)*(currPos.x-circleCenter.x)
                                  +(currPos.y-circleCenter.y)*(currPos.y-circleCenter.y));
            sf::CircleShape previewC(radius);
            previewC.setOrigin(radius,radius);
            previewC.setPosition(circleCenter);
            previewC.setFillColor(sf::Color::Transparent);
            previewC.setOutlineColor(sf::Color::Black);
            previewC.setOutlineThickness(1.f);
            window.draw(previewC);
        }

        
        if(showColorPicker) {
            sf::Image colorPickerImage;
            colorPickerImage.create((unsigned int)colorPicker.getSize().x, (unsigned int)colorPicker.getSize().y);
            for(unsigned int x=0; x<colorPickerImage.getSize().x; x++){
                for(unsigned int y=0; y<colorPickerImage.getSize().y; y++){
                    colorPickerImage.setPixel(x,y,getColorFromPosition(x,y,
                       colorPickerImage.getSize().x,colorPickerImage.getSize().y));
                }
            }
            sf::Texture colorPickerTexture;
            colorPickerTexture.loadFromImage(colorPickerImage);
            colorPicker.setTexture(&colorPickerTexture);
            window.draw(colorPicker);
        }

        
        window.draw(bgColorButton);
        window.draw(bgColorLabel);
        window.draw(squaresButton);
        window.draw(drawButton);
        window.draw(circleButton);
        window.draw(textButton);
        window.draw(triangleButton);
        window.draw(rainbowButton);

        window.display();
    }

    return 0;
}
