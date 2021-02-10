#ifndef __UTIL_H
#define __UTIL_H
// ----------------------------------------------------------------------------
// util.h
// ----------------------------------------------------------------------------
#include <stdint.h>

#include <string>
#include <vector>
#include <map>

#include "tinyxml2.h"

namespace util {
// ----------------------------------------------------------------------------
// string to int
// ----------------------------------------------------------------------------
bool strtoi( const std::string &s, uint64_t &v, int base = 0 );
bool strtoi( const std::string &s, unsigned int &v, int base = 0 );
bool strtoi( const std::string &s, signed int &v, int base = 0 );
bool strtoi( const std::string &s, unsigned short &v, int base = 0 );
bool strtof( const std::string &s, float &v );

// ----------------------------------------------------------------------------
// xml
// ----------------------------------------------------------------------------
tinyxml2::XMLElement *xml_add_element( tinyxml2::XMLNode *root, const std::string &name );
tinyxml2::XMLElement *xml_add_element( tinyxml2::XMLNode *root, const std::string &name, const std::string &s );
tinyxml2::XMLElement *xml_add_element( tinyxml2::XMLNode *root, const std::string &name, const char *fmt, ... );
void xml_add_text( tinyxml2::XMLNode *root, const char *fmt, ... );
bool xml_node_value( const tinyxml2::XMLNode *node, std::string &value );
bool xml_node_value( const tinyxml2::XMLNode *node, uint64_t &value );
bool xml_node_value( const tinyxml2::XMLNode *node, unsigned int &value );
bool xml_node_value( const tinyxml2::XMLNode *node, unsigned short &value );
bool xml_node_value( const tinyxml2::XMLNode *node, bool &value );

// ----------------------------------------------------------------------------
// base encode / decode
// ----------------------------------------------------------------------------
// void base16_encode( const void *in, unsigned int size, std::string &out );
// void base16_decode( const std::string &in, void *out, unsigned int size );

void base64_encode( const void *in, unsigned int size, std::string &out );
void base64_decode( const std::string &in, void *out, unsigned int size );

// ----------------------------------------------------------------------------
// args
// ----------------------------------------------------------------------------
typedef std::vector< std::string > array_t;
typedef std::map< std::string, std::string > params_t;

void split( const std::string &text, const char delimiter, array_t &array );

bool parse_params( const std::string &s, params_t &params, const char delim = '&' );

namespace endian {
// ----------------------------------------------------------------------------
// endian
// ----------------------------------------------------------------------------
unsigned char *write8( unsigned char *&ptr, unsigned char v );
unsigned char *write16( unsigned char *&ptr, unsigned short v );
unsigned char *write24( unsigned char *&ptr, unsigned int v );
unsigned char *write32( unsigned char *&ptr, unsigned int v );
unsigned char *write64( unsigned char *&ptr, uint64_t v );
unsigned char *writeN( unsigned char *&ptr, const void *d, unsigned int len );

unsigned char read8( const unsigned char *&ptr );
unsigned short read16( const unsigned char *&ptr );
unsigned int read24( const unsigned char *&ptr );
unsigned int read32( const unsigned char *&ptr );
uint64_t read64( const unsigned char *&ptr );
void readN( const unsigned char *&ptr, void *d, unsigned int len );
void readS( const unsigned char *&ptr, std::string &s, unsigned int len );
} /* namespace endian */

// ----------------------------------------------------------------------------
// misc
// ----------------------------------------------------------------------------
std::string fmt( const char *fmt, ... );
std::string cat( const char *fmt, ... );
bool echo( const std::string &data, const char *fmt, ... );
std::string sanitize( const std::string &in );
std::string fmt_time( time_t t );
std::string trim( const std::string &s );
void to_lower( std::string &s );
bool leftcmp( const char *left, const char *right );

bool get_if_ip( const std::string &interface, std::string &ip );

// ----------------------------------------------------------------------------
} /* namespace util */
#endif
