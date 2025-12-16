#include <SFML/Graphics.hpp>
#include "input/input_manager.h"
#include "rendering/camera.h"
#include "core/time_manager.h"
#include <iostream>

using namespace Engine;

int main() {
    // Window setup
    sf::RenderWindow window(sf::VideoMode(800, 600), "Input & Camera Demo");
    window.setFramerateLimit(60);
    
    // Systems
    TimeManager time;
    InputManager input;
    Camera camera({400.0f, 300.0f}, {800.0f, 600.0f});
    
    // Configure input mappings
    input.mapAction("move_left", sf::Keyboard::A);
    input.mapAction("move_right", sf::Keyboard::D);
    input.mapAction("move_up", sf::Keyboard::W);
    input.mapAction("move_down", sf::Keyboard::S);
    input.mapAction("jump", sf::Keyboard::Space);
    input.mapAction("zoom_in", sf::Keyboard::Q);
    input.mapAction("zoom_out", sf::Keyboard::E);
    input.mapAction("shake", sf::Mouse::Left);
    
    // Player entity
    sf::Vector2f playerPos(400.0f, 300.0f);
    sf::CircleShape player(20.0f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(20.0f, 20.0f);
    
    // Player state
    bool isGrounded = true;
    float velocityY = 0.0f;
    const float jumpForce = -400.0f;
    const float gravity = 1000.0f;
    
    // Camera setup
    camera.setTarget(&playerPos);
    camera.setFollowMode(CameraFollowMode::Smooth);
    camera.setFollowSpeed(3.0f);
    
    // Configure jump buffering callback
    input.setActionPressedCallback("jump", [&]() {
        if (!isGrounded) {
            // Player in air - buffer the jump
            input.bufferAction("jump", 0.15f);
            std::cout << "Jump buffered (in air)\n";
        }
    });
    
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║    Input & Camera System Demo        ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\nControls:\n";
    std::cout << "  WASD       - Move player\n";
    std::cout << "  SPACE      - Jump (with buffering!)\n";
    std::cout << "  Q/E        - Zoom in/out\n";
    std::cout << "  Left Click - Screen shake\n";
    std::cout << "  ESC        - Quit\n\n";
    std::cout << "Try pressing jump just before landing!\n\n";
    
    while (window.isOpen()) {
        time.update();
        float dt = time.getDeltaTime();
        
        // Input handling - MUST call beginFrame before event polling
        input.beginFrame();
        
        // Event loop
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
                
            if (event.type == sf::Event::KeyPressed) {
                input.handleKeyPressed(event.key.code);
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
            
            if (event.type == sf::Event::KeyReleased) {
                input.handleKeyReleased(event.key.code);
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                input.handleMouseButtonPressed(event.mouseButton.button);
            }
            
            if (event.type == sf::Event::MouseButtonReleased) {
                input.handleMouseButtonReleased(event.mouseButton.button);
            }
        }
        
        // Update input system - MUST call after event polling
        input.update(dt);
        
        // Player movement
        float moveSpeed = 200.0f;
        if (input.isActionActive("move_left"))
            playerPos.x -= moveSpeed * dt;
        if (input.isActionActive("move_right"))
            playerPos.x += moveSpeed * dt;
        if (input.isActionActive("move_up"))
            playerPos.y -= moveSpeed * dt;
        if (input.isActionActive("move_down"))
            playerPos.y += moveSpeed * dt;
        
        // Jump with buffering
        bool jumpPressed = input.isActionPressed("jump");
        bool bufferedJump = input.consumeBufferedAction("jump");
        
        if ((jumpPressed || bufferedJump) && isGrounded) {
            velocityY = jumpForce;
            isGrounded = false;
            camera.addTrauma(0.2f);  // Small shake on jump
            
            if (bufferedJump) {
                std::cout << "Buffered jump executed!\n";
            }
        }
        
        // Simple gravity simulation
        if (!isGrounded) {
            velocityY += gravity * dt;
            playerPos.y += velocityY * dt;
            
            // Ground check (simplified)
            if (playerPos.y >= 550.0f) {
                playerPos.y = 550.0f;
                velocityY = 0.0f;
                isGrounded = true;
            }
        }
        
        // Camera zoom
        if (input.isActionPressed("zoom_in"))
            camera.zoomTo(0.5f, 0.3f);
        if (input.isActionPressed("zoom_out"))
            camera.zoomTo(1.5f, 0.3f);
        
        // Screen shake on click
        if (input.isActionPressed("shake")) {
            camera.addTrauma(0.8f);
            std::cout << "SHAKE!\n";
        }
        
        // Update camera
        camera.update(dt);
        
        // Update visuals
        player.setPosition(playerPos);
        if (!isGrounded) {
            player.setFillColor(sf::Color::Yellow);
        } else {
            player.setFillColor(sf::Color::Green);
        }
        
        // Render
        window.clear(sf::Color(30, 30, 40));
        window.setView(camera.toSFMLView());
        
        // Draw ground
        sf::RectangleShape ground(sf::Vector2f(2000.0f, 100.0f));
        ground.setPosition(-500.0f, 550.0f);
        ground.setFillColor(sf::Color(80, 80, 80));
        window.draw(ground);
        
        // Draw player
        window.draw(player);
        
        // Reset view for UI
        window.setView(window.getDefaultView());
        
        // Display FPS and info
        if (time.getFrameCount() % 30 == 0) {
            std::cout << "FPS: " << static_cast<int>(time.getFPS()) 
                      << " | Trauma: " << camera.getTrauma()
                      << " | Zoom: " << camera.getZoom()
                      << " | Grounded: " << (isGrounded ? "YES" : "NO")
                      << "\r" << std::flush;
        }
        
        window.display();
    }
    
    std::cout << "\n\nDemo complete!\n";
    return 0;
}