#include "SimpleMem.h"

uint32_t SimpleMem::Request::id_seed=0;

// Constructor
SimpleMem::SimpleMem() {
    std::cout << "[Functional Memory] SimpleMem initialized." << std::endl;
}

void SimpleMem::init(int phase) {
}

void SimpleMem::sendInitData(Request* req) {
    if (!req) {
        std::cerr << "[Error] sendInitData called with null request!" << std::endl;
        return;
    }
    sendRequest(req);
}

void SimpleMem::sendRequest(Request* req) {
    if (!req) {
        std::cerr << "[Error] processRequest called with null request!" << std::endl;
        return;
    }

    uint64_t addr = req->getAddress();
    switch (req->getcmd()) {
        case Request::Command::Read:
            std::cout << "[Read] Processing request at address " << req->getAddress() << std::endl;
            for (uint64_t offset=0; offset<req->getSize(); offset++) {
                if (dataArray.find(addr+offset) == dataArray.end()) {
                    dataArray[addr+offset] = 0;
                }
                req->getData().at(offset) = dataArray[addr+offset];
            }
            break;
        case Request::Command::Write:
            std::cout << "[Write] Processing request at address " << req->getAddress() << std::endl;
            for (uint64_t offset=0; offset<req->getSize(); offset++)
                dataArray[addr+offset] = req->getData().at(offset);
            break;
        case Request::Command::ReadResp:
        case Request::Command::WriteResp:
        default:
            std::cerr << "[Error] Unknown command type!" << std::endl;
            break;
    }
}
