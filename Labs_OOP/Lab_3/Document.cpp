#include "Document.h"

Group::Group(const Group& other) {
    align = other.align;
    border = other.border;
    border_char = other.border_char;
    std::copy(std::begin(other.margin), std::end(other.margin), margin);
    std::copy(std::begin(other.padding), std::end(other.padding), padding);
    use_mask = other.use_mask;
    mask_char = other.mask_char;
    min_width = other.min_width;
    for (const auto& child : other.children)
        children.push_back(child->clone());
}

std::unique_ptr<Element> Group::clone() const {
    return std::make_unique<Group>(*this);
}

void Group::collect(std::vector<Element*>& list) {
    list.push_back(this);
    for (auto& child : children)
        child->collect(list);
}

void Group::parseAttributes(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line[0] == '#') {
            std::istringstream ss(line.substr(1));
            std::string key;
            ss >> key;
            if (key == "align") ss >> align;
            else if (key == "border") ss >> border >> border_char;
            else if (key == "margin") for (int i = 0; i < 4; ++i) ss >> margin[i];
            else if (key == "padding") for (int i = 0; i < 4; ++i) ss >> padding[i];
            else if (key == "mask") {
                use_mask = true;
                ss >> mask_char;
            }
            else if (key == "width") ss >> min_width;
        }
        else if (line[0] == '{') {
            std::string block_content = line.substr(1);
            auto blk = std::make_unique<Group>();
            std::istringstream block_in(block_content);

            blk->parse(block_in);

            children.push_back(std::move(blk));
        }
        else if (line[0] == '$') {

            children.push_back(std::make_unique<Line>(line.substr(1)));
        }
        else if (line[0] == '}') {

            break;
        }
    }
}

void Group::parse(std::istream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line[0] == '#') {
            std::istringstream ss(line.substr(1));
            std::string key;
            ss >> key;
            if (key == "align") ss >> align;
            else if (key == "border") ss >> border >> border_char;
            else if (key == "margin") for (int i = 0; i < 4; ++i) ss >> margin[i];
            else if (key == "padding") for (int i = 0; i < 4; ++i) ss >> padding[i];
            else if (key == "mask") {
                use_mask = true;
                ss >> mask_char;
            }
            else if (key == "width") ss >> min_width;
        }
        else if (line[0] == '$') {

            children.push_back(std::make_unique<Line>(line.substr(1)));
        }
        else if (line[0] == '{') {

            std::string block_text;
            int brace_count = 1;
            while (brace_count > 0 && std::getline(in, line)) {
                if (line.empty()) continue;
                if (line[0] == '{') ++brace_count;
                if (line[0] == '}') --brace_count;

                if (brace_count > 0) {
                    block_text += line + '\n';
                }
            }

            std::istringstream nested_stream(block_text);
            auto blk = std::make_unique<Group>();
            blk->parse(nested_stream);
            children.push_back(std::move(blk));
        }
        else if (line[0] == '}') {
            return;
        }
    }
}

