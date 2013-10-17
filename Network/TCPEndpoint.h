
#ifndef TCPENDPOINT_H
#define TCPENDPOINT_H

#include <Network/Session.h>

#include <string>

class TCPEndpoint {
	public:
		enum MODE {CLIENT = 0, SERVER = 1, OTHER = 2}

		TCPEndpoint(std::string ip, unsigned short port, MODE mod) :
			m_ip(ip), m_port(port), m_mode(mod), m_psession(NULL) {}
		virtual ~TCPEndpoint() {}

		bool operator==(const TCPEndpoint &rhs);
	protected:
		std::string m_ip;
		unsigned short m_port;
		MODE m_mode;
		CSession *m_psession;
};

#endif
