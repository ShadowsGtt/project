#include "C_MyRedis.h"
#include <iostream>
using namespace std;

CMyRedis::CMyRedis()
{
	m_pRedisContext = NULL;
	m_pRedisReply = NULL;
}
CMyRedis::~CMyRedis()
{
	if(m_pRedisContext)
	{
		redisFree(m_pRedisContext);
	}
	m_pRedisContext = NULL;
	m_pRedisReply = NULL;
}

int CMyRedis::Connect(const string& strIp,int nPort)
{
		m_strIp = strIp;
		m_nPort = nPort;
		m_pRedisContext = redisConnect(strIp.c_str(), nPort);
		if(m_pRedisContext != NULL && m_pRedisContext->err)
		{
			string err = (m_pRedisContext->errstr==NULL?"connect error":m_pRedisContext->errstr);
			int code = m_pRedisContext->err;
			redisFree(m_pRedisContext);
			m_pRedisContext = NULL;
			throw CErrorMsg(err, code, "connect error!");
            return -1;
		}
		return 0;
}


/*#define REDIS_ERR_IO 1  Error in read or write */
/*#define REDIS_ERR_EOF 3  End of file */
/*#define REDIS_ERR_PROTOCOL 4  Protocol error */
/*#define REDIS_ERR_OOM 5  Out of memory */
/*#define REDIS_ERR_OTHER 2  Everything else... */
bool CMyRedis::CheckConenct()
{
	if(m_pRedisContext)
	{
		return !(m_pRedisContext->err);
	}
	return false;
}

/*
 *字符串相关操作
 */
void CMyRedis::Set(const string & strKey,const string &strValue)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"set %s %s", strKey.c_str(), strValue.c_str());
	if(m_pRedisReply == NULL)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("set error! m_pRedisReply is NULL");
	}
	if(!(m_pRedisReply->type == REDIS_REPLY_STATUS && (strcmp(m_pRedisReply->str, "OK") == 0)))
	{
		string strerr = (m_pRedisReply->str==NULL?"set error!":m_pRedisReply->str);
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg(strerr, -1, "set error!");
	}
	freeReplyObject(m_pRedisReply);
	return;
}

string CMyRedis::Get(const string & strKey)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"get %s", strKey.c_str());
	if(m_pRedisReply == NULL)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("get error! m_pRedisReply is NULL");
	}
	if((m_pRedisReply->type != REDIS_REPLY_NIL) && (m_pRedisReply->type != REDIS_REPLY_STRING))
	{
		string strerr = (m_pRedisReply->str==NULL?"get error!":m_pRedisReply->str);
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg(strerr, -1, "get error! m_pRedisReply type error!");
	}
	string str = (m_pRedisReply->str==NULL?"":m_pRedisReply->str);
	freeReplyObject(m_pRedisReply);
	return str;
}

int CMyRedis::Del(const string & strKey)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"del %s", strKey.c_str());
	if(m_pRedisReply == NULL)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("del error! m_pRedisReply is NULL");
	}
	if(m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		string strerr = (m_pRedisReply->str==NULL?"del error!":m_pRedisReply->str);
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg(strerr, -1, "del error! m_pRedisReply type error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

/*
 *列表相关操作
 */
int CMyRedis::Push(const string & strListKey,const string & strValue,int nFlag/* = 0*/)
{
	if(0 == nFlag)
	{
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"rpush %s %s", strListKey.c_str(), strValue.c_str());
	}else
	{
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"lpush %s %s", strListKey.c_str(), strValue.c_str());
	}
	if(m_pRedisReply == NULL)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("push error! m_pRedisReply is NULL");
	}
	if(m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		string strerr = (m_pRedisReply->str==NULL?"push error!":m_pRedisReply->str);
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg(strerr, -1, "push error! m_pRedisReply type error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

string CMyRedis::Pop(const string & strListKey,int nFlag /*= 0*/)
{
	if(0 == nFlag)
	{
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"rpop %s", strListKey.c_str());
	}else
	{
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"lpop %s", strListKey.c_str());
	}
	if(m_pRedisReply == NULL || (m_pRedisReply->type != REDIS_REPLY_NIL && m_pRedisReply->type != REDIS_REPLY_STRING))
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("pop error!");
	}
	string str = "";
	if(m_pRedisReply->type == REDIS_REPLY_NIL)
		str = "";
	if(m_pRedisReply->type == REDIS_REPLY_STRING)
		str = m_pRedisReply->str;
	freeReplyObject(m_pRedisReply);
	return str;
}

