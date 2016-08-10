#include "global.h"
#include "fileManager.h"
#include "database.h"
#include "modManager.h"
#include "fileServer.h"
#include "compressionCache.h"
#include <boost/thread.hpp>
global GLOBAL;

global::global() {
	fileBankBasePath = "T:/bank/";
}
//#Filetransfer http://stackoverflow.com/questions/6486745/c-lzma-compression-and-decompression-of-large-stream-by-parts

global::~global() {}

void global::init() {
	pDatabase = new pboBank::database();
	printf("connecting to DB\n");
	pDatabase->connect();
	printf("getting Mods from DB\n");
	auto modsInit = pDatabase->getMods();
	std::sort(modsInit.begin(), modsInit.end(), pboBank::mod::isLessThan);
	printf("getting ChangeSets from DB\n");
	auto changeSetsInit = pDatabase->getChangeSets();
	printf("getting Files from DB\n");
	auto filesInit = pDatabase->getFiles(changeSetsInit, modsInit);
	printf("Loaded %llu files, %llu mods and %llu changesets\n", filesInit.size(), modsInit.size(), changeSetsInit.size());
	pFileManager = new pboBank::fileManager(filesInit);
	pModManager = new pboBank::modManager(modsInit);
	pFileServer = new pboBank::fileServer(mainIOservice);
	pCompressionCache = new pboBank::compressionCache();
	pboBank::fileManager::getServerPathFromMD5(pboBank::fileManager::makeMD5Hash("T:/README")); //#testcase
	//printf("Current bank size: %llu MB\n", pFileManager->getCurrentBankSize() / (1024 * 1024));
	printf("Available Mods: \n");

	for (auto &it : modsInit) {
		//std::cout << "\tMod:" << std::setw(64) << it->name.c_str() << "\tVersion:"<< std::setw(32) << it->version.c_str() << "\tFiles: " << it->files.size() << std::endl;



		printf("\tMod: %-32s\tVersion: %-29s\tfiles: %-5llu\n", it->name.c_str(), it->version.c_str(), it->files.size());
	}




	//pModManager->indexMod("Mission Control Center Sandbox", "MCC Sandbox 3 is a dynamic mission creating tool for Arma 3 (SP/MP)",
	//	"http://www.armaholic.com/page.php?id=19580", "r18");
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/mcc18/@mcc_sandbox/addons/", pModManager->findModByNameAndVersion("Mission Control Center Sandbox", "r18"));
	//
	//pModManager->indexMod("ACE", "ACE3 is a joint effort by the teams behind ACE2, AGM and CSE to improve the realism and authenticity of Arma 3.",
	//	"https://github.com/acemod/ACE3", "3.6.2");
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/ace362/@ace/addons/", pModManager->findModByNameAndVersion("ACE", "3.6.2"));
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/ace362/@ace/optionals/", pModManager->findModByNameAndVersion("ACE", "3.6.2"));
	//
	//pModManager->indexMod("ACE", "ACE3 is a joint effort by the teams behind ACE2, AGM and CSE to improve the realism and authenticity of Arma 3.",
	//	"https://github.com/acemod/ACE3", "3.6.1");
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/ace361/@ace/addons/", pModManager->findModByNameAndVersion("ACE", "3.6.1"));
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/ace361/@ace/optionals/", pModManager->findModByNameAndVersion("ACE", "3.6.1"));
	//
	//pModManager->indexMod("CBA_A3", "Community Base Addons",
	//	"https://github.com/CBATeam/CBA_A3", "3.0.0.160713");
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/cba300/@CBA_A3/addons", pModManager->findModByNameAndVersion("CBA_A3", "3.0.0.160713"));
	//
	//pModManager->indexMod("CBA_A3", "Community Base Addons",
	//	"https://github.com/CBATeam/CBA_A3", "2.5.0.160711");
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/cba250/@CBA_A3/addons", pModManager->findModByNameAndVersion("CBA_A3", "2.5.0.160711"));
	//
	//pModManager->indexMod("ALiVE", "ALIVE is the next generation dynamic persistent mission addon for ArmA3. Developed by Arma community veterans, the easy to use modular mission framework provides everything that players and mission makers need to set up and run realistic military operations in almost any scenario up to Company level, including command, combat support, service support and logistics.",
	//	"http://alivemod.com/#Download", "1.1.1.1607191");
	//
	//pFileManager->indexDirectory("F:/Steam/SteamApps/common/Arma 3/pboBanktests/@ALiVE/addons", pModManager->findModByNameAndVersion("ALiVE", "1.1.1.1607191"));
	//pFileManager->indexFile("F:/Steam/SteamApps/common/Arma 3/pboBanktests/@ALiVE/ALiVEClient.dll", pModManager->findModByNameAndVersion("ALiVE", "1.1.1.1607191"));

	//for (int i = 0; i < 128; ++i) {
	//	pFileManager->dropFileLinks(pModManager->findModByNameAndVersion("ACE", "3.6.2")->files, "T:/acetest/" + boost::lexical_cast<std::string>(i) + "/");
	//	pFileManager->dropFileLinks(pModManager->findModByNameAndVersion("ALiVE", "1.1.1.1607191")->files, "T:/acetest/" + boost::lexical_cast<std::string>(i) + "/");
	//	pFileManager->dropFileLinks(pModManager->findModByNameAndVersion("CBA_A3", "3.0.0.160713")->files, "T:/acetest/" + boost::lexical_cast<std::string>(i) + "/");
	//	pFileManager->dropFileLinks(pModManager->findModByNameAndVersion("Mission Control Center Sandbox", "r18")->files, "T:/acetest/" + boost::lexical_cast<std::string>(i) + "/");
	//	printf("%d\n", i);
	//}

	pModManager->indexMod("test", "test",
		"test", "1");

	pFileManager->indexFile("F:/Steam/SteamApps/common/Arma 3/@GF_Vehicles/addons/gr_medium_utility_helicopters.pbo", pModManager->findModByNameAndVersion("test", "1"));




	//pFileManager->compressFileInBank(pModManager->findModByNameAndVersion("test", "1")->files.front());
	//pFileManager->uncompressFileInBank(pModManager->findModByNameAndVersion("test", "1")->files.front());
	boost::posix_time::ptime t0(boost::posix_time::microsec_clock::local_time());
	//pModManager->uncompressMod(pModManager->findModByNameAndVersion("ACE", "3.6.1"));
	boost::posix_time::ptime t1(boost::posix_time::microsec_clock::local_time());
	//pModManager->compressMod(pModManager->findModByNameAndVersion("ACE", "3.6.1"));
	boost::posix_time::ptime t2(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration diff = t2 - t1;
	boost::posix_time::time_duration diffunc = t1 - t0;
	printf("uncompress %lld", diffunc.total_milliseconds());
	printf("compress %lld", diff.total_milliseconds());

}

void global::run() {
	boost::asio::io_service::work wurk(mainIOservice);
	mainIOservice.run();
	boost::thread t([this]() {
		mainIOservice.run();
		printf("iostream_exit1\n");
	});
	boost::thread t2([this]() {
		mainIOservice.run();
		printf("iostream_exit1\n");
	});
	boost::thread t3([this]() {
		mainIOservice.run();
		printf("iostream_exit1\n");
	});
	boost::thread t4([this]() {
		mainIOservice.run();
		printf("iostream_exit1\n");
	});




}

void global::signal(int signum) {
	printf("signal %d", signum);   //2 is ctrl+C SIGTERM
}
