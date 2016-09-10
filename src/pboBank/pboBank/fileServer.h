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
	enum class fileServerState {
		sendingFile,
		commandMode
	};
	class fileServerClient
		: public boost::enable_shared_from_this<fileServerClient> {
	public:
		static boost::shared_ptr<fileServerClient> create(boost::asio::io_service& io_service) {
			return 	boost::make_shared<fileServerClient>(io_service);
		}

		boost::asio::ip::tcp::socket& socket() {
			return socket_;
		}
		void testTransfer();
		void start() {


			socket_.set_option(boost::asio::ip::tcp::no_delay(true));
			socket_.async_read_some(boost::asio::buffer(readBuf.data(), readBuf.size()),
				boost::bind(&fileServerClient::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			isReading = true;










			
		}
		void sendFile();

		explicit fileServerClient(boost::asio::io_service& io_service)
			: state(fileServerState::commandMode), m_sourceFile(nullptr), socket_(io_service), counter(0), isReading(false){}
	private:


		void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);
		void handle_read(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);
		fileServerState state;
		boost::posix_time::ptime transferstart;
		boost::shared_ptr<compressedFileBase> m_sourceFile; //this really doesnt need to be a ptr... but its easy to switch to compressionCache that way

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

		std::array<char, (1024 * 1024) * 1> m_buf;	  //keeping this at 1MB or lower for now.. because thats capable of pushing 100MB/s though we only need max 12
		std::array<char, (1024) * 8> readBuf;
		boost::asio::ip::tcp::socket socket_;
		std::string message_;
		uint64_t counter;
		bool isReading;
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
					printf("conn END\n");
			});
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
