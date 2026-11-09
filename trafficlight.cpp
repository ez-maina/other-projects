#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include<iostream>
#include <string>
#include <vector>
#include <cstdint>

constexpr int RED = 0, GREEN = 1, YELLOW = 2;

static sf::Color mixColor(sf::Color a, sf::Color b, float t)

{
    auto ch = [t](std::uint8_t x, std::uint8_t y) {
        return static_cast<std::uint8_t>(x + (y - x) * t);
    };
    return sf::Color(ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), ch(a.a, b.a));
}

static void centerText(sf::Text& text, sf::Vector2f c)
{
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    text.setPosition(c);
}

struct Button
{
    sf::RectangleShape box;
    sf::Text label;

    Button(const sf::Font& font, const std::string& str, sf::Vector2f topLeft)
        : box({110.f, 40.f}), label(font, str, 20)
    {
        box.setSize({200.f, 50.f});
        box.setPosition(topLeft);
        box.setFillColor(sf::Color(100, 100, 100));
        box.setOutlineThickness(5.f);
        box.setOutlineColor(sf::Color(35, 35, 35));

        label.setFont(font);
        label.setString(str);
        label.setCharacterSize(24);
        label.setFillColor(sf::Color::White);
        centerText(label, {topLeft.x + 100.f, topLeft.y + 25.f});
    }   
    bool contains(sf::Vector2f p) const { return box.getGlobalBounds().contains(p); }
    void setHover(bool h) { box.setFillColor(h ? sf::Color(95, 95, 110) : sf::Color(55, 55, 65)); }
};


