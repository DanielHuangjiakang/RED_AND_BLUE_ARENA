#include <functional>
#pragma once

class DecisionTreeNode {
public:
    virtual ~DecisionTreeNode() = default;
    virtual void execute() = 0;
};

class ActionNode : public DecisionTreeNode {
public:
    explicit ActionNode(std::function<void()> action) : action(action) {}
    void execute() override { action(); }

private:
    std::function<void()> action;
};

class ConditionNode : public DecisionTreeNode {
public:
    ConditionNode(std::function<bool()> condition, DecisionTreeNode* trueBranch, DecisionTreeNode* falseBranch)
        : condition(condition), trueBranch(trueBranch), falseBranch(falseBranch) {}

    void execute() override {
        if (condition()) {
            trueBranch->execute();
        } else {
            falseBranch->execute();
        }
    }

private:
    std::function<bool()> condition;
    DecisionTreeNode* trueBranch;
    DecisionTreeNode* falseBranch;
};