/*
 This exmple program provides a trivial server program that listens for TCP
 connections on port 9995.  When they arrive, it writes a short message to
 each client connection, and closes each connection once it is flushed.

 Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
 */

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif
#include <event2/thread.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

static char IP[16] = "127.0.0.1";
static const int PORT = 43002;

static void conn_readcb(struct bufferevent *bev, void *user_data);
static void conn_writecb(struct bufferevent *, void *);
static void conn_eventcb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

struct bufferevent *bev = NULL;

int bQuit = 0;

#include "../package.h"
#include "../MessageDef.h"

void* testthread(void* arg)
{
	int nPort = *(int*) arg;

	printf("Switch to: %d\n\n", MSG_CAMERA_CD::TYPE_SWITCH_0);
	CTlvLEPackage tlvPkgSwitch(MSG_CAMERA_CD::TYPE_HEADER, 4);
	tlvPkgSwitch.PutInt(MSG_CAMERA_CD::TYPE_SWITCH_0);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgSwitch.TlvData(), tlvPkgSwitch.Size());
	bufferevent_unlock(bev);
    
	sleep(1);

	//fps
	printf("Param(fps=9) setting\n\n");
	CTlvLEPackage tlvPkgfps(MSG_CAMERA_CD::TYPE_HEADER, 3 * 4);
	tlvPkgfps.PutInt(0x55aa55aa);
	tlvPkgfps.PutInt(0x92);
	tlvPkgfps.PutInt(1);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgfps.TlvData(), tlvPkgfps.Size());
	bufferevent_unlock(bev);

	sleep(1);

    //return NULL;

	//gain
	printf("Param(gain=180) setting\n\n");
	CTlvLEPackage tlvPkgGain(MSG_CAMERA_CD::TYPE_HEADER, 3 * 4);
	tlvPkgGain.PutInt(0x55aa55aa);
	tlvPkgGain.PutInt(0x90);
	tlvPkgGain.PutInt(180);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgGain.TlvData(), tlvPkgGain.Size());
	bufferevent_unlock(bev);
	sleep(1);

	//shutter
	printf("Param(shutter=40, 000) setting\n\n");
	CTlvLEPackage tlvPkgShutter(MSG_CAMERA_CD::TYPE_HEADER, 3 * 4);
	tlvPkgShutter.PutInt(0x55aa55aa);
	tlvPkgShutter.PutInt(0x93);
	tlvPkgShutter.PutInt(40 * 1000);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgShutter.TlvData(), tlvPkgShutter.Size());
	bufferevent_unlock(bev);
	sleep(1);

	//balance
	printf("Param(balance=0) setting\n\n");
	CTlvLEPackage tlvPkgBalance(MSG_CAMERA_CD::TYPE_HEADER, 3 * 4);
	tlvPkgBalance.PutInt(0x55aa55aa);
	tlvPkgBalance.PutInt(0xd4);
	tlvPkgBalance.PutInt(0);//0-close 1-open
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgBalance.TlvData(), tlvPkgBalance.Size());
	bufferevent_unlock(bev);
	sleep(1);

	//jpeg qty
	printf("Param(qty=50) setting\n\n");
	CTlvLEPackage tlvPkgqty(MSG_CAMERA_CD::TYPE_HEADER, 3 * 4);
	tlvPkgqty.PutInt(0x55aa55aa);
	tlvPkgqty.PutInt(0x128);
	tlvPkgqty.PutInt(50);//0-close 1-open
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgqty.TlvData(), tlvPkgqty.Size());
	bufferevent_unlock(bev);

	sleep(1);

	printf("Trigger camera\n\n");
	CTlvLEPackage tlvPkgTrigger(MSG_CAMERA_CD::TYPE_HEADER, 4);
	tlvPkgTrigger.PutInt(MSG_CAMERA_CD::TYPE_TRIGGER);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgTrigger.TlvData(), tlvPkgTrigger.Size());
	bufferevent_unlock(bev);

	sleep(1);

	while (1)
	{

		printf("Camera params query\n\n");
		CTlvLEPackage tlvPkgQuery(MSG_CAMERA_CD::TYPE_HEADER, 4);
		tlvPkgQuery.PutInt(MSG_CAMERA_CD::TYPE_PARAM_QUERY);
		bufferevent_lock(bev);
		bufferevent_write(bev, tlvPkgQuery.TlvData(), tlvPkgQuery.Size());
		bufferevent_unlock(bev);

	printf("Send heart beating\n\n");
	CTlvLEPackage tlvPkgBeat(MSG_CAMERA_CD::TYPE_HEADER, 16);
	for (int i = 0; i < 4; i++)
	{
		tlvPkgBeat.PutInt(MSG_CAMERA_CD::TYPE_BEAT);
	}
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgBeat.TlvData(), tlvPkgBeat.Size());
	bufferevent_unlock(bev);

	sleep(5);
	
	}

	printf("Reset camera\n\n");
	CTlvLEPackage tlvPkgReset(MSG_CAMERA_CD::TYPE_HEADER, 4);
	tlvPkgReset.PutInt(MSG_CAMERA_CD::TYPE_RESET);
	bufferevent_lock(bev);
	bufferevent_write(bev, tlvPkgReset.TlvData(), tlvPkgReset.Size());
	bufferevent_unlock(bev);
}

