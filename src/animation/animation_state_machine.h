// src/Animation/AnimationStateMachine.h
#pragma once
#include "animation_controller.h"
#include <string>
#include <unordered_map>
#include <functional>

namespace Engine {

// Transition conditions
enum class TransitionCondition {
    Immediate,     // Switch immediately
    OnFinish,      // Wait for current animation to finish
    CanInterrupt   // Can interrupt if priority allows
};

struct AnimationState {
    std::string name;
    int priority = 0;  // Higher = harder to interrupt
};

struct AnimationTransition {
    std::string fromState;
    std::string toState;
    TransitionCondition condition = TransitionCondition::Immediate;
    std::function<bool()> predicate;  // Custom condition check
};

class AnimationStateMachine {
public:
    AnimationStateMachine(AnimationController* controller);
    
    // State management
    void addState(const std::string& name, int priority = 0);
    void addTransition(const std::string& from, const std::string& to,
                      TransitionCondition condition = TransitionCondition::Immediate,
                      std::function<bool()> predicate = nullptr);
    
    // Control
    void transitionTo(const std::string& stateName, bool force = false);
    void update(float dt);
    
    // Query
    std::string getCurrentState() const { return currentStateName; }
    int getCurrentPriority() const;
    
private:
    AnimationController* controller;
    
    std::unordered_map<std::string, AnimationState> states;
    std::vector<AnimationTransition> transitions;
    
    std::string currentStateName;
    std::string pendingStateName;
    
    bool canTransition(const std::string& toState) const;
    void executeTransition(const std::string& toState);
};

} // namespace Engine