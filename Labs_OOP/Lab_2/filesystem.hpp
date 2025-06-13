#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <optional>

using namespace std;

class Folder;

class FileSystemObject 
{
protected:
    string name;
    chrono::system_clock::time_point creationDate;
    weak_ptr<Folder> parent;

public:
    FileSystemObject(const string& name);
    virtual ~FileSystemObject() = default;

    virtual bool isFolder() const = 0;

    string getName() const;
    void setName(const string& newName);
    chrono::system_clock::time_point getCreationDate() const;

    string getFullPath() const;
    shared_ptr<Folder> getParent() const;
    void setParent(shared_ptr<Folder> parentFolder);
};

class File : public FileSystemObject 
{
    size_t size;
    chrono::system_clock::time_point modifiedDate;

public:
    File(const string& name, size_t size);

    bool isFolder() const override;

    size_t getSize() const;
    chrono::system_clock::time_point getModifiedDate() const;

    void modify(size_t newSize);
};

class Folder : public FileSystemObject, public enable_shared_from_this<Folder> 
{
    vector<shared_ptr<FileSystemObject>> children;
    mutable bool cacheValid;
    mutable size_t cachedSize;
    mutable size_t cachedCount;
    mutable chrono::system_clock::time_point cachedModifiedDate;

public:
    Folder(const string& name);

    bool isFolder() const override;
    void updateCache() const;
    void add(const shared_ptr<FileSystemObject>& obj);
    void remove(const shared_ptr<FileSystemObject>& obj);

    vector<shared_ptr<FileSystemObject>> getChildren() const;
    shared_ptr<FileSystemObject> getChildByName(const string& name) const;

    size_t getTotalSize() const;
    size_t getDirectChildCount() const;
    size_t getAllDescendantsCount() const;
    chrono::system_clock::time_point getLastModified() const;
};

class FileSystem // Fasade FileSystem
{
    shared_ptr<Folder> root;

public:
    FileSystem();

    shared_ptr<Folder> getRoot() const;
    shared_ptr<FileSystemObject> get(const string& path) const;
    shared_ptr<FileSystemObject> get(const string& name, shared_ptr<Folder> folder) const;

    shared_ptr<Folder> createFolder(shared_ptr<Folder> parent, const string& name);
    shared_ptr<File> createFile(shared_ptr<Folder> parent, const string& name, size_t size);

    void modify(shared_ptr<File> file, size_t newSize);
    void remove(shared_ptr<FileSystemObject> obj);
    void rename(shared_ptr<FileSystemObject> obj, const string& newName);
    void move(shared_ptr<FileSystemObject> obj, shared_ptr<Folder> newParent);
    void print(shared_ptr<Folder> folder, int indent = 0) const;
    vector<shared_ptr<FileSystemObject>> find(
        shared_ptr<Folder> folder,
        const string& nameMask) const;

    vector<shared_ptr<FileSystemObject>> find(
        shared_ptr<Folder> folder,
        size_t minSize,
        size_t maxSize) const;
    shared_ptr<FileSystemObject> copy(shared_ptr<FileSystemObject> obj, shared_ptr<Folder> newParent, const optional<string>& newName = nullopt);
};
#endif