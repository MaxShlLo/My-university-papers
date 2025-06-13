#ifndef FIGURE_H
#define FIGURE_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <string>


class Figure {
protected:
    std::string name; 
public:
    virtual ~Figure() = default;


    void setName(const std::string& newName) {
        name = newName;
    }


    const std::string& getName() const {
        return name;
    }

    virtual void draw(sf::RenderWindow& window) const = 0;


    virtual sf::FloatRect getBounds() const = 0;

    virtual void setPosition(float x, float y) = 0;


    virtual std::shared_ptr<Figure> clone() const = 0;
};


class Circle : public Figure {
private:
    sf::CircleShape shape;

public:
    Circle(float radius, float x = 0.f, float y = 0.f) {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(x, y);
    }

    std::shared_ptr<Figure> clone() const override {
        auto copy = std::make_shared<Circle>(shape.getRadius(), shape.getPosition().x, shape.getPosition().y);
        copy->setName(name);
        return copy;
    }

    void draw(sf::RenderWindow& window) const override {
        window.draw(shape);
    }

    float getRadius() const {
        return shape.getRadius();
    }

    sf::FloatRect getBounds() const override {
        return shape.getGlobalBounds();
    }

    void setPosition(float x, float y) override {
        shape.setPosition(x, y);
    }
};

class Rectangle : public Figure {
private:
    sf::RectangleShape shape;

public:
    Rectangle(float width, float height, float x = 0.f, float y = 0.f) {
        shape.setSize({ width, height });
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x, y);
    }

    std::shared_ptr<Figure> clone() const override {
        auto copy = std::make_shared<Rectangle>(
            shape.getSize().x, shape.getSize().y
        );
        copy->setPosition(shape.getPosition().x, shape.getPosition().y);
        copy->setName(name);
        return copy;
    }

    void draw(sf::RenderWindow& window) const override {
        window.draw(shape);
    }

    sf::FloatRect getBounds() const override {
        return shape.getGlobalBounds();
    }

    void setPosition(float x, float y) override {
        shape.setPosition(x, y);
    }
};


class Group : public Figure {
private:

    std::vector<std::pair<std::string, std::shared_ptr<Figure>>> namedElements;
    float x, y, width, height;

public:
    Group(float width, float height, float x = 0.f, float y = 0.f)
        : x(x), y(y), width(width), height(height) {}


    const std::vector<std::pair<std::string, std::shared_ptr<Figure>>>& getNamedElements() const {
        return namedElements;
    }

    std::shared_ptr<Figure> clone() const override {
        auto copy = std::make_shared<Group>(width, height, x, y);
        copy->setName(name);
        for (const auto& [elementName, element] : namedElements) {
            float localX = element->getBounds().left - x;
            float localY = element->getBounds().top - y;
            copy->addWithName(elementName, element->clone(), localX, localY);
        }
        return copy;
    }

    void addWithName(const std::string& name, std::shared_ptr<Figure> element, float localX, float localY) {
        element->setPosition(x + localX, y + localY);
        element->setName(name);
        namedElements.emplace_back(name, element);
    }

    

    std::shared_ptr<Figure> getElement(const std::string& name) const {
        auto it = std::find_if(namedElements.begin(), namedElements.end(),
            [&name](const auto& pair) {
                return pair.first == name;
            });

        if (it != namedElements.end()) {
            return it->second;
        }

        std::cout << "Element with name: \"" << name << "\" not detected.\n";
        return nullptr;
    }

    void setPosition(float newX, float newY) override {
        float dx = newX - x;
        float dy = newY - y;
        x = newX;
        y = newY;

        for (auto& [name, element] : namedElements) {
            element->setPosition(element->getBounds().left + dx, element->getBounds().top + dy);
        }
    }

    void removeElement(const std::string& name) {
        auto it = std::find_if(namedElements.begin(), namedElements.end(),
            [&name](const auto& pair) { return pair.first == name; });
        if (it != namedElements.end()) {
            namedElements.erase(it);
        }
        else {
            std::cout << "Element with name: \"" << name << "\" not detected.\n";
        }
    }


    void draw(sf::RenderWindow& window) const override {

        sf::View originalView = window.getView();

        sf::View groupView;
        groupView.setViewport({
            x / window.getSize().x,
            y / window.getSize().y,
            width / window.getSize().x,
            height / window.getSize().y
            });
        groupView.setCenter(x + width / 2.f, y + height / 2.f);
        groupView.setSize(width, height);

        window.setView(groupView);

        for (const auto& [name, element] : namedElements) {
            element->draw(window);
        }

        window.setView(originalView);

        sf::RectangleShape border;
        border.setPosition(x, y);
        border.setSize({ width, height });
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(sf::Color::White);
        border.setOutlineThickness(2.f);
        window.draw(border);
    }

