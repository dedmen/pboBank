#include "compressionCache.h"
#include <boost/make_shared.hpp>
#include <boost/filesystem/fstream.hpp>
#include "fileServer.h"


#define useCache
pboBank::compressionCache::compressionCache() : cacheSize(0) {}

/*
	Problems with caching
  the basic_refKeeper iterates and copies the whole buffer which is unnecessary and wastes a lot of cpu time and memory allocations
  pboBank::copyToBuf waits till the whole stream is compressed before outputting data thats because its not a symmetric_filter

	even in disabled state refKeeper does copy and iterate the buffer

	Best solution:
 create a Source/Sink for each file.. pass the source to the fileServer which calls boost::iostreams::read (As seen in the refKeeper code)
	 And pass the Sink to a thread which compresses and pushes into the sink
	 http://www.boost.org/doc/libs/1_46_0/libs/iostreams/doc/concepts/source.html
 http://www.boost.org/doc/libs/1_46_0/libs/iostreams/doc/concepts/sink.html



Currently the refKeeper iterating every char is capping speed to about 9MB/s max and about 5MB/s average
Thats why im completly avoiding the compressionCache in the fileServer... for now
*/

pboBank::compressionCache::~compressionCache() {}
//#TODO implement handling if cache is full
boost::shared_ptr<pboBank::compressedFileBase> pboBank::compressionCache::getCompressedFileBuffer(boost::shared_ptr<file> pFile) {
	boost::unique_lock<boost::mutex> lock(access);
#ifdef useCache
	std::map<boost::shared_ptr<file>, boost::shared_ptr<cachedFile>>::iterator found;
	if (pFile->fileSize > maxCachingFileSize || (found = cachedFiles.find(pFile)) == cachedFiles.end()) {
		printf("compression cache unknown file\n");
#endif		
		std::string filePath = GLOBAL.getFileManager()->getServerPathFromMD5(pFile->md5sum) + pFile->getFilename();
#ifdef 	useCache
		//printf("%d < %d %d \n", pFile->fileSize, maxCachingFileSize, pFile->fileSize < maxCachingFileSize);
		if (pFile->fileSize < maxCachingFileSize) {
			printf("caching file %s of size %u\n", pFile->getFilename().c_str(), pFile->fileSize);
			auto pCachedFile = boost::make_shared<cachedFile>();
			auto pCompressedFile = boost::make_shared<compressedFileCompressing>(filePath, pFile->fileSize, pCachedFile);

			cachedFiles[pFile] = pCachedFile;	//#TODO function addFileToCache(shared_ptr cinfo) and removeFileFromCache which take care about cacheSize
			cacheSize += pFile->fileSize;
			return pCompressedFile;
		}
#endif
		return boost::make_shared<compressedFileCompressing>(filePath, pFile->fileSize, nullptr);
#ifdef useCache
	} else {
		printf("get file from cache\n");
		return boost::make_shared<compressedFileCached>(found->second);
	}
#endif
}
#include <vector>
void pboBank::compressionCache::threadCompressionTakeover(boost::shared_ptr<compressedFileCompressing> pCompressor, uint64_t maxSpeedPerSec) {
	printf("thread takeover \n");

	boost::thread t([pCompressor, maxSpeedPerSec]() {
		boost::shared_ptr<compressedFileCompressing> refKeeper = pCompressor; //not sure if this is necessary
		std::vector<char> buf;
		buf.resize(1024 * 1024);
		auto maxBytesPerMillisecond = static_cast<double>(maxSpeedPerSec) / 1000.0 / 1000.0;


		auto overallFirst = boost::posix_time::microsec_clock::universal_time();
		std::streamsize totalRead = 0;
		while (!pCompressor->eof()) {
			auto first = boost::posix_time::microsec_clock::universal_time();
			auto dataRead = refKeeper->read(buf.data(), buf.capacity());
			totalRead += dataRead;
			auto compressionTime = boost::posix_time::microsec_clock::universal_time() - first;
			auto bytesPerMillisecondProcessed = (dataRead / static_cast<double>(std::max(1ll, static_cast<long long>(compressionTime.total_microseconds()))));
			//auto bytesOverkillPerMillisecond = (bytesPerMillisecondProcessed - maxBytesPerMillisecond)/100;	//Dont know why /100 but... it works this way


			auto allowedBytes = compressionTime.total_milliseconds()* (maxSpeedPerSec / 1000);
			auto waitMs = (dataRead - allowedBytes) / bytesPerMillisecondProcessed;
			auto timetaken = compressionTime.total_microseconds();

			//#TODO math is good enough 6MB/s with 7MB limit and 5,3MB/s with 5MB limit. not really on it but close enough for me
			boost::this_thread::sleep_for(boost::chrono::microseconds(static_cast<int>(round(std::min(1000000.0, waitMs)))));
		}
		auto overallTime = boost::posix_time::microsec_clock::universal_time() - overallFirst;
		printf("Tcompression speed %llu mics %llu bytes %f MB/s\n", overallTime.total_microseconds(), totalRead,
			static_cast<double>(totalRead) / 1024.0 / 1024.0 //MB
			/
			std::max(1.0, static_cast<double>(overallTime.total_microseconds()) / 1000.0 / 1000.0));//seconds
	});
}

