#pragma once
#include <string>
#include <vector>
#include <boost/asio/io_service.hpp>
namespace pboBank
{
	class fileManager;
	class database;
	class modManager;
	class fileServer;
	class compressionCache;
}

class global {
public:
	global();
	~global();
	std::string getFileBankBasePath() const { return fileBankBasePath; }
	pboBank::database* getDatabase() const { return pDatabase; }
	pboBank::fileManager* getFileManager() const { return pFileManager; }
	pboBank::modManager* getModManager() const { return pModManager; }
	pboBank::compressionCache* getCompressionCache() const { return pCompressionCache; }
	void init();
	void run();//gets called from main thread
	void signal(int signum);
private:
	boost::asio::io_service mainIOservice;
	pboBank::fileManager* pFileManager;
	pboBank::database* pDatabase;
	pboBank::modManager* pModManager;
	pboBank::fileServer* pFileServer;
	pboBank::compressionCache* pCompressionCache;
	std::string fileBankBasePath; //Path ends with /
};





class globalHelper {
	public:
	static std::vector<char> HexToBytes(const std::string& hex) {
		std::vector<char> bytes;

		for (unsigned int i = 0; i < hex.length(); i += 2) {
			std::string byteString = hex.substr(i, 2);
			char byte = (char) strtol(byteString.c_str(), NULL, 16);
			bytes.emplace_back(byte);
		}

		return bytes;
	};
};
extern global GLOBAL;
