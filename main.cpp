#include <iostream>
#include <cstdlib>
#include <memory>
#include <list>
#include <chrono>
#include <thread>
#include <ctime>

enum {
    State_Exit = -100,
    State_Init = 0,
    State_Run,
    State_clear,
    State_Render,
    State_Wait_timePoint
};

class StateBase {
public:
    StateBase() : state(State_Init) {}

    virtual int DoState(int currentState) = 0;

    int Run() {
        state = DoState(state);
        return state;
    }

protected:
    int state;

};

class Element : public StateBase {
public:
    Element(Element* i_parent = nullptr) 
    : parent(i_parent)
    , destroyed(false) {}

    virtual ~Element() {
        Destroy();
    }

    void SetParent(Element* parent) {
        this->parent = parent;
    }

    Element* GetParent() const {
        return parent;
    }

    Element* AddChild(Element* child) {
        children.push_back(child);
        return child;
    }

    const std::list<Element*>& GetChildren() const {
        return children;
    }

    void Destroy() {
        // Destroy all children
        for (auto child : children) {
            delete child;
        }

        // Remove this element from its parent
        if (parent) {
            parent->RemoveChild(this);
        }
    }

    void RemoveChild(Element* child) {
        children.remove(child);
    }

    void SetDestroyed(bool destroyed) {
        this->destroyed = destroyed;
    }

    bool IsDestroyed() const {
        return destroyed;
    }

    int DoChildStates() {
        for (auto child : children) {
            if (child->IsDestroyed()) {
                continue;
            }

            int childState = child->Run();
            if (childState == State_Exit) {
                return State_Exit;
            }
        }
        return 0;
    }

private:
    Element* parent;
    std::list<Element*> children;
    bool destroyed;
};



class ElementFactory {
public:
    template<typename T, typename... Args>
    static T* Create(Args&&... args) {
        T* element = new T(std::forward<Args>(args)...);
        element->Run(); // Init
        return element;
    }
};

class Commodity : public Element {
public:
    Commodity(Element* parent) : Element(parent) {}

    int DoState(int currentState) override {
        switch (currentState) {
            case State_Init:
                weight = rand() % 10 + 1;
                return State_Run;
            case State_Run:
                return State_Run;
        };
        return State_Exit;
    }

    int GetWeight() const {
        return weight;
    }

private:
    int weight;
};

class Hand : public Element {
public:
    Hand(Element* parent) : Element(parent) {}

    int DoState(int currentState) override {
        switch (currentState) {
            case State_Init:
                weight = 0;
                return State_Run;
            case State_Run:
                return State_Run;
        };
        return State_Exit;
    }

    int GetWeight() const {
        return weight;
    }

    void SetWeight(int weight) {
        this->weight = weight;
    }

private:
    int weight;
};

class Human : public Element {
public:
    Human(Element* parent) : Element(parent) {}

    int DoState(int currentState) override {
        switch (currentState) {
            case State_Init:
                score = 0;
                steup = 0;
                downWeight = 0;
                leftHand = static_cast<Hand*>(AddChild(ElementFactory::Create<Hand>(this)));
                rightHand = static_cast<Hand*>(AddChild(ElementFactory::Create<Hand>(this)));

                return State_Run;
            case State_Run:
                steup += 1;

                int diff = (leftHand->GetWeight() - rightHand->GetWeight()) / 10;
                downWeight += diff;
                std::cout << "Left hand weight: " << leftHand->GetWeight() << ", Right hand weight: " << rightHand->GetWeight() << ", DownWeight: " << downWeight << std::endl;
                if (std::abs(downWeight) > 10) {
                    std::cout << "the human is down" << std::endl;
                    return State_Exit;
                }

                score += leftHand->GetWeight() + rightHand->GetWeight();
                return State_Run;
        };
        return State_Exit;
    }

    int GetScore() const {
        return score;
    }

    int GetSteup() const {
        return steup;
    }

    void AddCommodity(Commodity* commodity) {
        if (rand() % 2 == 0) {
            leftHand->SetWeight(leftHand->GetWeight() + commodity->GetWeight());
        } else {
            rightHand->SetWeight(rightHand->GetWeight() + commodity->GetWeight());
        }
        std::cout << "add commodity: " << commodity->GetWeight() << std::endl;
        AddChild(commodity);
    }

private:
    int score;
    int steup;
    int downWeight;

    Hand* leftHand;
    Hand* rightHand;
};

class LfkMove : public Element {
public:
    LfkMove() : Element() {}

    int DoState(int currentState) override {
        switch (currentState) {
            case State_Init:
                human = static_cast<Human*>(AddChild(ElementFactory::Create<Human>(this)));

                return State_Run;
            case State_Run:
                if (DoChildStates() == State_Exit) {
                    return State_Exit;
                }
                std::cout << "setup: " << human->GetSteup() << " score: " << human->GetScore() << std::endl;
                if (human->GetScore() >= 2000) {
                    std::cout << "Win Win Win!!!" << std::endl;
                    return State_Exit;
                }

                if (rand() % 100 > 50) {
                    human->AddCommodity(ElementFactory::Create<Commodity>(human));
                }
                return State_clear;
            case State_clear:
                // std::cout << "LfkMove: State 2" << std::endl;
                return State_Render;
            case State_Render:
                // std::cout << "LfkMove: State 2" << std::endl;
                return State_Wait_timePoint;
            case State_Wait_timePoint:
                std::chrono::milliseconds sleepDuration(400);
                std::this_thread::sleep_for(sleepDuration);
                return State_Run;
        };
        return State_Exit;
    }

private:
    Human* human;
};


int main() {
    srand(time(0));
    LfkMove* machine = ElementFactory::Create<LfkMove>();
    while (machine->Run() != State_Exit) {
        // std::cout << "-------------------" << std::endl;
    }

    return 0;
}
