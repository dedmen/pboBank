#pragma once
#include <mysql.h>
#include <vector>
#include "changeSet.h"
#include "files.h"
#include <boost/multiprecision/cpp_int.hpp>
namespace pboBank {

	struct trunkData {
		boost::multiprecision::uint128_t md5sum;
		std::string filePath;
		std::string modpack;
	};

	class database {
	public:
		database();

		~database();
		bool connect();

		std::vector<boost::shared_ptr<changeSet>> getChangeSets();
		std::vector<boost::shared_ptr<file>> getFiles(std::vector<boost::shared_ptr<pboBank::changeSet>> changeSetsInit, std::vector<boost::shared_ptr<pboBank::mod>> modsInit);
		std::vector<boost::shared_ptr<mod>> getMods() const;
		std::vector<trunkData> getTrunk();
		bool insertMod(boost::shared_ptr<mod>) const;  //returns success
		bool insertFile(boost::shared_ptr<file> pFile, boost::shared_ptr<pboBank::mod> pMod) const;


	private:
		MYSQL * pMysqlCon;

	};
}
