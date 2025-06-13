#include <SFML/Graphics.hpp>
#include "Figure.h"
#include <iostream>


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "LB5");

 
    GraphicsFacade facade(800, 600);

  
    facade.addCircle("circle1", 50, 400, 100);
    facade.addRectangle("rect1", 100, 50, 200, 350);
    facade.addGroup("group1", 400, 200, 50, 50);
    facade.setCurrentGroup("group1");
    facade.addCircle("circle2", 20, 20, -5);
    facade.addRectangle("rect2", 100.f, 50.f, 300.f, 150.f);
    facade.resetToMainGroup();

  
    facade.printDocumentStructure();
    std::cout << "\n"; std::cout << "\n";

    
    facade.moveElement("circle1", 400.f, 400.f);

   
    facade.removeElementByName("rect1");
 
    facade.undo();
    facade.undo();

    facade.resetToMainGroup();
    facade.addGroup("group2", 100, 50, 200, 400);
    facade.setCurrentGroup("group2");
    facade.addCircle("circle2", 100, -50, -50);
    facade.resetToMainGroup();
    facade.addGroup("group2", 100, 50, 200, 350);

    facade.printDocumentStructure(); std::cout << "\n"; std::cout << "\n";


    facade.iterateAll([](const std::shared_ptr<Figure>& element) {
        std::cout << " - " << element->getName() << std::endl;
        });


    facade.printDocumentStructure(); std::cout << "\n"; std::cout << "\n";

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

                auto result = facade.findElementWithGroup(worldPos.x, worldPos.y);

                if (result.second != nullptr) {
                    if (result.first != nullptr) {
                        std::cout << "Select group: " << result.first->getName() << std::endl;
                    }
                    std::cout << "Select element: " << result.second->getName() << std::endl;
                }
                else {
                    std::cout << "Element do't detected.\n";
                }
            }
        }

        window.clear(sf::Color::Black);
        facade.draw(window);
        window.display();
    }

    return 0;
}