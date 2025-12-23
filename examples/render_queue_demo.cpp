// Example: main.cpp with RenderQueue
// Drop-in replacement for your current main.cpp to test depth sorting

#include <SFML/Graphics.hpp>
#include <iostream>
#include "core/time_manager.h"
#include "core/transform.h"
#include "rendering/render_queue.h"

using namespace Engine;

int main() {
    // Window setup
    sf::RenderWindow window(sf::VideoMode(800, 600), "Game Engine v0.2 - Depth Sorting");
    window.setFramerateLimit(60);
    
    // Systems
    TimeManager time;
    RenderQueue queue;
    
    // Create three entities at different depths
    // Entity 1: Background (furthest)
    Transform bgTransform;
    bgTransform.position = {400.0f, 300.0f};
    bgTransform.depth = 1000.0f;  // Far back
    
    sf::CircleShape bgCircle(80.0f);
    bgCircle.setFillColor(sf::Color(100, 100, 200, 150));  // Blue, translucent
    bgCircle.setOrigin(80.0f, 80.0f);
    
    // Entity 2: Player (middle)
    Transform playerTransform;
    playerTransform.position = {400.0f, 300.0f};
    playerTransform.depth = 200.0f;  // Middle
    
    sf::CircleShape player(30.0f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(30.0f, 30.0f);
    
    // Entity 3: Foreground obstacle (nearest)
    Transform fgTransform;
    fgTransform.position = {500.0f, 300.0f};
    fgTransform.depth = 100.0f;  // Foreground
    
    sf::CircleShape fgCircle(40.0f);
    fgCircle.setFillColor(sf::Color(200, 100, 100, 200));  // Red, translucent
    fgCircle.setOrigin(40.0f, 40.0f);
    
    // Camera transform (for demonstration)
    Transform cameraTransform;
    cameraTransform.position = {0.0f, 0.0f};  // No offset initially
    
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   Depth Sorting Demo - Engine v0.2    ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\nControls:\n";
    std::cout << "  Arrow Keys - Move player (green)\n";
    std::cout << "  WASD       - Move camera\n";
    std::cout << "  Space      - Swap player depth (toggle foreground/background)\n";
    std::cout << "  ESC        - Quit\n";
    std::cout << "\nDepth Layers:\n";
    std::cout << "  Background (blue):  depth=1000 (far)\n";
    std::cout << "  Player (green):     depth=200  (middle)\n";
    std::cout << "  Foreground (red):   depth=100  (near)\n";
    std::cout << "\nPress Space to see player jump behind background!\n\n";
    
    // Game loop
    while (window.isOpen()) {
        time.update();
        float dt = time.getDeltaTime();
        
        // Events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                    
                // Toggle player depth
                if (event.key.code == sf::Keyboard::Space) {
                    if (playerTransform.depth < 500.0f) {
                        playerTransform.depth = 1500.0f;  // Behind background
                        std::cout << "Player depth: 1500 (behind background)\n";
                    } else {
                        playerTransform.depth = 200.0f;   // Middle
                        std::cout << "Player depth: 200 (middle layer)\n";
                    }
                }
            }
        }
        
        // Player movement
        float speed = 200.0f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) 
            playerTransform.position.x += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) 
            playerTransform.position.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) 
            playerTransform.position.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) 
            playerTransform.position.y += speed;
        
        // Camera movement (WASD)
        float camSpeed = 150.0f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) 
            cameraTransform.position.x += camSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) 
            cameraTransform.position.x -= camSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) 
            cameraTransform.position.y -= camSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) 
            cameraTransform.position.y += camSpeed;
        
        // Setup camera for this frame
        queue.setCameraTransform(cameraTransform);
        
        // Submit entities to render queue
        // Note: We convert sf::Shape to sf::Sprite by rendering to texture first
        // In real game, you'd use TextureAtlas sprites
        queue.clear();
        
        // Background entity
        sf::Sprite bgSprite;
        bgSprite.setTexture(bgCircle.getTexture() ? *bgCircle.getTexture() : 
                           *new sf::Texture());  // Placeholder
        queue.submit(bgTransform.depth, bgSprite, bgTransform);
        
        // Player entity
        sf::Sprite playerSprite;
        queue.submit(playerTransform.depth, playerSprite, playerTransform);
        
        // Foreground entity
        sf::Sprite fgSprite;
        queue.submit(fgTransform.depth, fgSprite, fgTransform);
        
        // Sort and render with depth ordering
        queue.sort();
        
        window.clear(sf::Color(20, 20, 30));
        
        // For this demo, we'll render shapes directly (bypassing queue for simplicity)
        // In production, you'd use queue.render(window) with proper sprites
        
        // Manual depth-sorted rendering (for demo purposes)
        std::vector<std::pair<float, sf::Shape*>> shapes = {
            {bgTransform.depth, &bgCircle},
            {playerTransform.depth, &player},
            {fgTransform.depth, &fgCircle}
        };
        
        std::sort(shapes.begin(), shapes.end(), 
            [](const auto& a, const auto& b) { return a.first > b.first; });
        
        for (auto& [depth, shape] : shapes) {
            // Apply transform
            if (shape == &bgCircle) {
                shape->setPosition(bgTransform.position - cameraTransform.position);
            } else if (shape == &player) {
                shape->setPosition(playerTransform.position - cameraTransform.position);
            } else if (shape == &fgCircle) {
                shape->setPosition(fgTransform.position - cameraTransform.position);
            }
            window.draw(*shape);
        }
        
        // Draw depth indicators
        sf::Font font;
        // Note: In real implementation, load a font file
        
        window.display();
        
        // FPS to console
        if (time.getFrameCount() % 60 == 0) {
            std::cout << "FPS: " << time.getFPS() 
                      << " | Queue size: " << queue.size()
                      << " | Camera: (" << cameraTransform.position.x 
                      << ", " << cameraTransform.position.y << ")\r" << std::flush;
        }
    }
    
    std::cout << "\n\nEngine shutting down. Goodbye!\n";
    return 0;
}

/*
 * PRODUCTION VERSION (with proper sprites):
 * 
 * // Load sprites from TextureAtlas
 * TextureAtlas atlas;
 * atlas.loadFromFile("sprites.png", "sprites.json");
 * 
 * // Create entities with sprite components
 * struct Entity {
 *     Transform transform;
 *     std::string spriteName;
 * };
 * 
 * // In render loop:
 * queue.clear();
 * for (auto& entity : entities) {
 *     sf::Sprite sprite = atlas.createSprite(entity.spriteName);
 *     queue.submit(entity.transform.depth, sprite, entity.transform);
 * }
 * queue.sort();
 * queue.render(window);
 */
