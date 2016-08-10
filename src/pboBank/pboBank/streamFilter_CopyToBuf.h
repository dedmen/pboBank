# pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/pipeline.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/aggregate.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
namespace pboBank {
		//
		// Template name: basic_counter.
		// Template parameters:
		//      Ch - The character type.
		// Description: Filter which counts lines and characters.
		//
		template<typename Ch>
		class basic_copyFilter : public boost::iostreams::aggregate_filter<Ch> {  //#TODO use symmetric filter http://www.boost.org/doc/libs/1_61_0/libs/iostreams/doc/index.html
		public:
			typedef Ch char_type;
			//struct category
			//	: boost::iostreams::dual_use,
			//	boost::iostreams::filter_tag,
			//	boost::iostreams::multichar_tag,
			//	boost::iostreams::optimally_buffered_tag {};	
			explicit basic_copyFilter(boost::container::vector<char>* buf)
				: buf_(boost::make_shared<boost::iostreams::stream<boost::iostreams::back_insert_device<boost::container::vector<char>>>>(*buf)), vecref_(buf), wrote(0){
				//printf("construct %p %p\n", this, vecref_);

			}
			std::streamsize optimal_buffer_size() const { return 4096; }
			//template<typename Source>
			//std::streamsize read(Source& src, char_type* s, std::streamsize n) {
			//	std::streamsize result = boost::iostreams::read(src, s, n);
			//	if (result == -1)
			//		return -1;
			//	//printf("read\n");
			//	buf_->write(s, result);
			//	wrote += result;
			//	if (result != 1)
			//		printf("read %d", result);
			//	return result;
			//}
			//
			//template<typename Sink>
			//std::streamsize write(Sink& snk, const char_type* s, std::streamsize n) {
			//	std::streamsize result = boost::iostreams::write(snk, s, n);
			//	printf("write\n");
			//	buf_->write(s, result);
			//	return result;
			//}
			virtual void do_filter(const vector_type& src, vector_type& dest) {
				dest.insert(dest.end(),
					src.begin(),
					src.end());
				vecref_->insert(vecref_->end(),
					src.begin(),
					src.end());
				printf("filter %d\n", src.size());
			};
			virtual void do_close() {
				//printf("do_close source\n");



				//printf("before %llu\n", vecref_->capacity());
				//printf("wrote %u\n", wrote);
				vecref_->shrink_to_fit();
				//printf("after %llu\n", vecref_->capacity());

			}


			//template<typename Source>
			//void close(Source&) {
			//	printf("close source\n");
			//	vecref_->shrink_to_fit();
			//}

		private:
			boost::shared_ptr<boost::iostreams::stream<boost::iostreams::back_insert_device<boost::container::vector<char>>>> buf_;
			boost::container::vector<char>* vecref_;
			uint32_t wrote;
		};
		BOOST_IOSTREAMS_PIPABLE(basic_copyFilter, 1)

		typedef basic_copyFilter<char>     copyToBuf;





		template<typename Ch>
		class basic_refKeeper {
		public:
			typedef Ch ref_type;
			typedef char char_type;
			struct category
				: boost::iostreams::dual_use,
				boost::iostreams::filter_tag,
				boost::iostreams::multichar_tag,
				boost::iostreams::optimally_buffered_tag {};
			explicit basic_refKeeper(boost::shared_ptr<ref_type> ref)
				: ref_(ref) {}
			std::streamsize optimal_buffer_size() const { return 0; }

			template<typename Source>
			std::streamsize read(Source& src, char_type* s, std::streamsize n) {
				std::streamsize result = boost::iostreams::read(src, s, n);
				//printf("read %d\n", result);
				return result;
			}

			template<typename Sink>
			std::streamsize write(Sink& snk, const char_type* s, std::streamsize n) {
				std::streamsize result = boost::iostreams::write(snk, s, n);
				return result;
			}
		private:
			boost::shared_ptr<ref_type> ref_;
		};
		BOOST_IOSTREAMS_PIPABLE(basic_refKeeper, 1)

} // End namespaces iostreams, boost.