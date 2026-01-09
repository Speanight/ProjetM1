/*
 *Code SFLM - vérif
#include <iostream>
#include <SFML/Graphics.hpp>


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Hello Furball!");
     while (window.isOpen()) {
         sf::Event event;
         while (window.pollEvent(event))
             if (event.type == sf::Event::Closed)
                 window.close();
         window.clear();
         window.display();
     }
}
*/
/*
 * Code SFLM - Proto jeu
using namespace std;
int main () {
    // création de la fenêtre
    sf::RenderWindow window(sf::VideoMode(800, 600), "Proto déplacement");
    float x=300;
    float y=300;

    float x2 = 500;
    float y2= 300;

    float anim = 1;
    // Boucle de fonctionnement du joueur
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        // Mise du fond en blanc
        window.clear(sf::Color::White);

        // Client 1
            // construction d'un carré de 50 par 50
            sf::RectangleShape carre (sf::Vector2f(50, 50));
            // change sa couleur
            carre.setFillColor(sf::Color(255,0,0));
            // définit sa position à (x,y)=(150,50)

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                std::cout << "A pressed !" << std::endl;
                x-=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                std::cout << "D pressed !" << std::endl;
                x+=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                std::cout << "W pressed !" << std::endl;
                y-=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                std::cout << "S pressed !" << std::endl;
                y+=anim;
            }
            carre.setPosition(x, y);

        // Client 2
            // construction d'un carré de 50 par 50
            sf::RectangleShape carre2 (sf::Vector2f(50, 50));
            // change sa couleur
            carre2.setFillColor(sf::Color(0,0,255));

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                std::cout << "Left pressed !" << std::endl;
                x2-=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                std::cout << "Rigth pressed !" << std::endl;
                x2+=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                std::cout << "Up pressed !" << std::endl;
                y2-=anim;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                std::cout << "Down pressed !" << std::endl;
                y2+=anim;
            }
            carre2.setPosition(x2, y2);

        // dessin du carre
        window.draw(carre);
        window.draw(carre2);
        // affichage de tout ce qu'on a dessiné
        window.display();
    }
    return 0;
}
*/
/*
 * Code ImgUI - vérif
*/
