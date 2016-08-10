#pragma once
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/date_time.hpp>
namespace pboBank {

	enum class changeType {
		add,
		remove,
		move,
		deactivate
	};


	class changeSet;
	class change {

	public:
		change(boost::shared_ptr<changeSet> pChangeSet, changeType type, std::string filePath) : pChangeSet(pChangeSet), type(type), filePath(filePath) {}
		boost::shared_ptr<changeSet> getChangeSet() const { return pChangeSet; }
		changeType getChangeType() const { return type; }
		std::string getFilePath() const { return filePath; }
	//private:  //#TODO make stuff private
		uint32_t index;//index in database
		boost::shared_ptr<changeSet> pChangeSet;
		changeType type;
		std::string filePath;
	};

	class changeSet {
	public:
		changeSet(int index,std::string description, std::string changelog, boost::posix_time::ptime creationDate, boost::posix_time::ptime lastEditDate)
			: index(index),description(description), changelog(changelog), creationDate(creationDate), lastEditDate(lastEditDate) {}
		void addChange(boost::shared_ptr<change> pChange) {
			changes.emplace_back(pChange);
		}
		//#TODO make stuff private
		uint32_t index;//index in database
		std::vector<boost::shared_ptr<change>> changes;
		std::string description;
		std::string changelog;
		boost::posix_time::ptime creationDate;
		boost::posix_time::ptime lastEditDate;
	};
}