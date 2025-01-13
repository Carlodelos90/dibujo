#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <iostream>
#include <filesystem>
#include "LogoManager.hpp"

enum class DrawingMode {
    FreeDraw,
    Rectangle,
    Circle,
    Text,
    Triangle,
    Rainbow,
    PaintBucket
};

sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = std::sqrt(v.x*v.x + v.y*v.y);
    if(length != 0) return sf::Vector2f(v.x/length, v.y/length);
    return v;
}

bool loadFontFromSystem(sf::Font& font, const std::string& fontName) {
    std::string path = "/System/Library/Fonts/Supplemental/" + fontName;
    if(std::filesystem::exists(path)) {
        return font.loadFromFile(path);
    }
    return false;
}

std::shared_ptr<sf::VertexArray> makeThickLineSegment(
    const sf::Vector2f& start,
    const sf::Vector2f& end,
    float thickness,
    sf::Color color)
{
    auto d = normalize(end - start);
    auto perp = sf::Vector2f(-d.y, d.x)*(thickness*0.5f);
    sf::Vector2f p1 = start + perp;
    sf::Vector2f p2 = start - perp;
    sf::Vector2f p3 = end - perp;
    sf::Vector2f p4 = end + perp;

    auto quad = std::make_shared<sf::VertexArray>(sf::Quads,4);
    (*quad)[0] = sf::Vertex(p1,color);
    (*quad)[1] = sf::Vertex(p2,color);
    (*quad)[2] = sf::Vertex(p3,color);
    (*quad)[3] = sf::Vertex(p4,color);
    return quad;
}

sf::Color getColorFromPosition(int x,int y,int w,int h) {
    float r=(float)x/w*255.f;
    float g=(float)y/h*255.f;
    float b=(float)(w-x)/w*255.f;
    return sf::Color((sf::Uint8)r,(sf::Uint8)g,(sf::Uint8)b);
}

sf::Color HsvToRgb(float H,float S,float V) {
    float C=S*V;
    float X=C*(1.f-std::fabs(std::fmod(H/60.f,2.f)-1.f));
    float m=V-C;
    float r=0,g=0,b=0;
    if(H>=0&&H<60){r=C;g=X;}
    else if(H>=60&&H<120){r=X;g=C;}
    else if(H>=120&&H<180){g=C;b=X;}
    else if(H>=180&&H<240){g=X;b=C;}
    else if(H>=240&&H<300){r=X;b=C;}
    else{r=C;b=X;}
    return sf::Color(
        (sf::Uint8)((r+m)*255),
        (sf::Uint8)((g+m)*255),
        (sf::Uint8)((b+m)*255)
    );
}

std::string getAssetPath(const std::string& name) {
    using std::filesystem::exists;
    std::string c = std::filesystem::current_path().string();
    std::vector<std::string> paths = {
        c+"/"+name,
        "/usr/local/share/dibujo/"+name,
        "/opt/homebrew/share/dibujo/"+name
    };
    for(auto& p:paths){
        if(exists(p)) return p;
    }
    return "";
}

