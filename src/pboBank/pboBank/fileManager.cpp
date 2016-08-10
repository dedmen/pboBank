#include "fileManager.h"
#include "files.h"
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "global.h"
#include <openssl/md5.h>
#include "database.h"
pboBank::fileManager::fileManager(std::vector<boost::shared_ptr<file>> filesInit) : files(filesInit) {
	for (auto &it : files) {
		filesByMD5[it->md5sum] = it;
	}
}


pboBank::fileManager::~fileManager() {}

std::vector<std::string> pboBank::fileManager::findUnknownFiles() const {
	std::vector<std::string> unknownFiles;

	boost::filesystem::path fileBankBase(GLOBAL.getFileBankBasePath());

	for (auto &iterator : boost::filesystem::recursive_directory_iterator(fileBankBase)) {

		if (boost::filesystem::is_regular_file(iterator.status())) {
			boost::multiprecision::uint128_t md5sum = getMD5fromServerPath(iterator.path().string());
			if (filesByMD5.find(md5sum) == filesByMD5.end())  //hash not found so we dont know this one
				unknownFiles.emplace_back(iterator.path().generic_string());
		}
	}
	return unknownFiles;
}

boost::shared_ptr<pboBank::file> pboBank::fileManager::getFileByMD5(boost::multiprecision::uint128_t md5sum) {
	auto found = filesByMD5.find(md5sum);
	if (found == filesByMD5.end())
		return nullptr;
	return found->second;
}

void pboBank::fileManager::indexFile(std::string filePath, boost::shared_ptr<mod> pMod) {
	//#Changesets indexFile needs to create a change for adding a file
	if (!pMod) {
		printf("ERROR cant indexFile without Mod %s\n", filePath.c_str());
		return;
	}

	if (!boost::filesystem::exists(filePath)) {
		printf("ERROR cant index file because it doesnt exist: %s\n", filePath.c_str());
		return;
	}
	boost::multiprecision::uint128_t md5sum = makeMD5Hash(filePath);

	uint32_t filesize = static_cast<uint32_t>(boost::filesystem::file_size(filePath));
	auto filePtr = boost::make_shared<file>();
	filePtr->fileSize = filesize;
	filePtr->md5sum = md5sum;
	filePtr->addMod(pMod);
	filePtr->setFilename(filePath.substr(filePath.find_last_of("/") + 1));
	auto found = filesByMD5.find(md5sum);
	if (found != filesByMD5.end()) {
		if (boost::algorithm::contains(found->second->getMods(), std::vector<boost::shared_ptr<mod>>{ pMod })) {//#TODO use std::find or something more.. non-ugly
			printf("WARNING file %s already indexed\n", filePtr->getFilename().c_str());
			copyFileToBank(filePtr, filePath); //#remove only used for debugging because im constantly deleting the bank
			return;	 //file deletes itself
		}
		printf("ERROR file with same hash already exists: %s\n", filePtr->getFilename().c_str());
	}
	if (GLOBAL.getDatabase()->insertFile(filePtr,pMod)) {
		copyFileToBank(filePtr, filePath);
		files.emplace_back(filePtr);
		printf("Indexed file %s of Mod %s\n", filePtr->getFilename().c_str(), pMod->name.c_str());
	}
}

void pboBank::fileManager::indexDirectory(std::string directoryPath, boost::shared_ptr<mod> pMod) {
	if (!boost::filesystem::exists(directoryPath)) {
		printf("ERROR cant index directory because it doesnt exist: %s\n", directoryPath.c_str());
		return;
	}
	for (auto &iterator : boost::filesystem::recursive_directory_iterator(directoryPath)) {

		if (boost::filesystem::is_regular_file(iterator.status())) {
			indexFile(iterator.path().generic_string(), pMod);
		}
	}
}

void pboBank::fileManager::copyFileToBank(boost::shared_ptr<file> pFile, std::string originPath) const {
	printf("Copying %s to bank\n", pFile->getFilename().c_str());
	if (!boost::filesystem::create_directories(getServerPathFromMD5(pFile->md5sum))) {
		//printf("ERROR couldnt make path %s\n", getServerPathFromMD5(pFile->md5sum).c_str());//This also happens when path already exists.. 
	}
	std::string targetPath = getServerPathFromMD5(pFile->md5sum) + pFile->getFilename();
	if (boost::filesystem::exists(targetPath)) {
		printf("ERROR copyFileToBank target already exists %s\n", targetPath.c_str());
		return;
	}
	boost::filesystem::copy(originPath, targetPath);
}

uint64_t pboBank::fileManager::getCurrentBankSize() const {
	if (!boost::filesystem::exists(GLOBAL.getFileBankBasePath()))
		return 0;
	size_t size = 0;
	for (boost::filesystem::recursive_directory_iterator it(GLOBAL.getFileBankBasePath());
		it != boost::filesystem::recursive_directory_iterator();
		++it) {
		if (!is_directory(*it))
			size += boost::filesystem::file_size(*it);
	}
	return size;

}

