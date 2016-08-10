#include "files.h"


void pboBank::file::addMod(boost::shared_ptr<mod> pMod) {
	pMods.emplace_back(pMod);
	std::sort(pMods.begin(), pMods.end(),mod::isLessThan);
	pMods.erase(std::unique(pMods.begin(), pMods.end()), pMods.end());
}
