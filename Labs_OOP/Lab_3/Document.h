#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <istream>
#include <algorithm>

class Element {
public:
    virtual ~Element() = default;
    virtual std::string render(int width = 0) const = 0;
    virtual std::unique_ptr<Element> clone() const = 0;
    virtual void collect(std::vector<Element*>& list) = 0;
};

class Line : public Element {
    std::string text;
public:
    explicit Line(const std::string& t) : text(t) {}

    std::string render(int width = 0) const override {
        return text;
    }

    std::unique_ptr<Element> clone() const override {
        return std::make_unique<Line>(*this);
    }

    void collect(std::vector<Element*>& list) override {
        list.push_back(this);
    }
};

class Group : public Element {
public:
    std::vector<std::unique_ptr<Element>> children;
    std::string align = "left";
    int border = 0;
    char border_char = ' ';
    int margin[4] = { 0, 0, 0, 0 };   
    int padding[4] = { 0, 0, 0, 0 };
    bool use_mask = false;
    char mask_char = ' ';
    int min_width = 0;

    Group() = default;
    Group(const Group& other);
    std::unique_ptr<Element> clone() const override;

    void parseAttributes(std::istream& in);
    void parse(std::istream& in);

    std::string render(int width = 0) const override;
    void collect(std::vector<Element*>& list) override;
};


class Document {
    std::unique_ptr<Group> root;
    std::vector<Element*> flat;

    void rebuildIndex();

public:
    Document();
    Document(const Document& other);
    Document(Document&& other) noexcept;

    explicit Document(std::istream& stream);
    explicit Document(const std::string& text);

    Document& operator=(const Document& other);
    Document& operator=(Document&& other) noexcept;

    bool empty() const;
    size_t count() const;

    void remove(size_t idx);
    void update(size_t idx, std::istream& stream);
    void update(size_t idx, const std::string& text);

    void clear();
    void load(std::istream& stream);
    void load(const std::string& text);

    std::string render() const;
};
#endif