int main() {
    sf::RenderWindow window(sf::VideoMode(1024,768),"Dibujo");
    window.setFramerateLimit(60);

    LogoManager::setWindowIcon(window, "logo.png");

    sf::Cursor arrowCursor, textCursor;
    arrowCursor.loadFromSystem(sf::Cursor::Arrow);
    textCursor.loadFromSystem(sf::Cursor::Text);
    window.setMouseCursor(arrowCursor);

    sf::Font font;
    if(!font.loadFromFile("arial.ttf") &&
       !font.loadFromFile("../share/dibujo/arial.ttf") &&
       !loadFontFromSystem(font,"Arial.ttf"))
    {
        std::cerr<<"Failed to load font.\n";
        return -1;
    }

    sf::Texture squaresT, drawT, circleT, textT, triangleT, rainbowModeT, bucketT;
    if(!squaresT.loadFromFile(getAssetPath("squares.png")) ||
       !drawT.loadFromFile(getAssetPath("draw.png")) ||
       !circleT.loadFromFile(getAssetPath("circle.png")) ||
       !textT.loadFromFile(getAssetPath("text.png")) ||
       !triangleT.loadFromFile(getAssetPath("triangle.png")) ||
       !rainbowModeT.loadFromFile(getAssetPath("rainbow.png")) ||
       !bucketT.loadFromFile(getAssetPath("bucket.png")))
    {
        std::cerr<<"Failed to load textures.\n";
        return -1;
    }

    float rainbowRadius=20.f;
    int diam=(int)(rainbowRadius*2.f);
    sf::Image rImg;
    rImg.create(diam,diam,sf::Color::Transparent);
    for(int y=0;y<diam;y++){
        for(int x=0;x<diam;x++){
            float dx=x-rainbowRadius,dy=y-rainbowRadius;
            float dist=std::sqrt(dx*dx+dy*dy);
            if(dist<=rainbowRadius){
                float angle=std::atan2(dy,dx);
                if(angle<0)angle+=6.2831853f;
                float hue=angle/(6.2831853f)*360.f;
                rImg.setPixel(x,y,HsvToRgb(hue,1.f,1.f));
            }
        }
    }
    sf::Texture colorWheelT;
    colorWheelT.loadFromImage(rImg);
    sf::CircleShape colorWheel(rainbowRadius);
    colorWheel.setTexture(&colorWheelT);
    colorWheel.setPosition(1024-(float)diam-10.f,10.f);

    sf::Color bgc(62,63,63),brush(211,211,211);
    float thick=5.f;
    std::vector<sf::Color> bgColors={
        sf::Color(62,63,63),sf::Color(255,200,200),sf::Color(200,255,200),
        sf::Color(200,220,255),sf::Color(255,255,200),sf::Color(220,200,255),
        sf::Color(255,220,200)
    };
    int bgIndex=0;
    std::vector<std::shared_ptr<sf::Drawable>> stack;

    bool isDrawing=false, showPicker=false, isRect=false, isCircle=false,
         isTyping=false, isTri=false, rainbowModeActive=false;
    float rainbowHue=0.f;

    int triClicks=0;
    sf::Vector2f triPts[3];
    std::vector<std::shared_ptr<sf::Drawable>> stroke;
    sf::Vector2f lastPos, rectStart, circCenter;
    std::string typed;
    sf::Text currText;
    currText.setFont(font);
    currText.setCharacterSize(21);
    currText.setFillColor(brush);

    DrawingMode mode=DrawingMode::FreeDraw;

    sf::RectangleShape bgBtn({100.f,30.f});
    bgBtn.setFillColor(sf::Color(150,150,150));
    bgBtn.setPosition(10.f,10.f);

    sf::Text bgLbl("BG color",font,21);
    bgLbl.setFillColor(sf::Color::Black);
    bgLbl.setPosition(bgBtn.getPosition().x+(bgBtn.getSize().x-bgLbl.getLocalBounds().width)/2.f,
                      bgBtn.getPosition().y+(bgBtn.getSize().y-bgLbl.getLocalBounds().height)/2.f-5.f);

    sf::RectangleShape squaresBtn({70.f,30.f});
    squaresBtn.setTexture(&squaresT);
    squaresBtn.setPosition(250.f,10.f);

    sf::RectangleShape drawBtn({70.f,30.f});
    drawBtn.setTexture(&drawT);
    drawBtn.setPosition(330.f,10.f);

    sf::RectangleShape circleBtn({70.f,30.f});
    circleBtn.setTexture(&circleT);
    circleBtn.setPosition(410.f,10.f);

    sf::RectangleShape textBtn({70.f,30.f});
    textBtn.setTexture(&textT);
    textBtn.setPosition(490.f,10.f);

    sf::RectangleShape triBtn({70.f,30.f});
    triBtn.setTexture(&triangleT);
    triBtn.setPosition(570.f,10.f);

    sf::RectangleShape rainbowModeBtn({70.f,30.f});
    rainbowModeBtn.setTexture(&rainbowModeT);
    rainbowModeBtn.setPosition(650.f,10.f);

    sf::RectangleShape bucketBtn({70.f,30.f});
    bucketBtn.setTexture(&bucketT);
    bucketBtn.setPosition(730.f,10.f);

    sf::RectangleShape colorPick({150.f,150.f});
    colorPick.setPosition(10.f,50.f);

    while(window.isOpen()){
        sf::Event ev;
        while(window.pollEvent(ev)){
            if(ev.type==sf::Event::Closed) window.close();
            else if(ev.type==sf::Event::MouseButtonPressed){
                if(ev.mouseButton.button==sf::Mouse::Left){
                    sf::Vector2i mp=sf::Mouse::getPosition(window);
                    if(bgBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        bgIndex=(bgIndex+1)%bgColors.size();
                        bgc=bgColors[bgIndex];
                    }
                    else if(squaresBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::Rectangle;
                    }
                    else if(drawBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::FreeDraw;
                    }
                    else if(circleBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::Circle;
                    }
                    else if(textBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::Text;
                    }
                    else if(triBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::Triangle;
                    }
                    else if(rainbowModeBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::Rainbow;
                        rainbowModeActive=true;
                    }
                    else if(bucketBtn.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        mode=DrawingMode::PaintBucket;
                    }
                    else if(colorWheel.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        showPicker=!showPicker;
                    }
                    else if(showPicker && colorPick.getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                        int xx=mp.x-(int)colorPick.getPosition().x;
                        int yy=mp.y-(int)colorPick.getPosition().y;
                        brush=getColorFromPosition(xx,yy,(int)colorPick.getSize().x,(int)colorPick.getSize().y);
                        rainbowModeActive=false;
                        mode=DrawingMode::FreeDraw;
                    }
                    else{
                        if(mode==DrawingMode::Rectangle){
                            rectStart=sf::Vector2f((float)mp.x,(float)mp.y);
                            isRect=true;
                        }
                        else if(mode==DrawingMode::Circle){
                            circCenter=sf::Vector2f((float)mp.x,(float)mp.y);
                            isCircle=true;
                        }
                        else if(mode==DrawingMode::Text && !isTyping){
                            window.setMouseCursor(textCursor);
                            currText.setPosition((float)mp.x,(float)mp.y);
                            currText.setFillColor(brush);
                            typed.clear();
                            currText.setString("");
                            isTyping=true;
                        }
                        else if(mode==DrawingMode::Triangle){
                            if(!isTri){
                                isTri=true;
                                triClicks=1;
                                triPts[0]=sf::Vector2f((float)mp.x,(float)mp.y);
                            }
                            else{
                                triClicks++;
                                triPts[triClicks-1]=sf::Vector2f((float)mp.x,(float)mp.y);
                                if(triClicks==3){
                                    if(rainbowModeActive){
                                        brush=HsvToRgb(rainbowHue,1.f,1.f);
                                        rainbowHue+=30.f;
                                        if(rainbowHue>360.f) rainbowHue-=360.f;
                                    }
                                    auto tri=std::make_shared<sf::ConvexShape>(3);
                                    tri->setPoint(0,triPts[0]);
                                    tri->setPoint(1,triPts[1]);
                                    tri->setPoint(2,triPts[2]);
                                    tri->setFillColor(brush);
                                    stack.push_back(tri);
                                    isTri=false;
                                    triClicks=0;
                                }
                            }
                        }
                        else if(mode==DrawingMode::FreeDraw||mode==DrawingMode::Rainbow){
                            isDrawing=true;
                            lastPos=sf::Vector2f((float)mp.x,(float)mp.y);
                            stroke.clear();
                        }
                        else if(mode==DrawingMode::PaintBucket){
                            bool shapeFound=false;
                            for(auto it=stack.rbegin(); it!=stack.rend(); ++it){
                                sf::Drawable* raw=it->get();
                                if(auto rect=dynamic_cast<sf::RectangleShape*>(raw)){
                                    if(rect->getGlobalBounds().contains((float)mp.x,(float)mp.y)){
                                        rect->setFillColor(brush);
                                        shapeFound=true;
                                        break;
                                    }
                                }
                                else if(auto circ=dynamic_cast<sf::CircleShape*>(raw)){
                                    auto pos=circ->getPosition();
                                    float r=circ->getRadius();
                                    sf::Vector2f diff((float)mp.x-pos.x,(float)mp.y-pos.y);
                                    if(std::sqrt(diff.x*diff.x+diff.y*diff.y)<=r){
                                        circ->setFillColor(brush);
                                        shapeFound=true;
                                        break;
                                    }
                                }
                                else if(auto tri=dynamic_cast<sf::ConvexShape*>(raw)){
                                    auto gb=tri->getGlobalBounds();
                                    if(gb.contains((float)mp.x,(float)mp.y)){
                                        tri->setFillColor(brush);
                                        shapeFound=true;
                                        break;
                                    }
                                }
                                else if(auto tx=dynamic_cast<sf::Text*>(raw)){
                                    auto gb=tx->getGlobalBounds();
                                    if(gb.contains((float)mp.x,(float)mp.y)){
                                        tx->setFillColor(brush);
                                        shapeFound=true;
                                        break;
                                    }
                                }
                                else if(auto va=dynamic_cast<sf::VertexArray*>(raw)){
                                }
                            }
                            if(!shapeFound){
                                bgc=brush;
                            }
                        }
                    }
                }
            }
            else if(ev.type==sf::Event::MouseButtonReleased){
                if(ev.mouseButton.button==sf::Mouse::Left){
                    if(mode==DrawingMode::Rectangle && isRect){
                        sf::Vector2i mp=sf::Mouse::getPosition(window);
                        sf::Vector2f ep((float)mp.x,(float)mp.y);
                        sf::Vector2f sz(std::fabs(ep.x-rectStart.x),std::fabs(ep.y-rectStart.y));
                        if(rainbowModeActive){
                            brush=HsvToRgb(rainbowHue,1.f,1.f);
                            rainbowHue+=30.f;
                            if(rainbowHue>360.f) rainbowHue-=360.f;
                        }
                        auto rect=std::make_shared<sf::RectangleShape>(sz);
                        rect->setFillColor(brush);
                        rect->setPosition(std::min(rectStart.x,ep.x),std::min(rectStart.y,ep.y));
                        stack.push_back(rect);
                        isRect=false;
                    }
                    else if(mode==DrawingMode::Circle && isCircle){
                        sf::Vector2i mp=sf::Mouse::getPosition(window);
                        sf::Vector2f ep((float)mp.x,(float)mp.y);
                        float rad=std::sqrt((ep.x-circCenter.x)*(ep.x-circCenter.x)+(ep.y-circCenter.y)*(ep.y-circCenter.y));
                        if(rainbowModeActive){
                            brush=HsvToRgb(rainbowHue,1.f,1.f);
                            rainbowHue+=30.f;
                            if(rainbowHue>360.f) rainbowHue-=360.f;
                        }
                        auto circ=std::make_shared<sf::CircleShape>(rad);
                        circ->setFillColor(brush);
                        circ->setOrigin(rad,rad);
                        circ->setPosition(circCenter);
                        stack.push_back(circ);
                        isCircle=false;
                    }
                    else if((mode==DrawingMode::FreeDraw||mode==DrawingMode::Rainbow) && isDrawing){
                        if(rainbowModeActive){
                            brush=HsvToRgb(rainbowHue,1.f,1.f);
                            rainbowHue+=30.f;
                            if(rainbowHue>360.f) rainbowHue-=360.f;
                        }
                        for(auto& quad:stroke){
                            stack.push_back(quad);
                        }
                        stroke.clear();
                        isDrawing=false;
                    }
                    if(mode==DrawingMode::Text) window.setMouseCursor(arrowCursor);
                }
            }
            else if(ev.type==sf::Event::TextEntered && isTyping){
                if(ev.text.unicode=='\b'){
                    if(!typed.empty()) typed.pop_back();
                    currText.setString(typed);
                }
                else{
                    if(ev.text.unicode=='\r') typed+='\n';
                    else typed+=(char)ev.text.unicode;
                    currText.setString(typed);
                }
            }
            else if(ev.type==sf::Event::KeyPressed){
                if(isTyping && ev.key.code==sf::Keyboard::Escape){
                    auto tx=std::make_shared<sf::Text>(currText);
                    stack.push_back(tx);
                    isTyping=false;
                    window.setMouseCursor(arrowCursor);
                }
                else if(ev.key.code==sf::Keyboard::C){
                    stack.clear();
                }
                else if(ev.key.code==sf::Keyboard::P){
                    showPicker=!showPicker;
                }
                else if(ev.key.code==sf::Keyboard::B){
                    bgIndex=(bgIndex+1)%bgColors.size();
                    bgc=bgColors[bgIndex];
                }
                else if(ev.key.code==sf::Keyboard::Up){
                    thick+=1.f;
                }
                else if(ev.key.code==sf::Keyboard::Down){
                    thick=std::max(1.f,thick-1.f);
                }
            }
        }
        if(isDrawing && (mode==DrawingMode::FreeDraw||mode==DrawingMode::Rainbow)){
            sf::Vector2i mp=sf::Mouse::getPosition(window);
            sf::Vector2f cp((float)mp.x,(float)mp.y);
            if(cp!=lastPos){
                auto seg=makeThickLineSegment(lastPos,cp,thick,brush);
                stroke.push_back(seg);
                lastPos=cp;
            }
        }
        if(rainbowModeActive && mode==DrawingMode::Rainbow){
            brush=HsvToRgb(rainbowHue,1.f,1.f);
            rainbowHue+=0.5f;
            if(rainbowHue>360.f) rainbowHue-=360.f;
        }
        window.clear(bgc);
        for(auto& d:stack){
            window.draw(*d);
        }
        for(auto& seg:stroke){
            window.draw(*seg);
        }
        if(isTyping){
            window.draw(currText);
            static sf::Clock cl;
            bool vis=((int)(cl.getElapsedTime().asSeconds()/0.5f)%2==0);
            if(vis){
                sf::RectangleShape cc(sf::Vector2f(1.f,currText.getCharacterSize()));
                sf::FloatRect tb=currText.getLocalBounds();
                cc.setPosition(currText.getPosition().x+tb.width+2.f,currText.getPosition().y);
                cc.setFillColor(sf::Color::Black);
                window.draw(cc);
            }
        }
        if(isTri && triClicks>0 && triClicks<3){
            sf::Vector2i mp=sf::Mouse::getPosition(window);
            sf::Vector2f current((float)mp.x,(float)mp.y);
            int pc=triClicks+1;
            sf::ConvexShape preview(pc);
            for(int i=0;i<triClicks;++i){
                preview.setPoint(i,triPts[i]);
            }
            preview.setPoint(pc-1,current);
            preview.setFillColor(sf::Color::Transparent);
            preview.setOutlineColor(sf::Color::Black);
            preview.setOutlineThickness(1.f);
            window.draw(preview);
        }
        if(isRect && mode==DrawingMode::Rectangle){
            sf::Vector2i mp=sf::Mouse::getPosition(window);
            sf::Vector2f cp((float)mp.x,(float)mp.y);
            sf::Vector2f topleft(std::min(rectStart.x,cp.x),std::min(rectStart.y,cp.y));
            sf::Vector2f sz(std::fabs(cp.x-rectStart.x),std::fabs(cp.y-rectStart.y));
            sf::RectangleShape pr(sz);
            pr.setPosition(topleft);
            pr.setFillColor(sf::Color::Transparent);
            pr.setOutlineColor(sf::Color::Black);
            pr.setOutlineThickness(1.f);
            window.draw(pr);
        }
        if(isCircle && mode==DrawingMode::Circle){
            sf::Vector2i mp=sf::Mouse::getPosition(window);
            sf::Vector2f cp((float)mp.x,(float)mp.y);
            float rad=std::sqrt((cp.x-circCenter.x)*(cp.x-circCenter.x)+(cp.y-circCenter.y)*(cp.y-circCenter.y));
            sf::CircleShape c(rad);
            c.setOrigin(rad,rad);
            c.setPosition(circCenter);
            c.setFillColor(sf::Color::Transparent);
            c.setOutlineColor(sf::Color::Black);
            c.setOutlineThickness(1.f);
            window.draw(c);
        }
        if(showPicker){
            sf::Image im;
            im.create((unsigned int)colorPick.getSize().x,(unsigned int)colorPick.getSize().y);
            for(unsigned int xx=0;xx<im.getSize().x;xx++){
                for(unsigned int yy=0;yy<im.getSize().y;yy++){
                    im.setPixel(xx,yy,getColorFromPosition(xx,yy,im.getSize().x,im.getSize().y));
                }
            }
            sf::Texture ctex;
            ctex.loadFromImage(im);
            colorPick.setTexture(&ctex);
            window.draw(colorPick);
        }
        window.draw(bgBtn);
        window.draw(bgLbl);
        window.draw(squaresBtn);
        window.draw(drawBtn);
        window.draw(circleBtn);
        window.draw(textBtn);
        window.draw(triBtn);
        window.draw(rainbowModeBtn);
        window.draw(bucketBtn);
        window.draw(colorWheel);
        window.display();
    }
    return 0;
}
