#include "ConfFile.h"
#include<fstream>
#include<iostream>
#include<cstring>

using namespace std;
/**
 * Constructor
 */
CConfFile::CConfFile(const char* file,void* arg,CfgHandle handle)
{
	m_state = UNUC;
	m_file = string(file);
	m_handle = handle;
	m_arg = arg;
	for(int i=0;i<MAX_SECTIONS;i++){
		m_config[i].sectionName = "";
	}
	if(file != NULL && loadFile())
		m_state = LOUC;
}

/* Load configuration file */
bool CConfFile::loadFile(void)
{
	int index = -1;  // index of section
	string temp;   
	ifstream fin; 
	// Open the configuration file
	fin.open(m_file.c_str());
	if(fin.is_open()) {
		while(!fin.eof()){
			temp.clear();
			fin>>temp;
			// Empty line or commentary,do nothing
			if(temp.size() == 0 || temp[0] == '#')
				continue;
			// New section
			if(temp[0] == '['){
				unsigned int pos = temp.find_first_of(']');
				if(pos == string::npos)
					continue; // Wrong configuration
				index ++;
				m_config[index].sectionName = temp.substr(1,pos-1);
			} else if(index == -1) {
				continue; // This line is not in a section
			} else {
				unsigned int pos = temp.find_first_of('=');
				if(pos == string::npos){
					continue;
				} else {
					// cout<<"new item: "<<endl;
					string str1,str2;
					str1 = temp.substr(0,pos);
					str2 = temp.substr(pos+1);
					// Insert an item into this section
					m_config[index].items.insert(
							pair<string,string>(str1,str2) );
				}
			}
		} // End of while
		fin.close();
		return true;
	} else {
		// Open file failed
		// fin.close();
		return false;
	} // End of if(fin.is_open())
}

bool CConfFile::saveFile(void)
{
	if( m_state <= LOUC)
		return true; // unchanged 
	ofstream fout;
	fout.open(m_file.c_str(),ios_base::trunc);
	if(!fout.is_open())return false;

	for(int i = 0; i < MAX_SECTIONS; i++) {
		if(m_config[i].sectionName != ""){
			fout<<'['<<(m_config[i].sectionName)<<']'<<'\n';
			CfgItems *p = &(m_config[i].items);
			CfgItems::iterator cp = p->begin();
			while(cp !=  p->end()){
				fout<<cp->first<<'='<<cp->second<<'\n';
				cp++;
			}
		}
	}
	fout.close();
	m_state = LOUC;
	return true;
}

/* 通过sect名字找到sect的下标  */
int CConfFile::findSectIndex(const string& sect)
{
	if(sect == "") return -1; // 空的字符串
	for(int i = 0; i< MAX_SECTIONS; i++){
		if(m_config[i].sectionName == sect)
			return i;
	}
	return -1;
}

/* 通过项以及key找到迭代器 */
int CConfFile::getIterator(string sect,
		string key,CfgItems::iterator& cp)
{
	int index = -1;
	index = findSectIndex(sect); // 找到对应的项
	if(index == -1)return -1; // 没有找到项
	CfgItems *ptr = NULL;
	ptr = &(m_config[index].items); 
	cp = ptr->find(key);
	if(cp == ptr->end())
		return -1;// 没有查找到key
	return index; // 返回true表示出参cp可用
}

/* 从配置中获取一个key的值 */
bool CConfFile::getValue(const char* sect,
		const char* key,char *val,size_t len)
{
	if(sect == NULL || key == NULL || val == NULL)
		return false;
	CfgItems::iterator cp;
	if(0 <= getIterator(string(sect),string(key),cp) ) {
		// 若找到则拷贝值并返回true
		memset(val, 0, len);
		strncpy(val,cp->second.c_str(),
			len > cp->second.size()?cp->second.size():len );
		return true;
	}
	return false;
}

/* 从配置中获取一个key的值,如果没有则设置 */
bool CConfFile::getValueWithDefault(const char* sect,
		const char*key,char* val,size_t len)
{
	if(sect == NULL || key == NULL || val == NULL)
		return false;
	if( ! getValue(sect,key,val,len) )
		return addValue(sect,key,val);
	else 
		return true;
}

/* 设置某个配置的值 */
bool CConfFile::setValue(const char* sect,
		const char* key,const char* val)
{
	if(sect == NULL || key == NULL || val == NULL)
		return false;
	int index;
	CfgItems::iterator cp;
	index = getIterator(string(sect),string(key),cp);
	if(index >= 0) {
		cp->second.clear();
		cp->second = string(val);
		if(m_handle != NULL) // 如果有回调，则调用
			(*m_handle)(m_config[index],m_arg);
		m_state = LOCH;
		return true;
	}	
	return false;
}

/* 一次配置多个项,并根据配置情况调用回调函数 */
int CConfFile::setValues(const struct SecKeyVal* ptr,size_t count)
{
	if(ptr == NULL)return false;
	int ret = 0; // 返回设置的个数
	bool signs[MAX_SECTIONS] = {false}; // 用来标志被修改段
	for(size_t i;i<count;i++){// 对于每个要修改的键值对
		int index = -1; // 定位
		CfgItems::iterator cp; // 迭代器
		index = getIterator(ptr[i].sect,ptr[i].key,cp);
		if(index < 0)continue; // 没有找到配置项
		cp->second.clear();
		cp->second = ptr[i].value;
		signs[index] = true; // 标记为已修改
		ret++;
	}
	if(m_handle != NULL){
		for(int j = 0;j<MAX_SECTIONS;j++){
			if(signs[j])
				(*m_handle)(m_config[j],m_arg);
		}
	}
	if(ret > 0) m_state = LOCH;
	return ret;
}

bool CConfFile::delValue(const char* sect,
		const char* key)
{
	if(sect == NULL || key == NULL)
		return false;
	int index = -1;
	index = findSectIndex(string(sect)); // 找到对应的项
	if(index == -1)return true; //相当于删除成功？
	CfgItems *ptr = NULL;
	ptr = &(m_config[index].items); 
	CfgItems::iterator cp = ptr->find(string(key));
	if(cp == ptr->end())
		return true;// 没有查找到key
	//如果有，执行删除操作
	ptr->erase(cp);
	m_state = LOCH;
	return true; // 返回true表示出参cp可用
}

bool CConfFile::addValue(const char* sect,
		const char* key,const char*val)
{
	if(sect == NULL || key == NULL || val == NULL)
		return false;
	int index = -1;
	index = findSectIndex(string(sect));
	if(index == -1) {// 要新建一个项
		for(int k = 0;k < MAX_SECTIONS; k++){
			if(m_config[k].sectionName == ""){
				m_config[k].sectionName = string(sect);
				m_config[k].items.insert(
						pair<string,string>( string(key),string(val) ) 
						);
				m_state = LOCH;
				return true;
			}
		}
		return false; // 没有可用的项了
	} else { // 加入到已有的项中
		CfgItems *ptr = &(m_config[index].items);
		ptr->insert( pair<string,string>( string(key),string(val) ) );
		m_state = LOCH;
		return true;
	}
}

/* 删除一个文件 */
int CConfFile::removeFile(const string &fileName)
{
	int nRet = 0;
	if(!fileName.empty()) {
		nRet = remove(fileName.c_str());
		if(nRet == -1){
			printf("remove %s failed!\n",fileName.c_str());
			perror("remove");
		}
	}
	return nRet;
}

void CConfFile::setHandleArg(void* arg)
{
	m_arg = arg;
}

void CConfFile::setHandle(CfgHandle h)
{
	m_handle = h;
}