int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 700}), "Traffic Light Simulation");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf") && !font.openFromFile("C:\\Windows\\Fonts\\arial.ttf"))
    {
        std::cerr << "could not load font. Put a .ttf next to the exe and change the path.\n";
        return 1;
    }
    
    // ------------- Background (built once) -------------
    sf::VertexArray sky (sf::PrimitiveType::TriangleStrip, 4);
    sky[0].position = {0.f, 0.f}; sky[1].position = {800.f, 0.f}; 
    sky[2].position = {0.f, 560.f}; sky[3].position = {800.f, 560.f};
    sky[0].color = sky[1].color = sf::Color(8, 10, 30);
    sky[2].color = sky[3].color = sf::Color(55, 40, 85);

    std::vector<sf::CircleShape> celestial;
    {
        sf::CircleShape moon(28.f);
        moon.setPosition({690.f, 50.f});
        moon.setFillColor(sf::Color(235, 235, 210));
        celestial.push_back(moon);
        for (int i = 0; i < 45; ++i)
        {
           
            sf::CircleShape star (i % 3 == 0 ? 1.5f : 1.f);
            star.setPosition({static_cast<float>((i * 137 + 53) % 800), static_cast<float>((i * 71 + 17)% 330)});
            star.setFillColor(sf::Color(200, 200, 230));
            celestial.push_back(star);

        }

    }

    

    std::vector<sf::RectangleShape> scene;
    auto addRect = [&scene](float x, float y, float w, float h, sf::Color c) {
        sf::RectangleShape r({w, h});
        r.setPosition({x, y});
        r.setFillColor(c);
        scene.push_back(r);

    };

    struct Building {float x, w, h; };
    const Building buildings[] = 
        {
        { 0, 90, 180}, { 85, 70, 260}, {150, 100, 150}, {245, 80, 320}, {280, 70, 210},
        {385, 90, 170}, {470, 70, 280}, {535, 100, 200}, {630, 80, 330}, {705, 95, 230},
        };
    
    int bi = 0;
    for (const Building& b : buildings)
     
    {
        float top = 560.f - b.h;
        addRect(b.x, top, b.w, b.h, bi % 2 ? sf::Color(28, 30, 48) : sf::Color(22, 24, 40));
        int cols = static_cast<int>((b.w - 10.f) / 18.f);
        int rows = static_cast<int>((b.h - 10.f) / 26.f);
        for (int r = 0; r < rows; ++r)
        
            for (int c = 0; c < cols; ++c)
                if ((bi * 5 + r * 3 + c * 7) % 4 == 0)
                    addRect(b.x + 10.f + c * 18.f, top + 10.f + r * 26.f, 8.f, 16.f,
                            sf::Color(255, 220 , 120));
        ++bi;

            
    }
 
    addRect(0.f, 560.f, 800.f, 40.f, sf::Color(70, 70, 80)); // sidewalk
    addRect(0.f, 596.f, 800.f, 4.f, sf::Color(120, 120, 130)); // curb
    addRect(0.f, 600.f, 800.f, 100.f, sf::Color(35,35,40)); // road
    for (float x = 20.f; x < 800.f; x += 80.f)
        addRect(x, 622.f, 40.f, 6.f, sf::Color(200, 200, 200));
    addRect(388.f, 440.f, 24.f, 120.f, sf::Color(55, 55, 60)); // pole
    addRect(391.f, 440.f, 5.f, 110.f, sf::Color(90, 90, 98)); // pole highlight
    addRect(372.f, 548.f, 56.f, 12.f, sf::Color(50, 50, 55)); // base plate

    // ------------- Traffic light -------------
    sf::RectangleShape housing({160.f, 400.f});
    housing.setPosition({320.f, 40.f});
    housing.setFillColor(sf::Color(30, 30, 32));
    housing.setOutlineThickness(5.f);
    housing.setOutlineColor(sf::Color(90, 90, 95));

    // Indexed by RED, GREEN, YELLOW
    const float lampY[3] = { 117.5f, 362.5f, 240.f };
    const sf::Color dimColor[3] = { sf::Color(70, 12, 12), sf::Color(12, 70, 25), sf::Color(70, 60, 8)};
    const sf::Color brightColor[3] = { sf::Color(255, 50, 50), sf::Color(50, 255, 90), sf::Color(255, 215, 20)};
    const float duration[3] = { 4.f, 4.f, 2.f}; 
    const char* stateName[3] = { "RED", "GREEN", "YELLOW"};

    sf::CircleShape lamp(45.f);
    lamp.setOrigin({45.f, 45.f});
    lamp.setOutlineThickness(3.f);
    lamp.setOutlineColor(sf::Color(15, 15, 15));

    sf::CircleShape glow;

    // ----------Countdown panel ------------
    sf::RectangleShape panel ({150.f, 110.f});
    panel.setPosition({560.f, 150.f});
    panel.setFillColor(sf::Color(5, 5, 8));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(90, 90, 95));

    sf::Text countdown(font, "4", 72);
    sf::Text stateLabel(font, "RED", 16);
    sf::Text status(font, "READY", 18);

    // ---------- Buttons ------------
    Button startButton(font, "START", {90.f, 640.f});
    Button pauseButton(font, "PAUSE", {300.f, 640.f});
    Button resetButton(font, "RESET", {510.f, 640.f});

    // ---------- simulation state ------------
    int state = RED;
    float timer = 0.f;
    bool running = false;
    bool started = false;
    float brightness[3] = {1.f, 0.f , 0.f};
    sf::Clock frameClock;

     while (window.isOpen())
    {
        float dt = std::min(frameClock.restart().asSeconds(), 0.1f);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouse->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2f p(mouse->position);
                    if (startButton.contains(p))
                    {
                        running = true;
                        started = true;
                    }
                    else if (pauseButton.contains(p))
                     {
                        running = false;
                     }
                    else if (resetButton.contains(p))
                    {
                        running = false;
                        started = false;
                        state = RED;
                        timer = 0.f;
    
    
                    
             
                    }
                }
            }
        }                
 
         // Timing
         if (running)
         {
             timer += dt;
             if (timer >= duration[state])
             {
                 timer -= duration[state];
                 state = (state + 1) % 3;
             }
         }
 
         // Smooth fade
         for (int i = 0; i < 3; ++i)
         {
             float target = (i == state) ? 1.f : 0.f;
             brightness[i] += (target - brightness[i]) * std::min(1.f, dt * 8.f);
         }
 
         // UI text
         int secs = std::max(0, static_cast<int>(std::ceil(duration[state] - timer)));
         countdown.setString((secs < 10 ? "0" : "") + std::to_string(secs));
         countdown.setFillColor(brightColor[state]);
         centerText(countdown, {635.f, 190.f});
         stateLabel.setString(stateName[state]);
         stateLabel.setFillColor(brightColor[state]);
         centerText(stateLabel, {635.f, 240.f});
         status.setString(running ? "RUNNING" : (started ? "PAUSED" : "READY"));
         centerText(status, {400.f, 578.f});
 
         sf::Vector2f mp(sf::Mouse::getPosition(window));
         startButton.setHover(startButton.contains(mp));
         pauseButton.setHover(pauseButton.contains(mp));
         resetButton.setHover(resetButton.contains(mp));
 
         // Draw
         window.clear();
         window.draw(sky);
         for (const auto& c : celestial) window.draw(c);
         for (const auto& r : scene) window.draw(r);
         window.draw(housing);
 
        for (int i = 0; i < 3; ++i)
        {
             lamp.setPosition({400.f, lampY[i]});
             lamp.setFillColor(mixColor(dimColor[i], brightColor[i], brightness[i]));
             window.draw(lamp);
        }
 
         // Glow
         sf::RenderStates additive;
         additive.blendMode = sf::BlendAdd;
         for (int i = 0; i < 3; ++i)
        {
             if (brightness[i] < 0.02f) continue;
             for (int k = 1; k <= 5; ++k)
            {
                 float r = 45.f + k * 8.f;
                 glow.setRadius(r);
                 glow.setOrigin({r, r});
                 glow.setPosition({400.f, lampY[i]});
                 glow.setFillColor(sf::Color(brightColor[i].r, brightColor[i].g, brightColor[i].b,
                                             static_cast<std::uint8_t>(brightness[i] * 22.f)));
                 window.draw(glow, additive);
            }
        }
 
         window.draw(panel);
         window.draw(countdown);
         window.draw(stateLabel);
         window.draw(status);
         for (Button* b : {&startButton, &pauseButton, &resetButton})
        {
             window.draw(b->box);
             window.draw(b->label);
        }
         window.display();
    }
 
     return 0;
}
 
    
    