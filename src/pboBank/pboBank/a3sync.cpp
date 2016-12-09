#include "a3sync.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>


a3sync::a3sync() {}


a3sync::~a3sync() {}

void javaSerializer::writeString(std::vector<char>& output, std::string msg, javaSerializeType writeType) {
    if (writeType) output.push_back(writeType);
    writeInt16(output, msg.length());
    output.insert(output.end(), msg.begin(), msg.end());
}
  //#TODO check current system endianess
void javaSerializer::writeInt64(std::vector<char>& output, int64_t num) {
    char* pt = reinterpret_cast<char*>(&num);

    output.push_back(pt[7]);
    output.push_back(pt[6]);
    output.push_back(pt[5]);
    output.push_back(pt[4]);

    output.push_back(pt[3]);
    output.push_back(pt[2]);
    output.push_back(pt[1]);
    output.push_back(pt[0]);
}

void javaSerializer::writeInt32(std::vector<char>& output, int32_t num) {
    char* pt = reinterpret_cast<char*>(&num);
    output.push_back(pt[3]);
    output.push_back(pt[2]);
    output.push_back(pt[1]);
    output.push_back(pt[0]);
}

void javaSerializer::writeInt16(std::vector<char>& output, int16_t num) {
    char* pt = reinterpret_cast<char*>(&num);
    output.push_back(pt[1]);
    output.push_back(pt[0]);
}

void javaSerializer::writeBool(std::vector<char>& output, bool val) {
    output.push_back(val ? 1 : 0);
}
#include "compressionCache.h"
#include <boost/iostreams/device/file.hpp>

#include <boost/iostreams/filter/gzip.hpp>


void javaSerializer::testSync() {
    std::vector<char> output;
    writeInt32(output, java_streamHeader);
   /*
     a3syncSyncDirectory base(false, false, false, false, {
        std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{
            std::make_shared<a3syncSyncLeaf>(false,0,false,4,false,"test1.txt","7110eda4d09e062aa5e4a390b0a572ac0d2c0220"),
                std::make_shared<a3syncSyncLeaf>(false,0,false,8,false,"test2.txt","7c222fb2927d828af22f592134e8932480637c0d")
    },"@test"),

        std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{

                std::make_shared<a3syncSyncLeaf>(false,0,false,8,false,"test3.txt","7c222fb2927d828af22f592134e8932480637c0d"),
                    std::make_shared<a3syncSyncLeaf>(false,0,false,4,false,"test4.txt","7110eda4d09e062aa5e4a390b0a572ac0d2c0220"),
            },"@test2"),


                std::make_shared<a3syncSyncDirectory>(false,false,true,false, std::vector<std::shared_ptr<javaSerializeable>>{
                std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{
                    std::make_shared<a3syncSyncLeaf>(false,0,false,0,false,"test6.txt","0"),
                        std::make_shared<a3syncSyncLeaf>(false,0,false,0,false,"test7.txt","0")
                },"addons")
            },"@test4")




    }, "racine");
   */


    a3syncSyncDirectory base(false, false, false, false, {
        std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{
            std::make_shared<a3syncSyncLeaf>(false,0,false,4,false,"IDontHaveBoobs.txt","7110eda4d09e062aa5e4a390b0a572ac0d2c0220"),
            std::make_shared<a3syncSyncLeaf>(false,0,false,8,false,"ILikeBoobs.txt","7c222fb2927d828af22f592134e8932480637c0d")
        },"@test"),
        std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{
            std::make_shared<a3syncSyncLeaf>(false,0,false,8,false,"IHaveBoobs.txt","7c222fb2927d828af22f592134e8932480637c0d"),
            std::make_shared<a3syncSyncLeaf>(false,0,false,4,false,"ILikeOtherBoobs.txt","7110eda4d09e062aa5e4a390b0a572ac0d2c0220"),
         },"@test2"),
         std::make_shared<a3syncSyncDirectory>(false,false,true,false, std::vector<std::shared_ptr<javaSerializeable>>{
            std::make_shared<a3syncSyncDirectory>(false,false,false,false, std::vector<std::shared_ptr<javaSerializeable>>{
                std::make_shared<a3syncSyncLeaf>(false,0,false,0,false,"ILikeLuke.txt","0"),
                std::make_shared<a3syncSyncLeaf>(false,0,false,0,false,"Pff..ILoveHim.txt","0")
            },"addons")
        },"@test4")
    }, "racine");
    javaSerializeContext ctx;
    base.serialize(output, ctx);
    output.push_back(javaSerializeType_typeDefEnd);






    boost::iostreams::filtering_ostream m_targetFile;

    m_targetFile.push(boost::iostreams::gzip_compressor());

    m_targetFile.push(boost::iostreams::file_sink("T:\\outsync", std::ios_base::binary | std::ios::out));

    m_targetFile.write(output.data(), output.size());
}