void pboBank::fileManager::dropFileLinks(std::vector<boost::shared_ptr<file>> pFiles, std::string targetDirectory) const {
	boost::filesystem::create_directories(targetDirectory);
	 for (auto &it : pFiles) {
		 std::string filePath = getServerPathFromMD5(it->md5sum) + it->getFilename();
		 std::string targetPath = targetDirectory + it->getFilename();

		if(boost::filesystem::exists(targetPath)) {
			boost::filesystem::remove(targetPath);
		}
		//boost::system::error_code err;
		boost::filesystem::create_hard_link(filePath, targetPath/*, err*/);
		//printf("ERROR %s", err.message().c_str());
	 }
}

// used by bin2hex for conversion via stream.
struct bin2hex_str {
	std::ostream& os;
	bin2hex_str(std::ostream& os) : os(os) {}
	void operator ()(unsigned char ch) const {
		os << std::hex
			<< std::setw(2)
			<< static_cast<int>(ch);
	}
};
std::string bin2hex(const std::vector<unsigned char>& bin) {
	std::ostringstream oss;
	oss << std::setfill('0');
	std::for_each(bin.begin(), bin.end(), bin2hex_str(oss));
	return oss.str();
}

boost::multiprecision::uint128_t pboBank::fileManager::makeMD5Hash(std::string filepath) {
	boost::multiprecision::uint128_t md5sum;


	MD5_CTX ctx;
	MD5_Init(&ctx);

	std::ifstream ifs(filepath, std::ios::binary);

	char file_buffer[4096];
	while (ifs.read(file_buffer, sizeof(file_buffer)) || ifs.gcount()) {
		MD5_Update(&ctx, file_buffer, ifs.gcount());
	}
	//unsigned char digest[MD5_DIGEST_LENGTH] = {};
	std::vector<unsigned char> digest(MD5_DIGEST_LENGTH);
	MD5_Final(static_cast<unsigned char*>(digest.data()), &ctx);
	//std::cout << bin2hex(digest) << "\n";	 //#testcase


	md5sum.assign("0x" + bin2hex(digest));
	//std::cout << std::setfill('0') << std::setw(32) << std::hex << md5sum; //#testcase
	return md5sum;
}

const boost::regex md5FromPathRegex(".*\\/((?:[a-z0-9]{2,}\\/){16,})");
boost::multiprecision::uint128_t pboBank::fileManager::getMD5fromServerPath(std::string path) {	 //#TODO test
	// /home/pbobank/trolll/filebank/a3/cc/a2/b2/aa/1e/3b/5b/3b/5a/ad/99/a8/52/90/74/franz.pbo
	std::string hashString = boost::regex_replace(path, md5FromPathRegex, "\\1", boost::match_default | boost::format_sed);
	boost::replace_all(hashString, "/", "");
	path.insert(0, "0x");
	boost::multiprecision::uint128_t intsum(path);
	return intsum;
}

std::string pboBank::fileManager::getServerPathFromMD5(boost::multiprecision::uint128_t md5sum) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(32) << std::hex << md5sum;
	std::string md5string = ss.str();
	for (int i = 32; i >= 2; i -= 2) {
		md5string.insert(i, "/");
	}
	return 	GLOBAL.getFileBankBasePath() + md5string;
}
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/copy.hpp>
void pboBank::fileManager::compressFileInBank(boost::shared_ptr<file> pFile) const {
	
	std::string filePath = getServerPathFromMD5(pFile->md5sum) + pFile->getFilename();
	//#TODO check if file exists

	boost::iostreams::filtering_istream os;
	boost::filesystem::ifstream inStream(filePath,std::ios_base::binary);
	boost::filesystem::ofstream outStream(filePath + ".cmpress", std::ios_base::binary);
	os.push(boost::iostreams::zlib_compressor{ boost::iostreams::zlib::best_speed,4096});
	os.push(inStream);

	/*auto x = */boost::iostreams::copy(os, outStream);
	//printf("%lld \n", x);
	os.pop();
	inStream.close();
	boost::filesystem::remove(filePath);
}

void pboBank::fileManager::uncompressFileInBank(boost::shared_ptr<file> pFile) const {
	std::string filePath = getServerPathFromMD5(pFile->md5sum) + pFile->getFilename();
	//#TODO check if file exists
 
	boost::iostreams::filtering_istream os;
	boost::filesystem::ifstream inStream(filePath + ".cmpress", std::ios_base::binary);
	boost::filesystem::ofstream outStream(filePath, std::ios_base::binary);

	os.push(boost::iostreams::zlib_decompressor{});
	os.push(inStream);
	/*auto x = */boost::iostreams::copy(os, outStream);
	//printf("%lld \n", x);
	os.pop();
	inStream.close();
	boost::filesystem::remove(filePath + ".cmpress");
}
