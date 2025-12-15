#include <SFML/Graphics.hpp>
#include "rendering/texture_atlas.h"
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "TextureAtlas Test");
    window.setFramerateLimit(60);
    
    // Load atlas
    Engine::TextureAtlas atlas;
    if (!atlas.loadFromFile("assets/player_atlas.png", "assets/player_atlas.json")) {
        std::cerr << "Failed to load atlas!\n";
        return 1;
    }
    
    std::cout << "Loaded atlas:\n";
    std::cout << "  Frames: " << atlas.getFrameCount() << "\n";
    std::cout << "  Animations: " << atlas.getAnimationCount() << "\n";
    
    // List animations
    std::cout << "\nAnimations:\n";
    for (const auto& name : atlas.getAnimationNames()) {
        const Engine::AnimationData* anim = atlas.getAnimation(name);
        std::cout << "  - " << name << " (" << anim->getFrameCount() 
                  << " frames, loop=" << anim->loop << ")\n";
    }
    
    // Display all frames in a grid
    std::vector<sf::Sprite> sprites;
    auto frameNames = atlas.getFrameNames();
    
    for (size_t i = 0; i < frameNames.size(); ++i) {
        sf::Sprite sprite = atlas.createSprite(frameNames[i]);
        sprite.setPosition(50.0f + (i % 8) * 80.0f, 50.0f + (i / 8) * 80.0f);
        sprite.setScale(2.0f, 2.0f);
        sprites.push_back(sprite);
    }
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear(sf::Color(40, 40, 50));
        
        for (const auto& sprite : sprites) {
            window.draw(sprite);
        }
        
        window.display();
    }
    
    return 0;
}