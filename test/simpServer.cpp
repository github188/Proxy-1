#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include"jsbytearray.h"
#include"timeclass.h"

int createPkgAndSend(int fd, bool overspeed, int speed)
{
	CJSByteArray pack;

	int type = 0;
	type = CreateType(1, 0, 0, 1000);
	
	pack.PutInt(type);  // T 4
	pack.PutInt(0);     // L 4
	overspeed ? pack.PutInt(2) : pack.PutInt(0);     //   4

	char monitorID[32];
	monitorID[0] = 0;
	strcpy(monitorID, "10001");
	pack.PutString(monitorID, 32); // 32 

	char direction[8];
	direction[0] = 0;
	strcpy(direction, "01");
	pack.PutString(direction, 8);   // 8
	pack.PutInt(1);  // 4 川大智胜，从0开始

	pack.PutInt(1);      // 4

	pack.PutString("川A12345", 16);  // 16
	pack.PutInt(1);        // 4
	pack.PutInt(2);        // 4
	pack.PutInt(3);    // 4
	pack.PutInt(4);   // 4
	pack.PutInt(5);    // 4
	pack.PutShort(6);  // 2

	pack.PutShort(7);       // 2
	pack.PutShort(8);       // 2

	pack.PutByte(0x00);               // 1

	pack.PutByte(9);       // 1

	pack.PutInt(speed);         // 4  * speed
	pack.PutInt(60);    // 4  * limiteSpeed

	char pID[64];
	pID[0] = 0;
	strcpy(pID, "ABCDEFGHIJK");
	pack.PutString(pID, 64);          // 64

	pack.PutShort(1);    // 2  imageNum

	for (int i=0; i < 1; i++)
	{
		//时间
        CTime t(time(NULL));
        pack.PutShort(t.Year());      // 2   *
        pack.PutShort(t.Month());     // 2
        pack.PutShort(t.Day());       // 2
        pack.PutShort(t.Hour());      // 2
        pack.PutShort(t.Minute());    // 2
        pack.PutShort(t.Second());    // 2
        pack.PutShort(t.MilliSecond()); // 2

		char imgData[] = {'I','m','a','g','e','D','a','t','a'};
		//图片长度
		pack.PutLong(9);

		//jpeg图片数据
		pack.PutByte((unsigned char*)(imgData), 9);
	}

	pack.End();

	return write( fd, pack.GetData(), pack.Length() );
}


int main(int argc, char** argv)
{
	int fd = -1;
	struct sockaddr_in addrSelf, addrPeer;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == fd) 
	{
		perror("socket");
		return -1;
	}
	
	addrSelf.sin_family = AF_INET;
	addrSelf.sin_port = htons(9999);
	addrSelf.sin_addr.s_addr = 0;
	if( -1 == bind( fd, (struct sockaddr*)&addrSelf, sizeof(addrSelf) ) )
	{
		perror("bind");
		close(fd);
		return -1;
	}

	if( -1 == listen(fd, 5) )
	{
		perror("listen");
		close(fd);
		return -1;
	}

	socklen_t lPeer = sizeof(addrPeer);
	int peerFd = -1;
	peerFd = accept(fd, (struct sockaddr*)&addrPeer, &lPeer);
	if( -1 == peerFd )
	{
		perror("accept");
		close(fd);
		return -1;
	} else {
		close(fd);
		while(1) {
			if(0 >= createPkgAndSend(peerFd, false, 69) 
						|| 0 >= createPkgAndSend(peerFd, true, 69) )
			{
				printf("Peer closed \n");
				break;
			}
			sleep(10);
		}
	}
	close(peerFd);

	return 0;
}
