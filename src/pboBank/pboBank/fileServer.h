#pragma once
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/filesystem/fstream.hpp>
#include "global.h"
#include "fileManager.h"
#include "files.h"
#include <boost/thread/thread.hpp>
#include "compressionCache.h"
#include "modManager.h"
/*
to test this use
nc 192.168.137.1 1337 > /dev/null
on Linux
*/
#define fileServerMultiThread
namespace pboBank {
	class fileServerClient
		: public boost::enable_shared_from_this<fileServerClient> {
	public:
		static boost::shared_ptr<fileServerClient> create(boost::asio::io_service& io_service) {
			return 	boost::make_shared<fileServerClient>(io_service);
		}

		boost::asio::ip::tcp::socket& socket() {
			return socket_;
		}

		void start() {

			//This will be called outside of mainthread!

			printf("startTransfer \n");

#ifdef _WIN32_WINNT
			auto pfile = GLOBAL.getModManager()->findModByNameAndVersion("test", "1")->files;
#else
			auto pfile = GLOBAL.getModManager()->findModByNameAndVersion("linuxtext", "1")->files;
#endif

			//auto pfile = GLOBAL.getFileManager()->getFiles();

			auto getFileByName = [&](std::string name) ->boost::shared_ptr<file> {
				for (auto& it : pfile) {
					if (it->getFilename().compare(name) == 0)
						return it;
				}
				return nullptr;
			};




			auto pFile = getFileByName("gr_medium_utility_helicopters.pbo");
			//auto pFile = getFileByName("mcc_sandbox_mod.pbo");
			m_sourceFile = GLOBAL.getCompressionCache()->getCompressedFileBuffer(pFile);
			printf("input %u\n", pFile->fileSize);
			transferstart = boost::posix_time::microsec_clock::local_time();

			/*
			2MB buffer
			93MB/s	 on 9 connections
			10MB buffer
			101MB/s
			4MB buffer
			104MB/s
			1MB buffer
			102MB/s

			*/
			sendFile();
		}
		void sendFile() {
#ifdef fileServerMultiThread
			while
#else
			if
#endif

				(m_sourceFile && m_sourceFile->valid() && !m_sourceFile->eof()) {
				auto dataRead = m_sourceFile->read(m_buf.data(), m_buf.size());
				if (m_sourceFile->fail() && !m_sourceFile->eof()) {
					//auto msg = "Failed while reading file";
					//BOOST_LOG_TRIVIAL(error) << msg;
					//throw std::fstream::failure(msg);
					printf("CRITICAL sendFile error\n");
					return;
				}

				auto buf = boost::asio::buffer(m_buf.data(), static_cast<size_t>(dataRead));
				auto xthis = this->shared_from_this();
#ifdef fileServerMultiThread
				boost::system::error_code ec;
				xthis->counter += boost::asio::write(socket_, buf, ec);
				if (ec) {
					auto compressor = boost::dynamic_pointer_cast<compressedFileCompressing>(m_sourceFile);
					if (!compressor || !compressor->isCaching())
						break;
					GLOBAL.getCompressionCache()->threadCompressionTakeover(compressor, 7_megaByte);  //continue compressing file to cache
					break;
				}
#else
				boost::asio::async_write(socket_,
					buf,
					[xthis](boost::system::error_code ec, size_t length) {
					xthis->counter += length;
					xthis->sendFile();
				});
				return;
#endif
			}
			boost::posix_time::ptime t2(boost::posix_time::microsec_clock::local_time());
			boost::posix_time::time_duration diff = t2 - transferstart;
			printf("transmission done %llu in %lld with %llu KB/s\n", counter, diff.total_milliseconds(), ((counter / std::max(static_cast<int64_t>(1), diff.total_milliseconds())) * 1000) / 1024);

			//file done






		}

		explicit fileServerClient(boost::asio::io_service& io_service)
			: m_sourceFile(nullptr), socket_(io_service), counter(0) {}
	private:


		void handle_write(const boost::system::error_code& /*error*/,
			size_t /*bytes_transferred*/) {}

		boost::posix_time::ptime transferstart;
		boost::shared_ptr<compressedFileBase> m_sourceFile; //this really doesnt need to be a ptr... but its easy to switch to compressionCache that way
		std::array<char, (1024 * 1024) * 1> m_buf;	  //keeping this at 1MB or lower for now.. because thats capable of pushing 100MB/s though we only need max 12
		boost::asio::ip::tcp::socket socket_;
		std::string message_;
		uint64_t counter;
	};

	class fileServer {
	public:
		explicit fileServer(boost::asio::io_service& io_service) : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1337)) {
			start_accept();
		}
	private:
		void start_accept() {
			boost::shared_ptr<fileServerClient> new_connection =
				fileServerClient::create(acceptor_.get_io_service());

			acceptor_.async_accept(new_connection->socket(),
				boost::bind(&fileServer::handle_accept, this, new_connection,
					boost::asio::placeholders::error));
		}

		void handle_accept(boost::shared_ptr<fileServerClient> new_connection,
			const boost::system::error_code& error) {
			if (!error) {
				new_connection->socket().set_option(boost::asio::ip::tcp::no_delay(true));
#ifdef fileServerMultiThread
				boost::thread t([new_connection]() {
					new_connection->start();
					printf("conn END\n"); });
#else
				new_connection->start();
#endif

				//single thread can push about 15MB/s for 9 connections on AMD FX 8350
				//We only need 12MB/s max so one Thread does suffice



			}

			start_accept();
		}

		boost::asio::ip::tcp::acceptor acceptor_;
	};

}