int main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		strncpy(IP, argv[1], sizeof(IP));
	}

	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;

	evthread_use_pthreads();

	base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = inet_addr(IP);
	memset(sin.sin_zero, 0, 8);

	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE
			| BEV_OPT_THREADSAFE);
	if (!bev)
	{
		fprintf(stderr, "Error constructing bufferevent!\n");
		event_base_loopbreak(base);
		return 0;
	}

	bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, (void*) "43002");

	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);

	int ret;
	ret = bufferevent_socket_connect(bev, (struct sockaddr*) &sin, sizeof(sin));
	if (ret == 0)
	{
		printf("Connected to 43002\n");
	}
	else
	{
		printf("connect nPort=43002 failed\n");
	}

	struct sockaddr_in sin2;
	memset(&sin2, 0, sizeof(sin2));
	sin2.sin_family = AF_INET;
	sin2.sin_port = htons(PORT + 1);
	sin2.sin_addr.s_addr = inet_addr(IP);
	memset(sin2.sin_zero, 0, 8);

	pthread_t th1, th2;

	int i[2] =
	{ 1, 2 };
	pthread_create(&th1, NULL, testthread, &i[0]);

	event_base_dispatch(base);

	bQuit = 1;
	pthread_join(th1, NULL);

	printf("exit\n");

	//event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}

int pkgLen1 = -1;
char* pkg1 = NULL;

static void conn_readcb(struct bufferevent *bev, void *user_data)
{
	//	printf("conn_readcb\n");
	bufferevent_lock(bev);
	struct evbuffer *input = bufferevent_get_input(bev);

	if (evbuffer_get_length(input) >= 8 && pkgLen1 < 0 && !strcmp(
			(char*) user_data, "43002"))
	{
		char tl[8] =
		{ 0 };

		bufferevent_read(bev, tl, 8);

		CTlvLEPackage tlvPkg(tl);

		pkgLen1 = tlvPkg.Length();
		if (pkgLen1 <= 0)
		{
			pkgLen1 = -1;
		}
		else
		{
			printf("New TLV-package is coming(size=%dbytes)\n", pkgLen1 + 8);
			pkg1 = new char[8 + pkgLen1];
			memset(pkg1, 0, 8 + pkgLen1);
			memcpy(pkg1, tl, 8);
		}
	}

	if (pkgLen1 > 0 && evbuffer_get_length(input) >= pkgLen1 && !strcmp(
			(char*) user_data, "43002"))
	{
		CTlvLEPackage tlvPkgFrame(pkg1);

		bufferevent_read(bev, tlvPkgFrame.Data(), pkgLen1);

		int type = tlvPkgFrame.GetInt();
		switch (type)
		{
		case MSG_CAMERA_CD::TYPE_BEAT:
		{
			printf("Received beat reply>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

			pkgLen1 = -1;
			delete[] pkg1;
			pkg1 = NULL;
			break;
		}
		case MSG_CAMERA_CD::TYPE_PARAM_REPLY:
		{
		    printf("MSG_CAMERA_CD::TYPE_PARAM_REPLY: size = %d\n", tlvPkgFrame.Length());
			if (tlvPkgFrame.Length() > 12)
			{
				int cameraNo;

				int *param = new int[0x160];
				for (int i = 0; i < 0x160; i++)
				{
					param[i] = tlvPkgFrame.GetInt();
				}

				printf("Received param info from camera: %d\n", param[0]);
				printf("\tgain = %d\n", param[0x90]);
				printf("\tfps = %d\n", param[0x92]);
				printf("\tShutter = %d\n", param[0x93]);
				printf("\tBalanceMode = %d\n", param[0xd2]);
				printf("\tConstrast = %d\n", param[0xe0]);
				printf("\tJpegQuality = %d\n", param[0x128]);
				printf("\tExpEnable = %d\n", param[0x130]);

				delete[] param;
			}
			else
			{
				int paramNo = tlvPkgFrame.GetInt();
				int val = tlvPkgFrame.GetInt();

				printf("\nparamNo = 0x%x, value = %d\n\n", paramNo, val);
			}
			pkgLen1 = -1;
			delete[] pkg1;
			pkg1 = NULL;
			break;
		}
		default:
		{
			static int index = 1;
			char fileName[50] =
			{ 0 };

			//sprintf(fileName, "%d.jpeg", index++);
			//FILE* fp = fopen(fileName, "w");
			int firstInt = tlvPkgFrame.Type();

			/*fwrite(&type, 4, 1, fp);
			fwrite(tlvPkgFrame.GetBytes(tlvPkgFrame.Length() - 8), 1, pkgLen1
					- 8, fp);
			fclose(fp);
			*/

			tlvPkgFrame.GetBytes(tlvPkgFrame.Length() - 8);

			int cameraNo;

			cameraNo = tlvPkgFrame.GetInt();

			printf(
					"Received video frame(%dbytes) from camera(%d)\n",
					tlvPkgFrame.Length() - 4, cameraNo);

			pkgLen1 = -1;
			delete[] pkg1;
			pkg1 = NULL;
			break;
		}
		}
	}

	bufferevent_unlock(bev);
}

static void conn_writecb(struct bufferevent *bev, void *user_data)
{
	printf("conn_writecb\n");
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0)
	{
		//printf("flushed answer\n");
		//bufferevent_free(bev);
	}
}

static void conn_eventcb(struct bufferevent *bev1, short events,
		void *user_data)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("Connection closed.\n");
		//bufferevent_free(bev);
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("Got an error on the connection: %s\n", strerror(errno));/*XXX win32*/

		//bufferevent_free(bev1);

		//bev = NULL;
	}
	else if (events & BEV_EVENT_CONNECTED)
	{
		printf("connected\n");
	}
	else if (events & BEV_EVENT_READING)
	{
		printf("reading\n");
	}
	else if (events & BEV_EVENT_WRITING)
	{
		printf("writing\n");
	}
	else if (events & BEV_EVENT_TIMEOUT)
	{
		printf("time out\n");
	}
	else
	{
		printf("other\n");
	}

	printf("events = %d\n", events);

}

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = (struct event_base*) user_data;
	struct timeval delay =
	{ 0, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}
