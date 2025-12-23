#pragma once
#include "animation_controller.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Engine {

// Transition conditions determine when a state change is allowed
enum class TransitionCondition {
    Immediate,     // Switch immediately
    OnFinish,      // Wait for current animation to finish
    CanInterrupt   // Can interrupt if priority allows
};

// Animation state with priority
struct AnimationState {
    std::string name;
    int priority = 0;  // Higher = harder to interrupt
    
    AnimationState() = default;
    AnimationState(const std::string& n, int p = 0) : name(n), priority(p) {}
};

// Transition between states
struct AnimationTransition {
    std::string from_state;
    std::string to_state;
    TransitionCondition condition = TransitionCondition::Immediate;
    std::function<bool()> predicate;  // Custom condition check (optional)
    
    AnimationTransition() = default;
    AnimationTransition(const std::string& from, const std::string& to,
                       TransitionCondition cond = TransitionCondition::Immediate,
                       std::function<bool()> pred = nullptr)
        : from_state(from), to_state(to), condition(cond), predicate(pred) {}
};

class AnimationStateMachine {
public:
    explicit AnimationStateMachine(AnimationController* controller);
    ~AnimationStateMachine() = default;
    
    // State management
    void add_state(const std::string& name, int priority = 0);
    void add_transition(const std::string& from, const std::string& to,
                       TransitionCondition condition = TransitionCondition::Immediate,
                       std::function<bool()> predicate = nullptr);
    
    // Control
    void transition_to(const std::string& state_name, bool force = false);
    void update(float dt);
    
    // Query
    std::string get_current_state() const { return current_state_name; }
    std::string get_pending_state() const { return pending_state_name; }
    int get_current_priority() const;
    bool has_pending_transition() const { return !pending_state_name.empty(); }
    
    // Debug
    std::vector<std::string> get_state_names() const;
    const AnimationState* get_state(const std::string& name) const;
    
private:
    AnimationController* controller;
    
    std::unordered_map<std::string, AnimationState> states;
    std::vector<AnimationTransition> transitions;
    
    std::string current_state_name;
    std::string pending_state_name;
    
    bool can_transition(const std::string& to_state) const;
    void execute_transition(const std::string& to_state);
};

} // namespace Engine