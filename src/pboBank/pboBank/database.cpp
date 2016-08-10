#include "database.h"
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/make_shared.hpp>

pboBank::database::database() {

	pMysqlCon = mysql_init(nullptr);
	mysql_options(pMysqlCon, MYSQL_OPT_COMPRESS, nullptr);
	uint32_t timeout = 30;
	mysql_options(pMysqlCon, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
	my_bool reconnect = true;
	mysql_options(pMysqlCon, MYSQL_OPT_RECONNECT, &reconnect);
}

pboBank::database::~database() {}

bool pboBank::database::connect() {

	if (mysql_real_connect(pMysqlCon, "bravo.dedmen.de", "pboBank", "pboBank", "pboBank", 0, nullptr, CLIENT_MULTI_RESULTS | CLIENT_COMPRESS) == nullptr) {
		printf("db connection failed %s\n", mysql_error(pMysqlCon));
		mysql_close(pMysqlCon);//#TODO report error
		return false;
	}
	printf("db connection success\n");
	return true;
}

std::vector<boost::shared_ptr<pboBank::changeSet>> pboBank::database::getChangeSets() {
	std::vector<boost::shared_ptr<changeSet>> changeSets;
	return changeSets;//#Changesets
	if (mysql_query(pMysqlCon,
		"SELECT"
		"changesets.`index` AS `index`,"
		"changesets.description AS description,"
		"changesets.changelog AS changelog,"
		"UNIX_TIMESTAMP(changesets.creationDate) AS creationDate,"
		"UNIX_TIMESTAMP(changesets.lastEditDate) AS lastEditDate"
		"FROM"
		"(changesets)"
	)) {
		return std::vector<boost::shared_ptr<changeSet>>();
	}

	MYSQL_RES *result = mysql_store_result(pMysqlCon);

	if (result == NULL) {
		return std::vector<boost::shared_ptr<changeSet>>();
	}


	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result))) {
		uint32_t index = boost::lexical_cast<uint32_t>(row[0]);
		std::string description(row[1]);
		std::string changelog(row[2]);
		boost::posix_time::ptime creationDate = boost::posix_time::from_time_t(boost::lexical_cast<uint32_t>(row[3]));
		boost::posix_time::ptime lastEditDate = boost::posix_time::from_time_t(boost::lexical_cast<uint32_t>(row[4]));
		changeSets.emplace_back(boost::make_shared<changeSet>(index, description, changelog, creationDate, lastEditDate));
	}

	mysql_free_result(result);


	if (mysql_query(pMysqlCon,
		"SELECT"
		"changes.changeset AS changeset,"
		"changes.changeType+0 AS changeType,"
		"changes.changeFilepath` AS changeFilepath"
		"FROM"
		"`changes`"
	)) {
		return std::vector<boost::shared_ptr<changeSet>>();
	}

	auto findChangeSetByIndex = [&](int index) -> boost::shared_ptr<changeSet> {
		for (auto &it : changeSets) {
			if (it->index == index)
				return it;
		}
		return nullptr;
	};



	result = mysql_store_result(pMysqlCon);

	if (result == NULL) {
		return std::vector<boost::shared_ptr<changeSet>>();
	}

	while ((row = mysql_fetch_row(result))) {
		uint32_t changeSetIndex = boost::lexical_cast<uint32_t>(row[0]);
		changeType type = static_cast<changeType>(boost::lexical_cast<uint32_t>(row[1]));
		std::string changeFilepath(row[2]);

		auto changeSet = findChangeSetByIndex(changeSetIndex);
		if (changeSet)
			changeSet->addChange(boost::make_shared<change>(changeSet, type, changeFilepath));
	}
	mysql_free_result(result);
	return changeSets;
}