void javaSerializer::testServerInfo() {
    std::vector<char> output;
    writeInt32(output, java_streamHeader);
    javaSerializeContext ctx;
    a3syncServerInfo(false,true,10,1337,true,156,13123123,std::chrono::system_clock::now(),{"@serverside"}).serialize(output, ctx);



    boost::iostreams::filtering_ostream m_targetFile;

    m_targetFile.push(boost::iostreams::gzip_compressor());

    m_targetFile.push(boost::iostreams::file_sink("T:\\outServerInfo", std::ios_base::binary | std::ios::out));

    m_targetFile.write(output.data(), output.size());
}

void a3syncSyncDefinition_TreeDirectory::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.treeDirectory) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.treeDirectory);
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.treeDirectory = context.getInstance();
    javaSerializer::writeString(output, "fr.soe.a3s.domain.repository.SyncTreeDirectory", javaSerializeType_TC_CLASSDESC);
    unsigned char rawData[11] = {
        0xD8, 0x5F, 0xEB, 0x3C, 0x78, 0x4F, 0x59, 0xF8, 0x02, 0x00, 0x07
    };
    output.insert(output.end(), &rawData[0], &rawData[11]);
    javaSerializer::writeString(output, "deleted", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "hidden", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "markAsAddon", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "updated", javaSerializeType_defVarBoolean);


    javaSerializer::writeString(output, "list", javaSerializeType_defVarClassInstance);
    context.definitions.list = context.getInstance();
    javaSerializer::writeString(output, "Ljava/util/List;");

    javaSerializer::writeString(output, "name", javaSerializeType_defVarClassInstance);
    context.definitions.string = context.getInstance();
    javaSerializer::writeString(output, "Ljava/lang/String;");

    javaSerializer::writeString(output, "parent", javaSerializeType_defVarClassInstance);
    context.definitions.treeDirectoryMemberType = context.getInstance();
    javaSerializer::writeString(output, "Lfr/soe/a3s/domain/repository/SyncTreeDirectory;");
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
}

void a3syncSyncDefinition_ArrayList::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.arrayList) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.arrayList);
        context.getInstance();//current Instance of array
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.arrayList = context.getInstance();
    javaSerializer::writeString(output, "java.util.ArrayList", javaSerializeType_TC_CLASSDESC);

    unsigned char rawData[11] = {
        0x78, 0x81, 0xD2, 0x1D, 0x99, 0xC7, 0x61, 0x9D, 0x03, 0x00, 0x01
    };
    output.insert(output.end(), &rawData[0], &rawData[11]);
    javaSerializer::writeString(output, "size", javaSerializeType_defVarInt32);
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
    context.getInstance();//First instance when we create definition
}

void a3syncSyncDefinition_TreeLeaf::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.treeLeaf) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.treeLeaf);
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.treeLeaf = context.getInstance();
    javaSerializer::writeString(output, "fr.soe.a3s.domain.repository.SyncTreeLeaf", javaSerializeType_TC_CLASSDESC);
    unsigned char rawData[11] = {
        0x7A, 0xCE, 0xD8, 0x4D, 0x24, 0x27, 0x12, 0xD7, 0x02, 0x00, 0x08
    };
    output.insert(output.end(), &rawData[0], &rawData[11]);


    javaSerializer::writeString(output, "compressed", javaSerializeType_defVarBoolean);

    javaSerializer::writeString(output, "compressedSize", javaSerializeType_defVarLong);
    javaSerializer::writeString(output, "deleted", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "size", javaSerializeType_defVarLong);
    javaSerializer::writeString(output, "updated", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "name", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);//#TODO write  pointer func. Like string taking optional arg
    javaSerializer::writeInt32(output, context.definitions.string);
    javaSerializer::writeString(output, "parent", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);
    javaSerializer::writeInt32(output, context.definitions.treeDirectoryMemberType);
    javaSerializer::writeString(output, "sha1", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);
    javaSerializer::writeInt32(output, context.definitions.string);

    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
}

