#include "fileServer.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/scope_exit.hpp>
void pboBank::fileServerClient::sendFile() {
	//This will be called outside of mainthread!
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
	state = fileServerState::commandMode;
	//file done
}

void pboBank::fileServerClient::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
		switch (state)
		{
			case fileServerState::sendingFile:
				sendFile();
				counter += bytes_transferred;
				break;
			case fileServerState::commandMode:
				break;

		}
}

void pboBank::fileServerClient::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
	if (error)
		return;
	BOOST_SCOPE_EXIT_ALL(&) {
		socket_.async_read_some(boost::asio::buffer(readBuf.data(), readBuf.size()),
			boost::bind(&fileServerClient::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		isReading = true;
	};

   if (state == fileServerState::sendingFile) {
	   return;
   }
   std::string data(readBuf.data(), bytes_transferred);
   if (data.length() > 3 && !boost::algorithm::starts_with(data, "{")) {

	   if(boost::algorithm::starts_with(data, "recv")) {
		   testTransfer();
	   }
	   return;
   }
   std::stringstream ss;
   ss << data;

   boost::property_tree::ptree pt;
   boost::property_tree::read_json(ss, pt);

   //print(pt);
   //printf("endPrint\n");



   std::string command = pt.get<std::string>("command", "invalid");

   if (!command.compare("")) {
	   
   }
   //#ifdef fileServerMultiThread
   //   boost::thread t([new_connection]() {
   //	   new_connection->start();
   //	   printf("conn END\n"); });
   //#else
   //   new_connection->start();
   //#endif






   
}

void pboBank::fileServerClient::testTransfer() {
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

#ifdef fileServerMultiThread
	boost::thread t([this]() {
		sendFile();
		printf("transfer END\n"); });
#else
	sendFile();
#endif
	















}
