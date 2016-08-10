#include "modManager.h"
#include <boost/make_shared.hpp>
#include "global.h"
#include "database.h"
#include "fileManager.h"
pboBank::modManager::modManager(std::vector<boost::shared_ptr<mod>> modsInit) : mods(modsInit) {

}


pboBank::modManager::~modManager() {}

boost::shared_ptr<pboBank::mod> pboBank::modManager::findModByNameAndVersion(std::string name, std::string version) {
	for (auto& it : mods) {
		if (it->name.compare(name) == 0 && it->version.compare(version) == 0)
			return it;
	}
	return nullptr;
}

void pboBank::modManager::indexMod(std::string name, std::string description, std::string download, std::string version) {
	if (findModByNameAndVersion(name, version)) {
		printf("ERROR mod \"%s\" already exists\n", name.c_str());
		return;
	}
	auto modPtr = boost::make_shared<mod>();
	modPtr->name = name;
	modPtr->description = description;
	modPtr->download = download;
	modPtr->version = version;
	if (GLOBAL.getDatabase()->insertMod(modPtr)) {
		mods.emplace_back(modPtr);
	}
	//#TODO indexMod should return ptr to new mod
}

void pboBank::modManager::compressMod(boost::shared_ptr<mod> pMod) {
	for (auto &it : pMod->files) {
		GLOBAL.getFileManager()->compressFileInBank(it);
	}
}

void pboBank::modManager::uncompressMod(boost::shared_ptr<mod> pMod) {
	for (auto &it : pMod->files) {
		GLOBAL.getFileManager()->uncompressFileInBank(it);
	}
}