void a3syncSyncLeaf::serialize(std::vector<char>& output, javaSerializeContext& context) {


    /*
    class fr.soe.a3s.domain.repository.SyncTreeLeaf implements java.io.Serializable {
    boolean compressed;
    long compressedSize;
    boolean deleted;
    long size;
    boolean updated;
    java.lang.String name;
    fr.soe.a3s.domain.repository.SyncTreeDirectory parent;
    java.lang.String sha1;
    }
    */
    //bool compressed = false;
    //int32_t compressedSize = 0;
    //bool deleted = false;
    //int32_t size = 574;
    //bool updated = false;
    //std::string name = "ace_advanced_ballistics.pbo.ace_3.8.2.13-1c8955d9.bisign";
    //uintptr_t parent = 0x7e0009;
    //std::string sha1 = "cc3a8cfc92341223690606de7d893c5cb0a9489a";

    a3syncSyncDefinition_TreeLeaf::serialize(output, context);
    myInstance = context.getInstance();
    javaSerializer::writeBool(output, compressed);
    javaSerializer::writeInt64(output, compressedSize);
    javaSerializer::writeBool(output, deleted);
    javaSerializer::writeInt64(output, size);
    javaSerializer::writeBool(output, updated);
    javaSerializer::writeString(output, name);
    context.getInstance();//String inst counter
    output.push_back(javaSerializeType_TC_REFERENCE);
    javaSerializer::writeInt32(output, context.parents.top());
    javaSerializer::writeString(output, sha1);
    context.getInstance();//String inst counter
}

void a3syncSyncDirectory::serialize(std::vector<char>& output, javaSerializeContext& context) {



    /*
    class fr.soe.a3s.domain.repository.SyncTreeDirectory implements java.io.Serializable {
    boolean deleted;
    boolean hidden;
    boolean markAsAddon;
    boolean updated;
    java.util.List list;
    java.lang.String name;
    fr.soe.a3s.domain.repository.SyncTreeDirectory parent;
    }
    */
    //bool deleted = false;
    //bool hidden = false;
    //bool markAsAddon = true;
    //bool updated = false;
    ////list
    //std::string name;
    //uintptr_t parent;

    a3syncSyncDefinition_TreeDirectory::serialize(output, context);
    myInstance = context.getInstance();
    javaSerializer::writeBool(output, deleted);
    javaSerializer::writeBool(output, hidden);
    javaSerializer::writeBool(output, markAsAddon);
    javaSerializer::writeBool(output, updated);

    //Write list here
    /*
    List contains a whole directory structure of subfolders. and all files in current folder

    */
    //Write list start

    a3syncSyncDefinition_ArrayList::serialize(output, context);

    javaSerializer::writeInt32(output, list.size());//list size
    output.push_back(javaSerializeType_TC_BLOCKDATA);
    output.push_back(0x04);
    javaSerializer::writeInt32(output, list.size());//list size



    context.parents.push(myInstance);
    for (auto& it : list) {
        it->serialize(output, context);
    }
    context.parents.pop();
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    //Write list end


    javaSerializer::writeString(output, name); //#TODO string has instance count do in string writer
    context.getInstance();
    if (context.parents.top()) {
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.parents.top());
    }
       
}

void a3syncSyncDefinition_ServerInfo::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.serverInfo) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.serverInfo);
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.serverInfo = context.getInstance();
    javaSerializer::writeString(output, "fr.soe.a3s.domain.repository.ServerInfo", javaSerializeType_TC_CLASSDESC);
    unsigned char rawData[11] = {
        0x6A, 0xD2, 0x10, 0x56, 0xC3, 0x45, 0xA7, 0xF9, 0x02, 0x00, 0x09
    };

    output.insert(output.end(), &rawData[0], &rawData[11]);


    javaSerializer::writeString(output, "compressedPboFilesOnly", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "noPartialFileTransfer", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "numberOfConnections", javaSerializeType_defVarInt32);
    javaSerializer::writeString(output, "numberOfFiles", javaSerializeType_defVarLong);
    javaSerializer::writeString(output, "repositoryContentUpdated", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "revision", javaSerializeType_defVarInt32);
    javaSerializer::writeString(output, "totalFilesSize", javaSerializeType_defVarLong);


    javaSerializer::writeString(output, "buildDate", javaSerializeType_defVarClassInstance);

    context.getInstance();
    javaSerializer::writeString(output, "Ljava/util/Date;");

    javaSerializer::writeString(output, "hiddenFolderPaths", javaSerializeType_defVarClassInstance);

    context.definitions.set = context.getInstance();
    javaSerializer::writeString(output, "Ljava/util/Set;");



    output.push_back(javaSerializeType_TC_REFERENCE);//#TODO write  pointer func. Like string taking optional arg

    
    javaSerializer::writeString(output, "deleted", javaSerializeType_defVarBoolean);

    javaSerializer::writeString(output, "updated", javaSerializeType_defVarBoolean);
    javaSerializer::writeString(output, "name", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);//#TODO write  pointer func. Like string taking optional arg
    javaSerializer::writeInt32(output, context.definitions.string);
    javaSerializer::writeString(output, "parent", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);
    javaSerializer::writeInt32(output, context.definitions.treeDirectoryMemberType);
    javaSerializer::writeString(output, "sha1", javaSerializeType_defVarClassInstance);
    output.push_back(javaSerializeType_TC_REFERENCE);
    javaSerializer::writeInt32(output, context.definitions.string);

    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
}

