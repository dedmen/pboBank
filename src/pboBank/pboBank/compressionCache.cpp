#include "compressionCache.h"
#include <boost/make_shared.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem/fstream.hpp>
#include "fileServer.h"
#include <boost/iostreams/stream.hpp>
#include "streamFilter_CopyToBuf.h"
//#define useCache
pboBank::compressionCache::compressionCache() {}
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

boost::shared_ptr<boost::iostreams::filtering_istream> pboBank::compressionCache::getCompressedFileBuffer(boost::shared_ptr<file> pFile) {
	auto stream = boost::make_shared<boost::iostreams::filtering_istream>();
#ifdef useCache
	std::map<boost::shared_ptr<file>, boost::shared_ptr<cacheInfo>>::iterator found;
	if (pFile->fileSize > maxCachingFileSize || (found = cachedFiles.find(pFile)) == cachedFiles.end()) {
		printf("compression cache unknown file\n");
#endif		
		boost::shared_ptr<boost::filesystem::ifstream> inStream = boost::make_shared<boost::filesystem::ifstream>();
		std::string filePath = GLOBAL.getFileManager()->getServerPathFromMD5(pFile->md5sum) + pFile->getFilename();
		inStream->open(filePath, std::ios_base::binary);


		
		stream->push(basic_refKeeper<boost::filesystem::ifstream>(inStream));
#ifdef 	useCache
		//printf("%d < %d %d \n", pFile->fileSize, maxCachingFileSize, pFile->fileSize < maxCachingFileSize);
		if (pFile->fileSize < maxCachingFileSize) {
			printf("caching file %s of size %u\n", pFile->getFilename().c_str(), pFile->fileSize);
			auto cInfo = boost::make_shared<cacheInfo>();
			cInfo->lastUse = boost::posix_time::microsec_clock::local_time();
			cInfo->buf.reserve(pFile->fileSize);

			stream->push(pboBank::copyToBuf(&cInfo->buf));
			cachedFiles[pFile] = cInfo;	//#TODO function addFileToCache(shared_ptr cinfo) and removeFileFromCache which take care about cacheSize
			cacheSize += pFile->fileSize;
		}
#endif

		stream->push(boost::iostreams::zlib_compressor{ boost::iostreams::zlib::best_speed,4096 });
		stream->push(*inStream);
#ifdef useCache
	} else {
		printf("get file from cache\n");
		found->second->lastUse = boost::posix_time::microsec_clock::local_time();
		boost::shared_ptr<boost::iostreams::stream<boost::iostreams::array_source>> inStream = boost::make_shared<boost::iostreams::stream<boost::iostreams::array_source>>(found->second->buf.data(), found->second->buf.size());																   
		stream->push(basic_refKeeper<boost::iostreams::stream<boost::iostreams::array_source>>(inStream));
		stream->push(*inStream);

	}
#endif
	return stream;
	
	








}
