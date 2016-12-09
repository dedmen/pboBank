#pragma once
#include <vector>
#include <memory>
#include <stack>
#include <chrono>
#include <set>

//https://docs.oracle.com/javase/7/docs/api/constant-values.html#java.io.ObjectStreamConstants.TC_EXCEPTION

enum javaSerializeType : char {
    javaSerializeType_noType = 0x0,
    javaSerializeType_typeDefEnd = 0x70,//p
    javaSerializeType_TC_REFERENCE = 0x71,
    javaSerializeType_TC_CLASSDESC = 0x72,//r
    javaSerializeType_TC_OBJECT = 0x73,//s
    javaSerializeType_TC_STRING = 0x74,
    javaSerializeType_TC_ARRAY = 0x75,
    javaSerializeType_TC_CLASS = 0x76,
    javaSerializeType_TC_BLOCKDATA = 0x77,
    javaSerializeType_TC_ENDBLOCKDATA = 0x78,//x
    javaSerializeType_TC_RESET = 0x79,
    javaSerializeType_TC_BLOCKDATALONG = 0x7A,
    javaSerializeType_TC_EXCEPTION = 0x7B,
    javaSerializeType_TC_LONGSTRING = 0x7C,
    javaSerializeType_TC_PROXYCLASSDESC = 0x7D,
    javaSerializeType_TC_ENUM = 0x7E,
    javaSerializeType_defVarInt32 = 0x49, //I
    javaSerializeType_defVarLong = 0x4A,//J int64 8byte
    javaSerializeType_defVarClassInstance = 0x4C,
    javaSerializeType_defVarBoolean = 0x5A//Z
};

static int32_t java_STREAM_MAGIC = 0xACED0000;
static int32_t java_STREAM_VERSION = 5;
static int32_t java_streamHeader = java_STREAM_MAGIC & java_STREAM_VERSION;



class javaSerializeContext;

class javaSerializeable {
public:
    virtual ~javaSerializeable() {}

    virtual void serialize(std::vector<char>& output, javaSerializeContext& context) = 0;
};

class javaSerializeContext {
public:
    javaSerializeContext() {
        parents.push(0);//Only one needed... error buffer.. whatever
        parents.push(0);
        parents.push(0);
    }
    uintptr_t getInstance() {
        return  instanceCount++;
    };

    std::stack<uintptr_t> parents;
    uintptr_t instanceCount = 0x7e0000;
    struct {
        uintptr_t treeDirectory = 0;//fr.soe.a3s.domain.repository.SyncTreeDirectory
        uintptr_t treeDirectoryMemberType = 0;//Lfr/soe/a3s/domain/repository/SyncTreeDirectory; To be used on class variable type definitions
        uintptr_t treeLeaf = 0;//fr.soe.a3s.domain.repository.SyncTreeLeaf
        uintptr_t list = 0;//Ljava/util/List;
        uintptr_t arrayList = 0;//java.util.ArrayList;
        uintptr_t string = 0;//Ljava/lang/String;
        uintptr_t serverInfo = 0;//fr.soe.a3s.domain.repository.ServerInfo
        uintptr_t date = 0;//Ljava/util/Date;
        uintptr_t set = 0;//Ljava/util/Set;
        uintptr_t hashSet = 0;//Ljava/util/HashSet;
    } definitions;
};

class a3syncSyncDefinition_TreeDirectory {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};

class a3syncSyncDefinition_ArrayList {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};

class a3syncSyncDefinition_TreeLeaf {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};

class a3syncSyncDefinition_ServerInfo {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};


//https://docs.oracle.com/javase/7/docs/api/java/util/Date.html
//https://docs.oracle.com/javase/7/docs/api/serialized-form.html#java.util.Date
class a3syncSyncDefinition_Date {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};

//https://docs.oracle.com/javase/7/docs/api/serialized-form.html#java.util.HashSet
class a3syncSyncDefinition_HashSet {
public:
    static void serialize(std::vector<char>& output, javaSerializeContext& context);
};

