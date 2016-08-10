#pragma once
#include <boost/asio/streambuf.hpp>
#include <map>
#include <boost/container/static_vector.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include "files.h"
#include <boost/smart_ptr/make_shared_object.hpp>
#define maxCachingFileSize (1000*1000) * 4900 //40 MB
namespace pboBank {
	struct cacheInfo {
		boost::container::vector<char> buf;
		boost::posix_time::ptime lastUse;
	};


	class compressionCache {
	public:
		compressionCache();
		~compressionCache();



		boost::shared_ptr<boost::iostreams::filtering_istream>  getCompressedFileBuffer(boost::shared_ptr<file> pFile);
		std::map<boost::shared_ptr<file>, boost::shared_ptr<cacheInfo>> cachedFiles;
		uint64_t cacheSize;

	};
}
