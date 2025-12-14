#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "core/time_manager.h"
#include "core/transform.h"
#include "math/rectangle.h"

using namespace Engine;

int main() {
    // Window setup
    sf::RenderWindow window(sf::VideoMode(800, 600), "Game Engine v0.1");
    window.setFramerateLimit(60);
    
    // Systems
    TimeManager time;
    
    // Test entities
    Transform playerTransform;
    playerTransform.position = {400.0f, 300.0f};
    
    Transform enemyTransform;
    enemyTransform.position = {600.0f, 300.0f};
    
    // Visuals
    sf::CircleShape player(30.0f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(30.0f, 30.0f);
    
    sf::CircleShape enemy(25.0f);
    enemy.setFillColor(sf::Color::Red);
    enemy.setOrigin(25.0f, 25.0f);
    
    // UI
    sf::Font font;
    // Note: You'll need to provide a font file or use system fonts
    // For now, we'll skip text rendering
    
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║      Game Engine v0.1 - Running       ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\nControls:\n";
    std::cout << "  Arrow Keys - Move player\n";
    std::cout << "  ESC        - Quit\n";
    std::cout << "\nTests: Run 'ctest' in build directory\n\n";
    
    // Game loop
    while (window.isOpen()) {
        time.update();
        
        // Events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }
        
        // Input (test transform system)
        float speed = 200.0f * time.getDeltaTime();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) 
            playerTransform.position.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) 
            playerTransform.position.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) 
            playerTransform.position.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) 
            playerTransform.position.y += speed;
        
        // Test collision system
        Rectangle playerBounds(
            playerTransform.position.x - 30.0f,
            playerTransform.position.y - 30.0f,
            60.0f, 60.0f
        );
        
        Rectangle enemyBounds(
            enemyTransform.position.x - 25.0f,
            enemyTransform.position.y - 25.0f,
            50.0f, 50.0f
        );
        
        bool colliding = playerBounds.intersects(enemyBounds);
        
        // Update visuals
        player.setPosition(playerTransform.position);
        enemy.setPosition(enemyTransform.position);
        
        // Visual feedback for collision
        if (colliding) {
            player.setFillColor(sf::Color::Yellow);
            enemy.setFillColor(sf::Color::Magenta);
        } else {
            player.setFillColor(sf::Color::Green);
            enemy.setFillColor(sf::Color::Red);
        }
        
        // Render
        window.clear(sf::Color(20, 20, 30));
        window.draw(enemy);
        window.draw(player);
        window.display();
        
        // FPS to console every 60 frames
        if (time.getFrameCount() % 60 == 0) {
            std::cout << "FPS: " << std::fixed << std::setprecision(1) 
                      << time.getFPS() << " | "
                      << "Frame: " << time.getFrameCount() << " | "
                      << "Collision: " << (colliding ? "YES" : "NO") << "\r" << std::flush;
        }
    }
    
    std::cout << "\n\nEngine shutting down. Goodbye!\n";
    return 0;
}