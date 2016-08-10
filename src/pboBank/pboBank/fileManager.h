#pragma once
#include <vector>
#include <map>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/multiprecision/cpp_int.hpp>
namespace pboBank {
	class file;
	class mod;
	class fileManager {
	public:
		explicit fileManager(std::vector<boost::shared_ptr<file>> filesInit);
		~fileManager();

		std::vector<std::string> findUnknownFiles() const;
		boost::shared_ptr<file> getFileByMD5(boost::multiprecision::uint128_t md5sum);
		void indexFile(std::string filePath,boost::shared_ptr<mod> pMod);
		void indexDirectory(std::string directoryPath, boost::shared_ptr<mod> pMod);
		void copyFileToBank(boost::shared_ptr<file> pFile, std::string originPath) const;
		uint64_t getCurrentBankSize() const;//ATTENTION! This gets the files by iterating the bank on the HDD. This is slow!
		void dropFileLinks(std::vector<boost::shared_ptr<file>> pFiles,std::string targetDirectory) const;
		static boost::multiprecision::uint128_t makeMD5Hash(std::string filepath);
		static boost::multiprecision::uint128_t getMD5fromServerPath(std::string path);
		static std::string getServerPathFromMD5(boost::multiprecision::uint128_t);
		void compressFileInBank(boost::shared_ptr<file> pFile) const;
		void uncompressFileInBank(boost::shared_ptr<file> pFile) const;
		const std::vector<boost::shared_ptr<file>>& getFiles() const { return files; }
	private:
		std::vector<boost::shared_ptr<file>> files;
		std::map<boost::multiprecision::uint128_t, boost::shared_ptr<file>> filesByMD5;

	};
}
