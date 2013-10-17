
#ifndef CONVERT_H
#define CONVERT_H

class Convert {
	public:
		Convert();
		virtual ~Convert();
	public:
		virtual const char *Conv(CSession *pSession, char* data, int size) = 0;
	protected:
};


#endif