std::string Group::render(int) const {
    std::vector<std::string> lines;
    for (const auto& child : children) {
        std::istringstream ss(child->render());
        std::string line;
        while (std::getline(ss, line)) {
            if (use_mask) {
                std::replace_if(line.begin(), line.end(),
                    [](char c) { return !std::isspace(static_cast<unsigned char>(c)); },
                    mask_char);
            }
            lines.push_back(line);
        }
    }

    int content_width = 0;
    for (const auto& line : lines) {
        content_width = std::max(content_width, static_cast<int>(line.length()));
    }
    content_width = std::max(content_width, min_width);

    for (auto& line : lines) {
        int gap = content_width - static_cast<int>(line.length());
        if (gap > 0) {
            if (align == "right") {
                line = std::string(gap, ' ') + line;
            }
            else if (align == "center") {
                int left = gap / 2;
                int right = gap - left;
                line = std::string(left, ' ') + line + std::string(right, ' ');
            }
            else {

                line += std::string(gap, ' ');
            }
        }
    }

    std::string pad_left(padding[3], ' ');
    std::string pad_right(padding[1], ' ');
    int padded_width = content_width + padding[1] + padding[3];
    std::vector<std::string> padded_lines;

    for (int i = 0; i < padding[0]; ++i)
        padded_lines.emplace_back(padded_width, ' ');

    for (const auto& line : lines)
        padded_lines.push_back(pad_left + line + pad_right);

    for (int i = 0; i < padding[2]; ++i)
        padded_lines.emplace_back(padded_width, ' ');

    int bordered_width = padded_width + 2 * border;
    std::vector<std::string> bordered_lines;
    std::string border_line(bordered_width, border_char);

    for (int i = 0; i < border; ++i)
        bordered_lines.push_back(border_line);

    for (const auto& line : padded_lines)
        bordered_lines.push_back(
            std::string(border, border_char) + line + std::string(border, border_char)
        );

    for (int i = 0; i < border; ++i)
        bordered_lines.push_back(border_line);

    int final_width = bordered_width + margin[1] + margin[3];
    std::string margin_left(margin[3], ' ');
    std::string margin_right(margin[1], ' ');
    std::vector<std::string> final_lines;

    std::string empty_line(final_width, ' ');
    for (int i = 0; i < margin[0]; ++i)
        final_lines.push_back(empty_line);

    for (const auto& line : bordered_lines)
        final_lines.push_back(margin_left + line + margin_right);

    for (int i = 0; i < margin[2]; ++i)
        final_lines.push_back(empty_line);

    std::ostringstream out;
    for (const auto& line : final_lines)
        out << line << '\n';

    return out.str();
}

void Document::rebuildIndex() {
    flat.clear();
    if (root) root->collect(flat);
}

Document::Document() : root(std::make_unique<Group>()) {
    rebuildIndex();
}

Document::Document(const Document& other) : root(std::make_unique<Group>(*other.root)) {
    rebuildIndex();
}

Document::Document(Document&& other) noexcept = default;

Document::Document(std::istream& stream) {
    load(stream);
}

Document::Document(const std::string& text) {
    std::istringstream in(text);
    load(in);
}

Document& Document::operator=(const Document& other) {
    if (this != &other) {
        root = std::make_unique<Group>(*other.root);
        rebuildIndex();
    }
    return *this;
}

Document& Document::operator=(Document&& other) noexcept = default;

bool Document::empty() const {
    return flat.size() <= 1;
}

size_t Document::count() const {
    return flat.size();
}

void Document::remove(size_t idx) {
    if (idx == 0) clear();
    else if (idx < flat.size()) {
        Element* target = flat[idx];
        auto& children = static_cast<Group*>(root.get())->children;
        children.erase(std::remove_if(children.begin(), children.end(),
            [&](const std::unique_ptr<Element>& e) { return e.get() == target; }),
            children.end());
        rebuildIndex();
    }
}

void Document::update(size_t idx, std::istream& stream) {
    std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    update(idx, text);
}

void Document::update(size_t idx, const std::string& text) {
    if (idx == 0) {
        load(text);
        return;
    }
    if (idx >= flat.size()) return;

    std::istringstream in(text);
    auto parent = static_cast<Group*>(root.get());
    Element* target = flat[idx];

    auto it = std::find_if(parent->children.begin(), parent->children.end(),
        [&](const std::unique_ptr<Element>& e) { return e.get() == target; });

    if (it != parent->children.end()) {
        auto new_elem = std::make_unique<Group>();
        new_elem->parse(in);
        *it = std::move(new_elem);
    }
    rebuildIndex();
}

void Document::clear() {
    root = std::make_unique<Group>();
    rebuildIndex();
}

void Document::load(std::istream& stream) {
    clear();
    root->parse(stream);
    rebuildIndex();
}

void Document::load(const std::string& text) {
    std::istringstream in(text);
    load(in);
}

std::string Document::render() const {
    return root->render();
}