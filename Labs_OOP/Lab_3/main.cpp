#include <iostream>
#include <fstream>
#include "Document.h"

int main() {
    std::ifstream file("Test1.txt");
    if (!file) return 1;

    Document doc(file);
    std::cout << "=== Render document from file ===\n";
    std::cout << doc.render() << "\n";

    Document doc1(doc);
    std::cout << "=== Render copy document ===\n";
    std::cout << doc1.render() << "\n";

      // Створення нового документа вручну (update на корені)
    std::string doc_text =
        "#align left\n"
        "#border 2 *\n"
        "#margin 5 5 5 5\n"
        "#padding 10 10 10 10\n"
        "{\n"
        "$text1\n"
        "}\n";

    doc1.update(0, doc_text);
    std::cout << "=== Render after update (new group) ===\n";
    std::cout << doc1.render() << "\n";

    // Додавання ще одного рядка через update
    std::string text_end = "$the end";
    doc1.update(1, text_end); // додає в кінець
    std::cout << "=== Render after adding a new element ===\n";
    std::cout << doc1.render() << "\n";

    return 0;
} 