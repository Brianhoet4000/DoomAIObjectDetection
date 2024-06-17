#pragma once
#include <functional>
#include <memory>
#include <vector>

class BehaviourTree
{
public:
    // Forward declaration of Node class
    class Node;

    // Define a type for Node pointers
    using NodePtr = std::shared_ptr<Node>;

    // Base class for all nodes in the behavior tree
    class Node
	{
    public:
        virtual bool execute() = 0;
    };

    // Action node to simulate keyboard input or mouse click
    class ActionNode : public Node
	{
    private:
        std::function<void()> m_action;

    public:
        ActionNode(std::function<void()> action) : m_action(action) {}

        bool execute() override
    	{
            m_action();
            return true;
        }
    };

    // Sequence node to execute child nodes in sequence
    class SequenceNode : public Node
	{
    private:
        std::vector<NodePtr> children;

    public:
        void addChild(const NodePtr& child)
    	{
            children.push_back(child);
        }

        bool execute() override
    	{
            for (const auto& child : children)
            {
                if (!child->execute())
                {
                    return false;
                }
            }
            return true;
        }
    };

    // Condition node to evaluate a condition
    class ConditionNode : public Node
	{
    private:
        std::function<bool()> m_condition;

    public:
        ConditionNode(std::function<bool()> condition) : m_condition(condition) {}

        bool execute() override
    	{
            return m_condition();
        }
    };
};