std::vector<boost::shared_ptr<pboBank::file>> pboBank::database::getFiles(std::vector<boost::shared_ptr<pboBank::changeSet>> changeSetsInit, std::vector<boost::shared_ptr<pboBank::mod>> modsInit) {
	std::vector<boost::shared_ptr<pboBank::file>> files;

	if (mysql_query(pMysqlCon,
		"SELECT\n"
		"	`files`.`index` AS `index`,\n"
		"	`files`.`fileName` AS `fileName`,\n"
		"	`files`.`fileSize` AS `fileSize`,\n"
		"	`files`.`change` AS `change`,\n"
		"	`files`.`hash` AS `hash`\n"
		"FROM\n"
		"	`files`"
	)) {
		return files;  //returns empty vector
	}

	MYSQL_RES *result = mysql_store_result(pMysqlCon);
	if (!result)
		return files;  //returns empty vector

	auto getModByIndex = [&](uint32_t index) -> boost::shared_ptr<pboBank::mod> {
		for (auto &it : modsInit) {
			if (it->index == index)
				return it;
		}
		printf("ERROR mod with index %d not found\n", index);
		return nullptr;
	};
	auto getChangeByIndex = [&](uint32_t index) -> boost::shared_ptr<pboBank::change> {
		for (auto &it : changeSetsInit) {
			for (auto &iterator : it->changes)
				if (iterator->index == index)
					return iterator;
		}
		printf("ERROR change with index %d not found\n", index);
		return nullptr;
	};
	auto getFileByIndex = [&](uint32_t index) -> boost::shared_ptr<pboBank::file> {
		for (auto &it : files) {
			if (it->index == index)
				return it;
		}
		printf("ERROR file with index %d not found\n", index);
		return nullptr;
	};



	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint32_t fileIndex = boost::lexical_cast<uint32_t>(row[0]);
		std::string fileName = row[1];
		uint32_t fileSize = boost::lexical_cast<uint32_t>(row[2]);
		uint32_t changeIndex = boost::lexical_cast<uint32_t>(row[3]);
		std::string modHashString = row[4];	modHashString.insert(0, "0x");
		boost::multiprecision::uint128_t fileHash(modHashString);

		auto filePtr = boost::make_shared<file>();
		filePtr->index = fileIndex;
		filePtr->setFilename(fileName);
		filePtr->fileSize = fileSize;
		filePtr->md5sum = fileHash;
		//filePtr->pChange = getChangeByIndex(changeIndex);		  //#Changesets
		files.emplace_back(filePtr);
	}
	mysql_free_result(result);


	if (mysql_query(pMysqlCon,
		"SELECT\n"
		"	files_mods.fileIndex,\n"
		"	files_mods.modIndex\n"
		"FROM\n"
		"	files_mods"
	)) {
		return std::vector<boost::shared_ptr<pboBank::file>>();  //returns empty vector
	}

	result = mysql_store_result(pMysqlCon);
	if (!result)
		return std::vector<boost::shared_ptr<pboBank::file>>();  //returns empty vector

	while ((row = mysql_fetch_row(result))) {
		auto file = getFileByIndex(boost::lexical_cast<uint32_t>(row[0]));
		auto mod = getModByIndex(boost::lexical_cast<uint32_t>(row[1]));
		file->addMod(mod);
		mod->files.emplace_back(file);

	}
	mysql_free_result(result);
	return files;
}

std::vector<boost::shared_ptr<pboBank::mod>> pboBank::database::getMods() const {
	std::vector<boost::shared_ptr<mod>> mods;

	if (mysql_query(pMysqlCon,
		"SELECT\n"
		"	`mods`.`index` AS `index`,\n"
		"	`mods`.`name` AS `name`,\n"
		"	`mods`.`version` AS `version`,\n"
		"	`mods`.`description` AS `description`,\n"
		"	`mods`.`download` AS `download`,\n"
		"	UNIX_TIMESTAMP(`mods`.`creationDate`) AS `creationDate`\n"
		"FROM\n"
		"	`mods`"
	)) {
		return mods;  //returns empty vector
	}

	MYSQL_RES *result = mysql_store_result(pMysqlCon);
	if (!result)
		return mods;  //returns empty vector

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		uint32_t modIndex = boost::lexical_cast<uint32_t>(row[0]);
		std::string modName = row[1];
		std::string modVersion = row[2];
		std::string modDescription = row[3];
		std::string modDownload = row[4];
		boost::posix_time::ptime modCreationDate = boost::posix_time::from_time_t(boost::lexical_cast<uint32_t>(row[5]));
		auto modPtr = boost::make_shared<mod>();
		modPtr->index = modIndex;
		modPtr->name = modName;
		modPtr->version = modVersion;
		modPtr->description = modDescription;
		modPtr->download = modDownload;
		modPtr->creationDate = modCreationDate;
		mods.emplace_back(modPtr);
	}
	mysql_free_result(result);
	return mods;
}

bool pboBank::database::insertMod(boost::shared_ptr<mod> pMod) const {
	printf("insertMod\n");

	if (mysql_query(pMysqlCon,
		("CALL createMod('" + pMod->name + "','" + pMod->description + "','" + pMod->download + "','" + pMod->version + "');").c_str()//#TODO make base64 encoded transmission
	)) {

		//#TODO handle error... maybe its a msg to user
		printf("createMod error %s\n", mysql_error(pMysqlCon));
		return false;
	}

	auto result = mysql_store_result(pMysqlCon);
	auto row = mysql_fetch_row(result);
	pMod->index = boost::lexical_cast<uint32_t>(row[0]);
	mysql_free_result(result);
	printf("insertMods results %d\n", mysql_next_result(pMysqlCon));
	return true;
}

bool pboBank::database::insertFile(boost::shared_ptr<file> pFile, boost::shared_ptr<pboBank::mod> pMod) const {
	printf("insertFile\n");
	std::stringstream ss;			   //#TODO test
	ss << std::setfill('0') << std::setw(32) << std::hex << pFile->md5sum;
	std::string md5string = ss.str();
	if (!pFile || !pMod) {
		printf("CRITICAL tried to insert file that is invalid ptr or doesnt have a Mod reference\n");
		return false;
	}
	if (mysql_query(pMysqlCon,
		("CALL insertFile('" + pFile->getFilename() + "'," + boost::lexical_cast<std::string>(pFile->fileSize) + ",'" + md5string + "'," + boost::lexical_cast<std::string>(pMod->index) + ");").c_str()//#TODO make base64 encoded transmission
	)) {


		//#TODO handle error... maybe its a msg to user
		printf("insertFile error %s\n", mysql_error(pMysqlCon));


		return false;
	}
	auto result = mysql_store_result(pMysqlCon);
	auto row = mysql_fetch_row(result);
	pFile->index = boost::lexical_cast<uint32_t>(row[0]);
	mysql_free_result(result);
	printf("insertMods results %d\n", mysql_next_result(pMysqlCon));
	return true;

}