string CMyRedis::Lindex(const string & strListKey,int nIndex)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"lindex %s %d", strListKey.c_str(), nIndex);
	if(m_pRedisReply == NULL || (m_pRedisReply->type != REDIS_REPLY_NIL && m_pRedisReply->type != REDIS_REPLY_STRING))
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("lindex error!");
	}
	string str = "";
	if(m_pRedisReply->type == REDIS_REPLY_NIL)
		str = "";
	if(m_pRedisReply->type == REDIS_REPLY_STRING)
		str = m_pRedisReply->str;
	freeReplyObject(m_pRedisReply);
	return str;	
}

void CMyRedis::Lrange(const string & strListKey,int nStart,int nEnd,string strArray[],int & nCount)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"lrange %s %d %d", strListKey.c_str(), nStart, nEnd);
	if(m_pRedisReply == NULL || m_pRedisReply->type!= REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("lrange error!");
	}
	int nMax = nCount;
	size_t i = 0;
	for(; i<m_pRedisReply->elements; i++)
	{
		if((int)i>=nMax)
			break;
		strArray[i] = m_pRedisReply->element[i]->str;
	}
	nCount = i;
	freeReplyObject(m_pRedisReply);
	return;
}
/*
 *集合相关操作
 */
int CMyRedis::Sadd(const string & strSetKey,const string & strItem)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"sadd %s %s",strSetKey.c_str(), strItem.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("sadd error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

int CMyRedis::Srem(const string & strSetKey,const string & strItem)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"srem %s %s",strSetKey.c_str(), strItem.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("sadd error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

bool CMyRedis::SisMember(const string & strSetKey,const string & strItem)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"sismember %s %s",strSetKey.c_str(), strItem.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("sadd error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

void CMyRedis::Smembers(const string & strSetKey,string strArray[],int& nCount)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"smembers %s", strSetKey.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type!= REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("smembers error!");
	}
	int nMax = nCount;
	size_t i = 0;
	for(; i<m_pRedisReply->elements; i++)
	{
		if((int)i>=nMax)
			break;
		strArray[i] = m_pRedisReply->element[i]->str;
	}
	nCount = i;
	freeReplyObject(m_pRedisReply);
	return;
}

/*
 *散列操作
 */
int CMyRedis::HSet(const string & strHashKey,const string & key,const string & value)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"hset %s %s %s",strHashKey.c_str(), key.c_str(), value.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("hset error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

string CMyRedis::HGet(const string & strHashKey,const string & key)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"hget %s %s", strHashKey.c_str(), key.c_str());
	if(m_pRedisReply == NULL || (m_pRedisReply->type != REDIS_REPLY_NIL && m_pRedisReply->type != REDIS_REPLY_STRING))
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("hget error!");
	}
	string str = "";
	if(m_pRedisReply->type == REDIS_REPLY_NIL)
		str = "";
	if(m_pRedisReply->type == REDIS_REPLY_STRING)
		str = m_pRedisReply->str;
	freeReplyObject(m_pRedisReply);
	return str;	

}

int CMyRedis::HDel(const string & strHashKey,const string & key)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"hdel %s %s",strHashKey.c_str(), key.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("hdel error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;

}
void CMyRedis::HGetAll(const string & strHashKey, string strArray[], int& nCount)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"hgetall %s",strHashKey.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("hgetall error!");
	}
	int nMax = nCount;
	size_t i = 0;
	for(; i<m_pRedisReply->elements; i++)
	{
		if((int)i>=nMax)
			break;
		strArray[i] = m_pRedisReply->element[i]->str;
	}
	nCount = i;
	freeReplyObject(m_pRedisReply);
	return;
}

