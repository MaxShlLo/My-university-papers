#include "FileSystem.hpp"
#include <iostream>

using namespace std;

int main() {
    FileSystem fs;

    // Отримуємо кореневу папку
    auto root = fs.getRoot();

    // Створюємо деякі папки
    auto docs_folder = fs.createFolder(root, "Documents");
    auto pics_folder = fs.createFolder(root, "Pictures");
    auto music_folder = fs.createFolder(root, "Music");
    auto work_folder = fs.createFolder(docs_folder, "Work");

    // Створюємо деякі файли
    auto file1 = fs.createFile(docs_folder, "doc1.docx", 12000);
    auto file2 = fs.createFile(docs_folder, "doc2.txt", 3400);
    auto pic1 = fs.createFile(pics_folder, "photo1.jpg", 240000);
    auto pic2 = fs.createFile(pics_folder, "photo2.jpg", 180000);
    auto song = fs.createFile(music_folder, "song.mp3", 500);
    auto work_file = fs.createFile(work_folder, "report.pdf", 22000);

    // Виводимо початкову структуру
    cout << "\nInitial File System:\n";
    fs.print(root);

    // Перейменування файлу
    fs.rename(file2, "test1.txt");

    // Модифікація файлу
    fs.modify(file1, 12500);

    // Копіювання папки Pictures в Documents
    fs.copy(pics_folder, docs_folder, "BackupPics");

    // Переміщення файлу
    fs.move(file1, music_folder);

    // Пошук за шаблоном (*.jpg у Documents)
    cout << "\nFind *.jpg files in /Documents:\n";
    auto jpgs = fs.find(docs_folder, "*.jpg");
    for (auto& f : jpgs) {
        cout << "  - " << f->getFullPath() << "\n";
    }

    cout << "\nFind *.pdf files in /Documents:\n";
    auto pdfs = fs.find(docs_folder, "*.pdf");
    for (auto& f : pdfs) {
        cout << "  - " << f->getFullPath() << "\n";
    }

    // Пошук файлів за розміром
    cout << "\nFind files in / with size between 10000 and 200000 bytes:\n";
    auto sized = fs.find(root, 10000, 200000);
    for (auto& f : sized) {
        auto file = dynamic_pointer_cast<File>(f);
        if (file) {
            cout << "  - " << file->getFullPath() << " (" << file->getSize() << " bytes)\n";
        }
    }

    // Видалення одного з файлів
    fs.remove(pic2);

    // Видалення цілої папки
    fs.remove(work_folder);

    // Вивід фінальної структури
    cout << "\nFinal File System:\n";
    fs.print(root);

    return 0;
}