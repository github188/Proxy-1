#include "Buffer.h"

CBuffer::CBuffer():
m_buffer(DEFUALT_SIZE),
m_readerIndex(0),
m_writerIndex(0)
{
}


void CBuffer::Append(const char *data, size_t len)
{
    if (data == NULL || len <= 0)
    {
        return;
    }

    if (WriteableBytes() < len)
    {
        MakeSpace(len);
    }

    assert(WriteableBytes() >= len);

    copy(data, data+len, BeginWrite());

    m_writerIndex += len;
}


void CBuffer::Get(char* data, size_t len)
{
    if (data == NULL || len <= 0)
    {
        return;
    }

    memcpy(data, BeginRead(), len);
    Retrieve(len);
}


void CBuffer::MakeSpace(size_t more)
{
    assert(more > 0);

    if (WriteableBytes()+PrependableBytes() < more)
    {
        m_buffer.resize(m_writerIndex + more);
    }
    else
    {
        //将数据移动到开头
        size_t used = ReadableBytes();
        copy(&*m_buffer.begin()+m_readerIndex,
            &*m_buffer.begin()+m_writerIndex,
            m_buffer.begin());
        m_readerIndex = 0;
        m_writerIndex = used;

        assert(used == ReadableBytes());
    }
}


size_t CBuffer::ReadableBytes()
{
    return m_writerIndex - m_readerIndex;
}


size_t CBuffer::WriteableBytes()
{
    return m_buffer.size() - m_writerIndex;
}


size_t CBuffer::PrependableBytes()
{
    return m_readerIndex;
}


const char* CBuffer::BeginRead()
{
    return &*m_buffer.begin()+m_readerIndex;
}


char* CBuffer::BeginWrite()
{
    return &*m_buffer.begin()+m_writerIndex;
}


void CBuffer::Retrieve(size_t len)
{
    assert(len <= ReadableBytes());

    m_readerIndex += len;

    if (m_readerIndex == m_writerIndex)
    {
        m_readerIndex = 0;
        m_writerIndex = 0;
    }
}


void CBuffer::RetrieveAll()
{
    m_readerIndex = 0;
    m_writerIndex = 0;
}




