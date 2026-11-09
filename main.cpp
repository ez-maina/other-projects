#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::Vector2u({800,600}), "Traffic Light Simulation");
    
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            
        }
    }

    window.clear(sf::Color(35, 35, 35));
    window.display();

    return 0;
}
    