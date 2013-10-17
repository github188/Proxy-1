/**
 * Class: CConfFile
 * Provide some methods to 
 * 	operate configuration files
 */
#ifndef CONFFILE_H
#define CONFFILE_H
/********************
 * Content of a configuration file
 * ...
 * #some commentary
 * [section]
 * key1=value1
 * key2=value2
 * [section2]
 * ...
 ********************/
#include<string>
#include<map>

const int MAX_SECTIONS = 32;

typedef std::map<std::string,std::string> CfgItems;

/* A section,contain a name and a map */
struct CfgSection{
	std::string sectionName;
	CfgItems items; // keys and values
};
/* This struct is used to set or query a value*/
struct SecKeyVal{
	std::string sect; // section name
	std::string key;
	std::string value;
};

/* Definition: handler for section's change*/
typedef void (*CfgHandle)(const struct CfgSection& sec,void* arg);

/* Class CConfFile  */
class CConfFile{
	/* public methods */
	public:
		/* 
		 * Constructor
		 * param: file's name,args for handler,handler
		 */
		CConfFile(const char*,void* arg = NULL,
				CfgHandle handle = NULL);
		~CConfFile(){}; 
		/* Get value by section & key's name */
		bool getValue(const char* sect,
				const char* key,char* val,size_t len);
		/* This function will add a new item 
		 * when value is not found */
		bool getValueWithDefault(const char*,
				const char*,char*,size_t);
		/* Set value by section & key's name */
		bool setValue(const char* sect,
				const char* key,const char* val);
		/* Set several values at one time */
		int setValues(const struct SecKeyVal*,size_t);
		/* Delete value by section and key's name */
		bool delValue(const char* sect,
				const char* key);
		/* Add an item */
		bool addValue(const char* sect,
				const char* key,const char* val);
		bool saveFile(void);
		bool loadFile(void); 
		void setHandle(CfgHandle h);
		void setHandleArg(void* arg);
	private: 
		int findSectIndex(const std::string& sect);
		int getIterator(const std::string,
				const std::string,CfgItems::iterator&);
		static int removeFile(const std::string &fileName);
		/* 
		 * State of file:
		 * UNUC: unload,unchanged
		 * LOUC: loaded,unchanged 
		 * LOCH: loaded,changed
		 */
		enum State{UNUC,LOUC,LOCH};
	/* Private datas */
	private:
		/* Full path of configuration file*/
		std::string m_file;
		/* All sections */
		CfgSection m_config[MAX_SECTIONS];
		/* Data's state */
		State m_state;
		/* Handler for sections */
		CfgHandle m_handle;
		/* args for handler */
		void* m_arg;
};

#endif // CONFFILE_H

