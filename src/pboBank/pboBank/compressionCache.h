#pragma once
#include <map>
#include <boost/container/static_vector.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include "files.h"
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/iostreams/categories.hpp>
#include "global.h"
#include <boost/filesystem/fstream.hpp>
#define maxCachingFileSize (1024*1024) * 4900 //40 MB 
#define compressionMode boost::iostreams::zlib::best_speed



namespace pboBank {


	class cachedFile {
	public:
		typedef char        char_type;
		typedef boost::iostreams::sink_tag  category;
		cachedFile() : eof(0), completlyCached(false) {

		}
		std::streamsize read(char* s, std::streamoff pos, std::streamsize maxRead) {
			if (completlyCached) {
				if (pos > eof)
					return 0;
				std::streamsize willRead = std::min((eof - pos), maxRead);
				memcpy(s, buf.data() + pos, willRead);
				return willRead;
			}
			//wait till caching is done
			while (!((eof - pos) >= maxRead || completlyCached)) {
				boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			}
			std::streamsize willRead = std::min((eof - pos), maxRead);
			memcpy(s, buf.data() + pos, willRead);
			return willRead;
		}
		std::streamsize write(const char* s, std::streamsize n) {
			if (completlyCached) //cant write to a already done file
				return 0;
			buf.insert(buf.end(), s, s + n);
			eof += n;
			return n;
		}
		void reserve(std::streamsize n) {
			if (completlyCached) //cant write to a already done file
				return;
			buf.reserve(n);
		}
		void doneWriting() {
			printf("cachedFileDone %llu\n", eof);
			buf.shrink_to_fit();
			eof = buf.size();
			completlyCached = true;
		}
		std::streamsize getEof() const {
			return eof;
		}
		void used() {  //sets lastUse
			lastUse = boost::posix_time::microsec_clock::local_time();
		}
		bool isDone() {
			return completlyCached;
		}
	private:
		std::streamsize eof;
		boost::container::vector<char> buf;
		boost::atomic<bool> completlyCached;
		boost::posix_time::ptime lastUse;
	};




	class compressedFileBase {
	public:
		virtual ~compressedFileBase() {}

		typedef char        char_type;
		typedef boost::iostreams::source_tag  category;

		virtual std::streamsize read(char* s, std::streamsize n) = 0;
		virtual bool valid() = 0;
		virtual bool fail() = 0;//boost::istream overload
		virtual bool eof() = 0;//boost::istream overload
	private:
	};
	class compressedFileCompressing : public compressedFileBase {
	public:
		compressedFileCompressing(std::string filePath, std::streamsize fileSize, boost::shared_ptr<cachedFile> _pCachedFile);
		virtual ~compressedFileCompressing() {
			if (pCachedFile)
				pCachedFile->used();
			//#TODO investigate big allocation before this
			printf("~compressedFileCompressing speed %llu mics %llu bytes %f MB/s\n", timeSpent, bytesCompressed,
				static_cast<double>(bytesCompressed) / 1024 / 1024 //MB
				/
				std::max(1.0, static_cast<double>(timeSpent) / 1000 / 1000));//seconds
		}
		std::streamsize read(char* s, std::streamsize n) override;
		bool valid() override;
		bool fail() override;
		bool eof() override;
		bool isCaching() const;
	private:
		boost::iostreams::filtering_istream m_sourceFile;
		boost::shared_ptr<cachedFile> pCachedFile;
		boost::filesystem::ifstream inStream;
		std::array<char, 1_megaByte> compressingBuffer;
		uint64_t timeSpent;
		uint64_t bytesCompressed;

	};
	class compressedFileCached : public compressedFileBase {
	public:

		explicit compressedFileCached(const boost::shared_ptr<cachedFile>& pCachedFile)
			: pCachedFile(pCachedFile), curPos(0) {
			if (pCachedFile) {
				pCachedFile->used();
				printf("compressedFileCached %llu \n", pCachedFile->getEof());
			}
		}
		virtual ~compressedFileCached() {
			if (pCachedFile)
				pCachedFile->used();
		}
		std::streamsize read(char* s, std::streamsize n) override;
		bool valid() override;
		bool fail() override;
		bool eof() override;
	private:
		boost::shared_ptr<cachedFile> pCachedFile;
		std::streamoff curPos;
	};





	class compressionCache {
	public:
		compressionCache();
		~compressionCache();


		boost::mutex access;
		boost::shared_ptr<compressedFileBase>  getCompressedFileBuffer(boost::shared_ptr<file> pFile);
		void preCache(boost::shared_ptr<file> pFile);

		void threadCompressionTakeover(boost::shared_ptr<compressedFileCompressing> pCompressor, uint64_t maxSpeedPerSec);

		std::map<boost::shared_ptr<file>, boost::shared_ptr<cachedFile>> cachedFiles;
		uint64_t cacheSize;

	};
}