    sf::FloatRect getBounds() const override {
        return { x, y, width, height };
    }
};

class GroupIterator {
private:
    std::vector<std::shared_ptr<Figure>> elements;
    size_t index = 0;

    void collectElements(const std::shared_ptr<Group>& group) {
        for (const auto& [name, element] : group->getNamedElements()) {
            auto subgroup = std::dynamic_pointer_cast<Group>(element);
            if (subgroup) {
                collectElements(subgroup);
            }
            elements.push_back(element);
        }
    }

public:
    explicit GroupIterator(const std::shared_ptr<Group>& group) {
        collectElements(group);
    }

    bool hasNext() const {
        return index < elements.size();
    }

    std::shared_ptr<Figure> next() {
        if (!hasNext()) {
            std::cout << "Elements the ends.\n";
            return nullptr;
        }
        return elements[index++];
    }
};



class GraphicsFacade {
private:
    std::shared_ptr<Group> mainGroup;
    std::shared_ptr<Group> currentGroup;
    std::vector<std::pair<std::function<void()>, std::function<void()>>> actionHistory;


    void printGroupStructure(const std::shared_ptr<Group>& group, int indent) const {
        std::string indentStr(indent, '+');

        for (const auto& [name, element] : group->getNamedElements()) {
            auto subgroup = std::dynamic_pointer_cast<Group>(element);
            if (subgroup) {
                std::cout << indentStr << "Group ("
                    << subgroup->getBounds().left << ", "
                    << subgroup->getBounds().top << ")" << std::endl;

                printGroupStructure(subgroup, indent + 1);
            }
            else {
                auto circle = std::dynamic_pointer_cast<Circle>(element);
                if (circle) {
                    std::cout << indentStr << "Circle ("
                        << circle->getBounds().left << ", "
                        << circle->getBounds().top << ") R="
                        << circle->getRadius() << std::endl;
                }

                auto rectangle = std::dynamic_pointer_cast<Rectangle>(element);
                if (rectangle) {
                    std::cout << indentStr << "Rectangle ("
                        << rectangle->getBounds().left << ", "
                        << rectangle->getBounds().top << ") "
                        << rectangle->getBounds().width << "*"
                        << rectangle->getBounds().height << std::endl;
                }
            }
        }
    }

public:
    GraphicsFacade(float width, float height, float x = 0.f, float y = 0.f) {
        mainGroup = std::make_shared<Group>(width, height, x, y);
        currentGroup = mainGroup;
    }

    std::shared_ptr<Group> getMainGroup() const {
        return mainGroup;
    }

    void printDocumentStructure() const {
        std::cout << "Document structure:\n";
        printGroupStructure(mainGroup, 0);
    }

    void iterateAll(const std::function<void(const std::shared_ptr<Figure>&)>& func) {
        GroupIterator iterator(mainGroup);
        while (iterator.hasNext()) {
            func(iterator.next());
        }
    }

    void executeAction(const std::function<void()>& action, const std::function<void()>& undo) {
        action();
        actionHistory.emplace_back(action, undo);
    }

    void undo() {
        if (actionHistory.empty()) {
            std::cout << "No action to cancel.\n";
            return;
        }
        auto lastUndo = actionHistory.back().second;
        lastUndo();
        actionHistory.pop_back();
    }




    void setCurrentGroup(const std::string& name) {
        static std::shared_ptr<Group> previousGroup = nullptr;

        auto oldGroup = currentGroup;

        auto action = [this, name, oldGroup]() {
            if (name.empty()) {
                if (!previousGroup) {
                    std::cout << ("No previous group to return to.\n");
                }
                currentGroup.swap(previousGroup);
            }
            else {
                auto group = std::dynamic_pointer_cast<Group>(currentGroup->getElement(name));
                if (!group) {
                    std::cout << ("Group with this name was not found.\n");
                }
                previousGroup = currentGroup;
                currentGroup = group;
            }
            };

        auto undo = [this, oldGroup]() {
            currentGroup = oldGroup;
            };

        executeAction(action, undo);
    }

