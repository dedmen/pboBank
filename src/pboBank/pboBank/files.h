#pragma once
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <map>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/flyweight.hpp>
#include <boost/flyweight/no_tracking.hpp>
#include <boost/container/string.hpp>
class change;
namespace pboBank {

	class mod;
	class file {
	public:
		boost::shared_ptr<mod> getMod() const { return pMods.back(); }
		std::vector<boost::shared_ptr<mod>> getMods() const { return pMods; }

		void addMod(boost::shared_ptr<mod> pMod);
		uint32_t index; //index in database
		
		boost::multiprecision::uint128_t md5sum;
		uint32_t fileSize;
		//boost::shared_ptr<change> pChange;//Change this file was last changed in	 //#Changesets
		std::string getFilename() const { return std::string(fileName.data(), fileName.size()); }
		void setFilename(std::string val) { fileName = (val.c_str());/* fileName(std::move(val)); */}
	private:
		struct file_type{};
		//boost::flyweight<
			boost::container::string
		//	,boost::flyweights::tag<file_type>,boost::flyweights::no_tracking>	
			fileName;
		std::vector<boost::shared_ptr<mod>> pMods;//Mods this file belongs to
	};


	class mod {
	public:
		uint32_t index; //index in database
		std::string name;	  //#TODO convert to boost::container::string after adding getters/setters
		std::string description;
		std::string download;
		std::string version;
		std::vector<boost::shared_ptr<file>> files;//files of this mod
		boost::posix_time::ptime creationDate;
		static bool isLessThan(const boost::shared_ptr<pboBank::mod>& struct1, const boost::shared_ptr<pboBank::mod>& struct2) {
			if (struct1->name.compare(struct2->name) == 0)
				return struct1->version < struct2->version;
			return struct1->name < struct2->name;
		}
	};

}

