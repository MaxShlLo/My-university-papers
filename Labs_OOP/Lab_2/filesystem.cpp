#include "FileSystem.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <regex>


FileSystemObject::FileSystemObject(const string& name)
    : name(name), creationDate(chrono::system_clock::now()) {}

string FileSystemObject::getName() const 
{
    return name;
}

void FileSystemObject::setName(const string& newName) 
{
    name = newName;
}

chrono::system_clock::time_point FileSystemObject::getCreationDate() const 
{
    return creationDate;
}

string FileSystemObject::getFullPath() const 
{
    if (auto p = parent.lock()) {
        return p->getFullPath() + "/" + name;
    }
    return "/" + name;
}

shared_ptr<Folder> FileSystemObject::getParent() const 
{
    return parent.lock();
}

void FileSystemObject::setParent(shared_ptr<Folder> parentFolder) 
{
    parent = parentFolder;
}



File::File(const string& name, size_t size)
    : FileSystemObject(name), size(size), modifiedDate(chrono::system_clock::now()) {}

bool File::isFolder() const 
{
    return false;
}

size_t File::getSize() const 
{
    return size;
}

chrono::system_clock::time_point File::getModifiedDate() const 
{
    return modifiedDate;
}

void File::modify(size_t newSize) 
{
    size = newSize;
    modifiedDate = chrono::system_clock::now();
    if (auto p = getParent()) {
        p->updateCache(); // invalidate cache
    }
}


Folder::Folder(const string& name)
    : FileSystemObject(name), cacheValid(false), cachedSize(0), cachedCount(0) {}

bool Folder::isFolder() const 
{
    return true;
}

void Folder::add(const shared_ptr<FileSystemObject>& obj) 
{
    obj->setParent(shared_from_this());
    children.push_back(obj);
    cacheValid = false;
}

void Folder::remove(const shared_ptr<FileSystemObject>& obj) 
{
    children.erase(std::remove(children.begin(), children.end(), obj), children.end());
    cacheValid = false;
}

vector<shared_ptr<FileSystemObject>> Folder::getChildren() const 
{
    return children;
}

shared_ptr<FileSystemObject> Folder::getChildByName(const string& name) const 
{
    for (auto& child : children) 
    {
        if (child->getName() == name) return child;
    }
    return nullptr;
}

void Folder::updateCache() const 
{
    cachedSize = 0;
    cachedCount = 0;
    cachedModifiedDate = creationDate;

    for (const auto& child : children) 
    {
        if (child->isFolder()) 
        {
            auto folder = dynamic_pointer_cast<Folder>(child);
            folder->updateCache();
            cachedSize += folder->getTotalSize();
            cachedCount += 1 + folder->getAllDescendantsCount();
            if (folder->getLastModified() > cachedModifiedDate)
                cachedModifiedDate = folder->getLastModified();
        } 
        else 
        {
            auto file = dynamic_pointer_cast<File>(child);
            cachedSize += file->getSize();
            cachedCount += 1;
            if (file->getModifiedDate() > cachedModifiedDate)
                cachedModifiedDate = file->getModifiedDate();
        }
    }
    cacheValid = true;
}

size_t Folder::getTotalSize() const 
{
    if (!cacheValid) updateCache();
    return cachedSize;
}

size_t Folder::getDirectChildCount() const 
{
    return children.size();
}

size_t Folder::getAllDescendantsCount() const 
{
    if (!cacheValid) updateCache();
    return cachedCount;
}

chrono::system_clock::time_point Folder::getLastModified() const 
{
    if (!cacheValid) updateCache();
    return cachedModifiedDate;
}


FileSystem::FileSystem() 
{
    root = make_shared<Folder>("");
}

shared_ptr<Folder> FileSystem::getRoot() const 
{
    return root;
}

shared_ptr<FileSystemObject> FileSystem::get(const string& path) const 
{
    if (path.empty() || path[0] != '/')
        throw invalid_argument("Path must start with '/'");

    istringstream ss(path);
    string token;
    shared_ptr<FileSystemObject> current = root;

    getline(ss, token, '/'); // skip leading empty

    while (getline(ss, token, '/')) 
    {
        if (!current->isFolder())
            return nullptr;

        auto folder = dynamic_pointer_cast<Folder>(current);
        current = folder->getChildByName(token);
        if (!current) return nullptr;
    }

    return current;
}

