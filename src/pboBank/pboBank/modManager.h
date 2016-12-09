#pragma once
#include "files.h"
namespace pboBank {


	class modManager {
	public:
		explicit modManager(std::vector<boost::shared_ptr<mod>> modsInit);
		~modManager();
		boost::shared_ptr<mod> findModByNameAndVersion(std::string name,std::string version);


		void indexMod(std::string name, std::string description, std::string download,std::string version);
		std::vector<boost::shared_ptr<mod>> mods;

		static void compressMod(boost::shared_ptr<mod> pMod);
		static void uncompressMod(boost::shared_ptr<mod> pMod);
	};



}