    void resetToMainGroup() {
        auto prevGroup = currentGroup;

        auto action = [this]() {
            currentGroup = mainGroup;
            };

        auto undo = [this, prevGroup]() {
            currentGroup = prevGroup;
            };

        executeAction(action, undo);
    }

    void addCircle(const std::string& name, float radius, float localX, float localY) {
        auto action = [this, name, radius, localX, localY]() {
            currentGroup->addWithName(name, std::make_shared<Circle>(radius), localX, localY);
            };

        auto undo = [this, name]() {
            currentGroup->removeElement(name);
            };

        executeAction(action, undo);
    }

    void addRectangle(const std::string& name, float width, float height, float localX, float localY) {
        auto action = [this, name, width, height, localX, localY]() {
            currentGroup->addWithName(name, std::make_shared<Rectangle>(width, height), localX, localY);
            };

        auto undo = [this, name]() {
            currentGroup->removeElement(name);
            };

        executeAction(action, undo);
    }

    void addGroup(const std::string& name, float width, float height, float localX, float localY) {
        auto group = std::make_shared<Group>(width, height);

        auto action = [this, name, group, localX, localY]() {
            currentGroup->addWithName(name, group, localX, localY);
            };

        auto undo = [this, name]() {
            currentGroup->removeElement(name);
            };

        executeAction(action, undo);
    }

    void copyElementByName(const std::string& name, const std::string& newName, float offsetX, float offsetY) {
        auto element = currentGroup->getElement(name);
        auto copy = element->clone();

        auto action = [this, newName, copy, offsetX, offsetY]() {
            currentGroup->addWithName(newName, copy, offsetX, offsetY);
            };

        auto undo = [this, newName]() {
            currentGroup->removeElement(newName);
            };

        executeAction(action, undo);
    }

    void moveElement(const std::string& name, float newX, float newY) {
        auto element = currentGroup->getElement(name);
        float oldX = element->getBounds().left;
        float oldY = element->getBounds().top;

        auto action = [this, name, newX, newY]() {
            currentGroup->getElement(name)->setPosition(newX, newY);
            };

        auto undo = [this, name, oldX, oldY]() {
            currentGroup->getElement(name)->setPosition(oldX, oldY);
            };

        executeAction(action, undo);
    }

    std::pair<std::shared_ptr<Group>, std::shared_ptr<Figure>> findElementWithGroup(float x, float y) const {
        return findInGroupWithGroup(mainGroup, x, y);
    }

    std::pair<std::shared_ptr<Group>, std::shared_ptr<Figure>> findInGroupWithGroup(
        const std::shared_ptr<Group>& group, float x, float y) const {

        for (auto it = group->getNamedElements().rbegin(); it != group->getNamedElements().rend(); ++it) {
            auto element = it->second;

            auto subgroup = std::dynamic_pointer_cast<Group>(element);
            if (subgroup && subgroup->getBounds().contains(x, y)) {
                auto result = findInGroupWithGroup(subgroup, x, y);
                if (result.second != nullptr) {
                    return result;
                }
            }

            if (element->getBounds().contains(x, y)) {
                return { group, element };
            }
        }

        return { nullptr, nullptr };
    }


    void transferElement(const std::string& elementName, const std::string& targetGroupName, float newX, float newY) {
        auto element = currentGroup->getElement(elementName);
        auto sourceGroup = currentGroup;

        std::shared_ptr<Group> targetGroup = mainGroup;
        if (!targetGroupName.empty()) {
            targetGroup = std::dynamic_pointer_cast<Group>(mainGroup->getElement(targetGroupName));
            if (!targetGroup) {
                std::cout<<("Target group not found.\n");
            }
        }

        auto action = [this, elementName, element, targetGroup, newX, newY]() {
            currentGroup->removeElement(elementName);
            targetGroup->addWithName(elementName, element, newX, newY);
            };

        auto undo = [this, elementName, element, sourceGroup]() {
            currentGroup = sourceGroup;
            currentGroup->addWithName(elementName, element, element->getBounds().left, element->getBounds().top);
            };

        executeAction(action, undo);
    }

    void removeElementByName(const std::string& name) {
        auto element = currentGroup->getElement(name);

        auto action = [this, name]() {
            currentGroup->removeElement(name);
            };

        auto undo = [this, name, element]() {
            currentGroup->addWithName(name, element, element->getBounds().left, element->getBounds().top);
            };

        executeAction(action, undo);
    }

    void draw(sf::RenderWindow& window) {
        mainGroup->draw(window);
    }
};

#endif // FIGURE_H