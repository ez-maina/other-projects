#include <SFML/Graphics.hpp>


int main()
{
    sf::RenderWindow window(sf::VideoMode({600, 600}), "Traffic Light Simulation"
    );

    window.setFramerateLimit(60);

    //Traffic light housing
    sf::RectangleShape housing(sf::Vector2f(160, 400.f)

);
    housing.setPosition(sf::Vector2f(220.f, 80.f)
);
    housing.setFillColor(sf::Color(35, 35, 35)
);
    housing.setOutlineThickness(5.f);
    
    housing.setOutlineColor(sf::Color(100, 100, 100)
);
    
    //Three traffic light circles
    sf::CircleShape redLight(45.f
    );
    sf::CircleShape yellowLight(45.f
    );
    sf::CircleShape greenLight(45.f
    );

    redLight.setPosition(sf::Vector2f(255.f, 125.f)
);
    yellowLight.setPosition(sf::Vector2f(255.f, 235.f)
);
    greenLight.setPosition(sf::Vector2f(255.f, 345.f)
);

    //Initial light states
    redLight.setFillColor(sf::Color(255, 0, 0)
);    
    yellowLight.setFillColor(sf::Color(100, 100, 0)
);
    greenLight.setFillColor(sf::Color(0, 100, 0)
);

enum class LightState
{
    Red,
    Yellow,
    Green
};
LightState state = LightState::Red;
sf::Clock clock;


while (window.isOpen())
{
    while (const auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }
    }

    //State transitions
    float t = clock.getElapsedTime().asSeconds();
    if (state == LightState::Red && t >= 4.f)
    {
        state = LightState::Green;
        clock.restart();
    }
    else if (state == LightState::Green && t >= 4.f)
    {
        state = LightState::Yellow;
        clock.restart();
    }
    else if (state == LightState::Yellow && t >= 1.5f)
    {
        state = LightState::Red;
        clock.restart();
    }
        //Bright when active, dim otherwise
        redLight.setFillColor(state == LightState::Red ? sf::Color(255, 0, 0) : sf::Color(100, 0, 0));
        yellowLight.setFillColor(state == LightState::Yellow ? sf::Color(255, 255, 0) : sf::Color(100, 100, 0));
        greenLight.setFillColor(state == LightState::Green ? sf::Color(0, 255, 0) : sf::Color(0, 100, 0));
    
    window.clear(sf::Color(35, 35, 35));

    window.draw(housing);
    window.draw(redLight);
    window.draw(yellowLight);
    window.draw(greenLight);

    window.display();
}

    return 0;
}