/*
 *有序集合相关操作
 */
int CMyRedis::ZAdd(const string & strZSetKey,int nScore,const string & value)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"zadd %s %d %s",strZSetKey.c_str(), nScore, value.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		string strError = m_pRedisReply->str;
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg(strError);
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

void CMyRedis::ZRange(const string & strZSetKey,int nStartIndex,int nEndIndex,
						string strArray [ ],int & nCount)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"zrange %s %d %d",
												strZSetKey.c_str(),
												nStartIndex,
												nEndIndex);
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("zrange error!");
	}
	int nMax = nCount;
	size_t i = 0;
	for(; i<m_pRedisReply->elements; i++)
	{
		if((int)i>=nMax)
			break;
		strArray[i] = m_pRedisReply->element[i]->str;
	}
	nCount = i;
	freeReplyObject(m_pRedisReply);
	return;	
}

void CMyRedis::ZRangeByScore(const string & strZSetKey,int nStartScore,int nEndScore,string strArray [ ],int & nCount)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"zrangebyscore %s %d %d",
												strZSetKey.c_str(),
												nStartScore,
												nEndScore);
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("zrangebyscore error!");
	}
	int nMax = nCount;
	size_t i = 0;
	for(; i<m_pRedisReply->elements; i++)
	{
		if((int)i>=nMax)
			break;
		strArray[i] = m_pRedisReply->element[i]->str;
	}
	nCount = i;
	freeReplyObject(m_pRedisReply);
	return;	
}

int CMyRedis::ZRem(const string & strZSetKey,const string & value)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"zrem %s %s",strZSetKey.c_str(), value.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_INTEGER)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("zrem error!");
	}
	int nRet = m_pRedisReply->integer;
	freeReplyObject(m_pRedisReply);
	return nRet;
}

int CMyRedis::Hmset(int &key,vector<pair<string,string> > &vec)
{

    for(auto &c : vec)
    {
        m_pRedisReply = (redisReply*)redisCommand       \
                (                                       \
                m_pRedisContext,                        \
                "hmset %d %s %s",key,c.first.c_str(),   \
                c.second.c_str()                        \
                );                      

        if(m_pRedisReply == NULL || m_pRedisReply->type == REDIS_REPLY_ERROR )
        {

            cout<<"***redis command error***"<<endl;
            cout<<m_pRedisReply->str<<endl;
            freeReplyObject(m_pRedisReply);
            
            return -1;
        }

    }
	freeReplyObject(m_pRedisReply);
	return 0;
}
vector<pair<string,string> >  CMyRedis::Hmget(int &key,vector<pair<string,string> > &vec)
{
    vector<pair<string,string> >  ret;
    for(auto &c : vec)
    {
        m_pRedisReply = (redisReply*)redisCommand       \
                (                                       \
                m_pRedisContext,                        \
                "hmget %d %s %s",key,c.first.c_str(),   \
                c.second.c_str()                        \
                );                      

        if(m_pRedisReply == NULL || m_pRedisReply->type == REDIS_REPLY_ERROR )
        {

            cout<<"***redis command error***"<<endl;
            cout<<m_pRedisReply->str<<endl;
            freeReplyObject(m_pRedisReply);
            
            return ret;
        }
        ret.push_back(pair<string,string>(c.first.c_str(),c.second.c_str()) );

    }
	freeReplyObject(m_pRedisReply);
	return ret;

    
}

int CMyRedis::GetCommand(string &command,vector<string> &ResultData,int &ResultDataNum)
{
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext,"%s",command.c_str());
	if(m_pRedisReply == NULL || m_pRedisReply->type != REDIS_REPLY_ARRAY)
	{
		freeReplyObject(m_pRedisReply);
		throw CErrorMsg("zrangebyscore error!");
	}
    ResultDataNum = m_pRedisReply->elements;
    for(int i = 0 ;i < m_pRedisReply->elements; i++)
    {
        ResultData.push_back(m_pRedisReply->element[i]->str); 
    }

    return 0;
}