pboBank::compressedFileCompressing::compressedFileCompressing(std::string filePath, std::streamsize fileSize, boost::shared_ptr<cachedFile> _pCachedFile) :pCachedFile(_pCachedFile) {
	std::ios_base::iostate exceptionMask = inStream.exceptions() | std::ios::failbit;
	inStream.exceptions(exceptionMask);
	try {
		inStream.open(filePath, std::ios_base::binary);
	}
	catch (std::ios_base::failure& e) {
		printf("open err %s %s %s\n", e.what(), filePath.c_str(), strerror(errno));
	}
	m_sourceFile.push(boost::iostreams::zlib_compressor{ compressionMode,static_cast<int>(compressingBuffer.size()) });
	m_sourceFile.push(inStream);
	if (pCachedFile) {
		pCachedFile->reserve(fileSize);
		pCachedFile->used();
		printf("copy compressing to cache \n");
	}
}

std::streamsize pboBank::compressedFileCompressing::read(char* s, std::streamsize n) {
	auto first = boost::posix_time::microsec_clock::universal_time();

	try {
		m_sourceFile.read(s, n);
	}
	catch (std::ios_base::failure& e) {
		printf("open err %s %s\n", e.what(), strerror(errno));
	}
	auto dataRead = m_sourceFile.gcount();
	auto second = boost::posix_time::microsec_clock::universal_time();
	timeSpent += (second - first).total_microseconds();
	bytesCompressed += dataRead;
	if (pCachedFile) {
		pCachedFile->write(s, dataRead);
	}
	return dataRead;
}

bool pboBank::compressedFileCompressing::valid() {
	if (m_sourceFile)
		return true;
	return false;
}

bool pboBank::compressedFileCompressing::fail() {
	bool failed = m_sourceFile.fail();
	if (failed && pCachedFile)
		pCachedFile->doneWriting();
	return failed;
}

bool pboBank::compressedFileCompressing::eof() {
	bool eofed = m_sourceFile.eof();
	if (eofed && pCachedFile)
		pCachedFile->doneWriting();
	return eofed;
}

bool pboBank::compressedFileCompressing::isCaching() const {
	return static_cast<bool>(pCachedFile);
}

std::streamsize pboBank::compressedFileCached::read(char* s, std::streamsize n) {
	if (eof() || fail())
		return 0;
	auto bytesRead = pCachedFile->read(s, curPos, n);
	curPos += bytesRead;
	return bytesRead;
}

bool pboBank::compressedFileCached::valid() {	//cant make these const because member func stuff
	if (pCachedFile)
		return true;
	return false;
}

bool pboBank::compressedFileCached::fail() {
	if (!pCachedFile)
		return true;
	return false;
}

bool pboBank::compressedFileCached::eof() {
	if (!pCachedFile)
		return true;
	return (pCachedFile->isDone() && curPos == pCachedFile->getEof());
}
