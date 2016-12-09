#pragma once
#include <boost/asio/ip/address.hpp>
#include <chrono>
namespace pboBank {

    enum class clusterNodeType {
        master,
        slave
    };


    struct clusterInfo {
        boost::asio::ip::address address;
        clusterNodeType type;
        float load;//last reported Network load on that node between 0-1
        //someUUID the current state of that node. if different then nodes are OUT of sync



        std::chrono::system_clock::time_point lastUpdate;



    };












    class cluster {
    public:
        cluster();
        ~cluster();
    };


}