class javaDate : public javaSerializeable {
public:
    javaDate(std::chrono::system_clock::time_point _date) : date(_date) {}
    ~javaDate() override {};
    void serialize(std::vector<char>& output, javaSerializeContext& context) override;

    std::chrono::system_clock::time_point date;
};

class javaHashSet_string : public javaSerializeable {
public:
    javaHashSet_string(std::set<std::string> _data) : data(_data) {}
    ~javaHashSet_string() override {};
    void serialize(std::vector<char>& output, javaSerializeContext& context) override;

    std::set<std::string> data;
};


class a3syncServerInfo : public javaSerializeable {
public:
    a3syncServerInfo(
    bool _compressedPboFilesOnly,
    bool _noPartialFileTransfer,
    int _numberOfConnections,
    int64_t _numberOfFiles,
    bool _repositoryContentUpdated,
    int _revision,
    int64_t _totalFilesSize,
    std::chrono::system_clock::time_point _buildDate,
    std::set<std::string> _hiddenFolderPaths) :
    compressedPboFilesOnly(_compressedPboFilesOnly),
    noPartialFileTransfer(_noPartialFileTransfer),
    numberOfConnections(_numberOfConnections),
    numberOfFiles(_numberOfFiles),
    repositoryContentUpdated(_repositoryContentUpdated),
    revision(_revision),
    totalFilesSize(_totalFilesSize),
    buildDate(_buildDate),
    hiddenFolderPaths(_hiddenFolderPaths){}
    ~a3syncServerInfo() override {};
    void serialize(std::vector<char>& output, javaSerializeContext& context) override;

    bool compressedPboFilesOnly;
    bool noPartialFileTransfer;
    int numberOfConnections;
    int64_t numberOfFiles;
    bool repositoryContentUpdated;
    int revision;
    int64_t totalFilesSize;
    std::chrono::system_clock::time_point buildDate;
    std::set<std::string> hiddenFolderPaths;


    uintptr_t myInstance = 0;

};


class a3syncSyncLeaf : public javaSerializeable {


public:
    a3syncSyncLeaf(bool _compressed, int64_t _compressedSize, bool _deleted, int64_t _size, bool _updated, std::string _name, std::string _sha1) :
        compressed(_compressed), compressedSize(_compressedSize), deleted(_deleted), size(_size), updated(_updated), name(_name), sha1(_sha1) {}
    ~a3syncSyncLeaf() override {};
    void serialize(std::vector<char>& output, javaSerializeContext& context) override;

    bool compressed;
    int64_t compressedSize;
    bool deleted;
    int64_t size;
    bool updated;
    std::string name;
    std::string sha1;
    uintptr_t myInstance = 0;
};

class a3syncSyncDirectory : public javaSerializeable {


public:
    a3syncSyncDirectory(bool _deleted, bool _hidden, bool _markAsAddon, bool _updated, std::vector<std::shared_ptr<javaSerializeable>>&& _list, std::string _name)
        : deleted(_deleted), hidden(_hidden), markAsAddon(_markAsAddon), updated(_updated), list(std::move(_list)), name(_name) {}
    ~a3syncSyncDirectory() override {};
    void serialize(std::vector<char>& output, javaSerializeContext& context) override;
    bool deleted = false;
    bool hidden = false;
    bool markAsAddon = true;
    bool updated = false;
    std::vector<std::shared_ptr<javaSerializeable>> list;
    std::string name;
    uintptr_t myInstance = 0;
};


class javaSerializer {
public:
    static void writeString(std::vector<char>& output, std::string msg, javaSerializeType writeType = javaSerializeType_TC_STRING);
    static void writeInt64(std::vector<char>& output, int64_t num);
    static void writeInt32(std::vector<char>& output, int32_t num);
    static void writeInt16(std::vector<char>& output, int16_t num);
    static void writeBool(std::vector<char>& output, bool val);
    static void testSync();
    static void testServerInfo();

};



class a3sync {
public:
    a3sync();
    ~a3sync();


};

