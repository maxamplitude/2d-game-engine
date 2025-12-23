#include "animation_state_machine.h"
#include <stdexcept>
#include <algorithm>

namespace Engine {

AnimationStateMachine::AnimationStateMachine(AnimationController* controller)
    : controller(controller) {
    if (!controller) {
        throw std::invalid_argument("AnimationController cannot be null");
    }
}

void AnimationStateMachine::add_state(const std::string& name, int priority) {
    AnimationState state(name, priority);
    states[name] = state;
}

void AnimationStateMachine::add_transition(const std::string& from, 
                                          const std::string& to,
                                          TransitionCondition condition,
                                          std::function<bool()> predicate) {
    AnimationTransition transition(from, to, condition, predicate);
    transitions.push_back(transition);
}

void AnimationStateMachine::transition_to(const std::string& state_name, bool force) {
    // State doesn't exist
    if (states.find(state_name) == states.end()) {
        return;
    }
    
    // Already in this state
    if (current_state_name == state_name && !force) {
        return;
    }
    
    // If forcing, always transition
    if (force) {
        execute_transition(state_name);
        pending_state_name.clear();
        return;
    }
    
    // Check if we can transition based on priority
    if (!can_transition(state_name)) {
        // Can't transition now, queue it
        pending_state_name = state_name;
        return;
    }
    
    // Execute transition
    execute_transition(state_name);
}

void AnimationStateMachine::update(float dt) {
    // Update animation controller
    controller->update(dt);
    
    // Check for automatic transitions
    for (const auto& transition : transitions) {
        // Only check transitions from current state
        if (transition.from_state != current_state_name) {
            continue;
        }
        
        // Check custom predicate (if provided)
        if (transition.predicate && !transition.predicate()) {
            continue;
        }
        
        // Check transition condition
        bool should_transition = false;
        
        switch (transition.condition) {
            case TransitionCondition::Immediate:
                should_transition = true;
                break;
                
            case TransitionCondition::OnFinish:
                should_transition = controller->is_finished();
                break;
                
            case TransitionCondition::CanInterrupt:
                should_transition = can_transition(transition.to_state);
                break;
        }
        
        if (should_transition) {
            execute_transition(transition.to_state);
            return;  // Only one transition per frame
        }
    }
    
    // Try pending transition once the current animation is done; pending transitions
    // are allowed to complete even if they are lower priority (they were queued).
    if (!pending_state_name.empty() && controller->is_finished()) {
        execute_transition(pending_state_name);
        pending_state_name.clear();
    }
}

bool AnimationStateMachine::can_transition(const std::string& to_state) const {
    int current_priority = get_current_priority();
    
    auto it = states.find(to_state);
    if (it == states.end()) {
        return false;
    }
    
    int to_priority = it->second.priority;
    
    // Can transition if new state has higher or equal priority
    return to_priority >= current_priority;
}

void AnimationStateMachine::execute_transition(const std::string& to_state) {
    current_state_name = to_state;
    controller->play(to_state, true);  // Always restart animation
}

int AnimationStateMachine::get_current_priority() const {
    auto it = states.find(current_state_name);
    return (it != states.end()) ? it->second.priority : 0;
}

std::vector<std::string> AnimationStateMachine::get_state_names() const {
    std::vector<std::string> names;
    names.reserve(states.size());
    for (const auto& [name, _] : states) {
        names.push_back(name);
    }
    return names;
}

const AnimationState* AnimationStateMachine::get_state(const std::string& name) const {
    auto it = states.find(name);
    return (it != states.end()) ? &it->second : nullptr;
}

} // namespace Engine