void a3syncSyncDefinition_Date::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.date) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.date);
        context.getInstance();//current Instance of date
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.date = context.getInstance();
    javaSerializer::writeString(output, "java.util.Date", javaSerializeType_TC_CLASSDESC);

    unsigned char rawData[11] = {
        0x68, 0x6A, 0x81, 0x01, 0x4B, 0x59, 0x74, 0x19, 0x03, 0x00, 0x00
    };
    output.insert(output.end(), &rawData[0], &rawData[11]);
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
    context.getInstance();//First instance when we create definition
}

void a3syncSyncDefinition_HashSet::serialize(std::vector<char>& output, javaSerializeContext& context) {
    if (context.definitions.hashSet) {
        output.push_back(javaSerializeType_TC_OBJECT);
        output.push_back(javaSerializeType_TC_REFERENCE);
        javaSerializer::writeInt32(output, context.definitions.hashSet);
        context.getInstance();//current Instance of date
        return;
    }
    output.push_back(javaSerializeType_TC_OBJECT);
    context.definitions.hashSet = context.getInstance();
    javaSerializer::writeString(output, "java.util.HashSet", javaSerializeType_TC_CLASSDESC);

    unsigned char rawData[11] = {
        0xBA, 0x44, 0x85, 0x95, 0x96, 0xB8, 0xB7, 0x34, 0x03, 0x00, 0x00
    };

    output.insert(output.end(), &rawData[0], &rawData[11]);
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
    output.push_back(javaSerializeType_typeDefEnd);
    context.getInstance();//First instance when we create definition
}

void javaDate::serialize(std::vector<char>& output, javaSerializeContext& context) {
    a3syncSyncDefinition_Date::serialize(output, context);
    output.push_back(0x08);//Dunno what this is
    output.push_back(0x0);
    output.push_back(0x0);
    javaSerializer::writeInt64(output, date.time_since_epoch().count());
    output.push_back(0x78);//Dunno what this is
}

void javaHashSet_string::serialize(std::vector<char>& output, javaSerializeContext& context) {
    a3syncSyncDefinition_HashSet::serialize(output, context);
    output.push_back(javaSerializeType_TC_BLOCKDATA);
    //The capacity of the backing HashMap instance (int), and its load factor (float) are emitted, 
    //followed by the size of the set (the number of elements it contains) (int), followed by all of its elements (each an Object) in no particular order.
    javaSerializer::writeInt32(output,data.size() + 16 - (data.size() % 16));//rounded up to next multiple of 16  - test instance had 16 here with 2 elements in hashSet
    javaSerializer::writeInt32(output, 0x3F40'0000); //load factor float of 0.75. meaning at 75% capacity its capacity will be increased
    javaSerializer::writeInt32(output, data.size());
    for (auto& it : data) {
        javaSerializer::writeString(output, it);
    }
    output.push_back(javaSerializeType_TC_ENDBLOCKDATA);
}

void a3syncServerInfo::serialize(std::vector<char>& output, javaSerializeContext& context) {

    a3syncSyncDefinition_ServerInfo::serialize(output, context);

    javaSerializer::writeBool(output, compressedPboFilesOnly);
    javaSerializer::writeBool(output, noPartialFileTransfer);
    javaSerializer::writeInt32(output, numberOfConnections);
    javaSerializer::writeInt64(output, numberOfFiles);
    javaSerializer::writeBool(output, repositoryContentUpdated);
    javaSerializer::writeInt32(output, revision);
    javaSerializer::writeInt64(output, totalFilesSize);

    javaDate(buildDate).serialize(output, context);
    javaHashSet_string(hiddenFolderPaths).serialize(output, context);
}
