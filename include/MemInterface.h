#ifndef SIMPLEMEM_H
#define SIMPLEMEM_H

#include <vector>
#include <cstdint>
#include <iostream>

class SimpleMem {
public:
    // 내부 클래스 Request 정의
    class Request {
    public:
        enum class Command { Read, Write, ReadResp };
        static constexpr Command Read = Command::Read;
        static constexpr Command Write = Command::Write;
        static constexpr Command ReadResp = Command::ReadResp;

        Request(Command type, uint64_t addr, uint32_t size)
            : type(type), addr(addr), size(size) {}
        Request(Command type, uint64_t addr, uint32_t size, const std::vector<uint8_t>& data)
            : type(type), addr(addr), size(size), data(data) {}

        // Getter functions
        Command getType() const { return type; }
        uint64_t getAddress() const { return addr; }
        size_t getSize() const { return size; }
        const std::vector<uint8_t>& getData() const { return data; }
        void setPayload(const std::vector<uint8_t>& newPayload) { data = newPayload; }
        void sendInitData(Request* initRequest );

        uint32_t id;
    private:
        Command type;
        uint64_t addr;
        size_t size;
        std::vector<uint8_t> data;
    };

    SimpleMem() {
        std::cout << "SimpleMem initialized" << std::endl;
    }

    virtual ~SimpleMem() {}

    // 초기화 메서드
    virtual void init(int phase) {
        std::cout << "SimpleMem initialized with phase " << phase << std::endl;
    }

    // 초기화 데이터 전송 메서드
    virtual void sendInitData(Request* req) {
        std::cout << "Sending init data to address " << req->getAddress() 
                  << " of size " << req->getSize() << " bytes." << std::endl;
        processRequest(req);
    }

protected:
    // 실제 요청을 처리하는 가상 함수 (파생 클래스에서 구현 가능)
    virtual void processRequest(Request* req) {
        if (req->getType() == Request::Write) {
            std::cout << "Processing Write request at address " << req->getAddress() << std::endl;
        } else {
            std::cout << "Processing Read request at address " << req->getAddress() << std::endl;
        }
    }
};

#endif // SIMPLEMEM_H
