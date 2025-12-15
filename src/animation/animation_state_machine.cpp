// src/Animation/AnimationStateMachine.cpp
#include "animation_state_machine.h"

namespace Engine {

AnimationStateMachine::AnimationStateMachine(AnimationController* controller)
    : controller(controller) {}

void AnimationStateMachine::addState(const std::string& name, int priority) {
    AnimationState state;
    state.name = name;
    state.priority = priority;
    states[name] = state;
}

void AnimationStateMachine::addTransition(const std::string& from, 
                                         const std::string& to,
                                         TransitionCondition condition,
                                         std::function<bool()> predicate) {
    AnimationTransition transition;
    transition.fromState = from;
    transition.toState = to;
    transition.condition = condition;
    transition.predicate = predicate;
    transitions.push_back(transition);
}

void AnimationStateMachine::transitionTo(const std::string& stateName, bool force) {
    if (currentStateName == stateName && !force) return;
    
    if (!force && !canTransition(stateName)) {
        pendingStateName = stateName;
        return;
    }
    
    executeTransition(stateName);
}

void AnimationStateMachine::update(float dt) {
    controller->update(dt);
    
    // Check for automatic transitions
    for (const auto& transition : transitions) {
        if (transition.fromState != currentStateName) continue;
        
        // Check predicate
        if (transition.predicate && !transition.predicate()) continue;
        
        // Check condition
        bool shouldTransition = false;
        switch (transition.condition) {
            case TransitionCondition::Immediate:
                shouldTransition = true;
                break;
            case TransitionCondition::OnFinish:
                shouldTransition = controller->isFinished();
                break;
            case TransitionCondition::CanInterrupt:
                shouldTransition = canTransition(transition.toState);
                break;
        }
        
        if (shouldTransition) {
            executeTransition(transition.toState);
            return;  // Only one transition per frame
        }
    }
    
    // Try pending transition if current animation finished
    if (!pendingStateName.empty() && controller->isFinished()) {
        executeTransition(pendingStateName);
        pendingStateName.clear();
    }
}

bool AnimationStateMachine::canTransition(const std::string& toState) const {
    int currentPriority = getCurrentPriority();
    
    auto it = states.find(toState);
    if (it == states.end()) return false;
    
    int toPriority = it->second.priority;
    
    // Can transition if new state has higher or equal priority
    return toPriority >= currentPriority;
}

void AnimationStateMachine::executeTransition(const std::string& toState) {
    currentStateName = toState;
    controller->play(toState, true);  // Restart animation
}

int AnimationStateMachine::getCurrentPriority() const {
    auto it = states.find(currentStateName);
    return (it != states.end()) ? it->second.priority : 0;
}

} // namespace Engine