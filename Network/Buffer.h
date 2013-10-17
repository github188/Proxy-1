#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>
using namespace std;

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

class CBuffer
{
public:
    CBuffer();
    void Append(const char* data, size_t len);
    
    void Get(char* data, size_t len);

    size_t ReadableBytes();

    const char* BeginRead();

    void Retrieve(size_t len);
    void RetrieveAll();
private:
    size_t WriteableBytes();
    size_t PrependableBytes();

    char* BeginWrite();

    void MakeSpace(size_t more);
    CBuffer(const CBuffer& rhs);
    CBuffer& operator=(const CBuffer& rhs);

private:
    const static size_t DEFUALT_SIZE = 1024;

    vector<char> m_buffer;
    size_t m_readerIndex;
    size_t m_writerIndex;
};


#endif //BUFFER_H