shared_ptr<FileSystemObject> FileSystem::get(const string& name, shared_ptr<Folder> folder) const 
{
    return folder->getChildByName(name);
}

shared_ptr<Folder> FileSystem::createFolder(shared_ptr<Folder> parent, const string& name) 
{
    auto folder = make_shared<Folder>(name);
    parent->add(folder);
    return folder;
}

shared_ptr<File> FileSystem::createFile(shared_ptr<Folder> parent, const string& name, size_t size) 
{
    auto file = make_shared<File>(name, size);
    parent->add(file);
    return file;
}

void FileSystem::modify(shared_ptr<File> file, size_t newSize) 
{
    file->modify(newSize);
}

void FileSystem::remove(shared_ptr<FileSystemObject> obj) 
{
    if (auto parent = obj->getParent()) 
    {
        parent->remove(obj);
    }
}

void FileSystem::rename(shared_ptr<FileSystemObject> obj, const string& newName) 
{
    obj->setName(newName);
    if (auto parent = obj->getParent()) 
    {
        parent->updateCache();
    }
}

void FileSystem::move(shared_ptr<FileSystemObject> obj, shared_ptr<Folder> newParent) 
{
    if (auto parent = obj->getParent()) 
    {
        parent->remove(obj);
    }
    newParent->add(obj);
}

shared_ptr<FileSystemObject> FileSystem::copy(shared_ptr<FileSystemObject> obj, shared_ptr<Folder> newParent, const optional<string>& newName) 
{
    string name = newName ? *newName : obj->getName();

    if (obj->isFolder()) 
    {
        auto folder = make_shared<Folder>(name);
        newParent->add(folder);

        auto originalFolder = dynamic_pointer_cast<Folder>(obj);
        for (auto& child : originalFolder->getChildren()) 
        {
            copy(child, folder);
        }
        return folder;
    } 
    else 
    {
        auto file = dynamic_pointer_cast<File>(obj);
        auto newFile = make_shared<File>(name, file->getSize());
        newParent->add(newFile);
        return newFile;
    }
}

void FileSystem::print(shared_ptr<Folder> folder, int indent) const {
    string indentation(indent, ' ');
    cout << indentation << "[Folder] " << folder->getName() << "\n";

    for (const auto& child : folder->getChildren()) {
        if (auto subFolder = dynamic_pointer_cast<Folder>(child)) {
            print(subFolder, indent + 2);  // рекурсивний виклик!
        } else if (auto file = dynamic_pointer_cast<File>(child)) {
            cout << string(indent + 2, ' ') << "[File] " << file->getName() << "\n";
        }
    }
}

vector<shared_ptr<FileSystemObject>> FileSystem::find(shared_ptr<Folder> folder, const string& nameMask) const
{
    vector<shared_ptr<FileSystemObject>> result;

    string regexPattern = "^" + regex_replace(regex_replace(nameMask, regex(R"(\.)"), R"(\.)"), regex(R"(\*)"), ".*") + "$";
    regex nameRegex(regexPattern);

    for (const auto& obj : folder->getChildren()) 
    {
        if (regex_match(obj->getName(), nameRegex)) 
        {
            result.push_back(obj);
        }

        if (obj->isFolder()) 
        {
            auto nested = find(dynamic_pointer_cast<Folder>(obj), nameMask);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }

    return result;
}

vector<shared_ptr<FileSystemObject>> FileSystem::find(shared_ptr<Folder> folder, size_t minSize, size_t maxSize) const
{
    vector<shared_ptr<FileSystemObject>> result;

    for (const auto& obj : folder->getChildren()) 
    {
        if (!obj->isFolder()) 
        {
            auto file = dynamic_pointer_cast<File>(obj);
            if (file->getSize() >= minSize && file->getSize() <= maxSize) 
            {
                result.push_back(file);
            }
        } 
        else 
        {
            auto nested = find(dynamic_pointer_cast<Folder>(obj), minSize, maxSize);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }
    return result;
}