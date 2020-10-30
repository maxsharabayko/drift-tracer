#pragma once

#include <string>
#include <map>
#include <cstdlib>


template <class KeyType, class ValueType>
struct MapProxy
{
	std::map<KeyType, ValueType>& mp;
	const KeyType& key;

	MapProxy(std::map<KeyType, ValueType>& m, const KeyType& k): mp(m), key(k) {}

	void operator=(const ValueType& val)
	{
		mp[key] = val;
	}

	typename std::map<KeyType, ValueType>::iterator find()
	{
		return mp.find(key);
	}

	typename std::map<KeyType, ValueType>::const_iterator find() const
	{
		return mp.find(key);
	}

	operator ValueType() const
	{
		typename std::map<KeyType, ValueType>::const_iterator p = find();
		if (p == mp.end())
			return "";
		return p->second;
	}

	ValueType deflt(const ValueType& defval) const
	{
		typename std::map<KeyType, ValueType>::const_iterator p = find();
		if (p == mp.end())
			return defval;
		return p->second;
	}

	bool exists() const
	{
		return find() != mp.end();
	}
};

//++
// UriParser
//--

class UriParser
{
	// Construction
public:

	enum DefaultExpect { EXPECT_FILE, EXPECT_HOST };
	enum Type
	{
		UNKNOWN, FILE, UDP, TCP, SRT, RTMP, HTTP, RTP
	};

	UriParser(const std::string& strUrl, DefaultExpect exp = EXPECT_FILE);
	UriParser() : m_uriType(UNKNOWN) {}
	virtual ~UriParser(void);

	// Some predefined types
	Type type() const;

	typedef MapProxy<std::string, std::string> ParamProxy;

	// Operations
public:
	std::string uri() const { return m_origUri; }
	std::string proto() const;
	std::string scheme() const { return proto(); }
	std::string host() const;
	std::string port() const;
	unsigned short int portno() const;
	std::string hostport() const { return host() + ":" + port(); }
	std::string path() const;
	std::string queryValue(const std::string& strKey) const;
	ParamProxy operator[](const std::string& key) { return ParamProxy(m_mapQuery, key); }
	const std::map<std::string, std::string>& parameters() const { return m_mapQuery; }

private:
	void Parse(const std::string& strUrl, DefaultExpect);

	// Overridables
public:

	// Overrides
public:

	// Data
private:
	std::string m_origUri;
	std::string m_proto;
	std::string m_host;
	std::string m_port;
	std::string m_path;
	Type m_uriType;

	std::map<std::string, std::string> m_mapQuery;
};
