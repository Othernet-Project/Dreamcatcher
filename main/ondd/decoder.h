
#pragma once
#include <map>
#include "tinyxml2.h"

static const unsigned int  FOOTER_MAGIC		= 0x54464C46;

static const unsigned int  FRAG_SIZE			= 1468;
static const unsigned char FRAG_TYPE1_DATA		= 0xE1; // mpeg2
static const unsigned char FRAG_TYPE1_END		= 0x78;
static const unsigned char FRAG_TYPE2_DATA		= 0xC3; // 
static const unsigned char FRAG_TYPE2_END		= 0x3C;
static const unsigned char FRAG_TYPE2_FEC		= 0x69;

static const unsigned char BLOCK_TYPE_DATA				= 0x18;
static const unsigned char BLOCK_TYPE_SIGNALING			= 0x42;
static const unsigned char BLOCK_TYPE_SIGNALING_ZLIB	= 0x5A;
static const unsigned char BLOCK_TYPE_FILE_META			= 0x69;
static const unsigned char BLOCK_TYPE_IDENT				= 0x81;
static const unsigned char BLOCK_TYPE_FEC				= 0xFF;

static const unsigned int  MAXIMUM_BLOCK_SIZE	= 20 * 1024;
static const unsigned int  MINIMUM_FREE_SPACE	= 120 * 1024 * 1024;


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

// std::string sanitize( const std::string &in );

} /* namespace util */

typedef struct
{
    unsigned int carousel_id	= 0;	// Carousel this file is on
    uint64_t size				= 0;	// Size of transfer
    std::string path;					// Destination path
    std::string hash;					// SHA256 hash of file
    unsigned int block_size		= 0;	// Size of blocks
    std::string fec;					// FEC information
} file_t;
typedef std::map< unsigned int, file_t > signaling_t;		// transfer id, file_t
signaling_t m_signaling;
const signaling_t &signaling 	= m_signaling;

typedef struct
{
    unsigned int k			= 0;
    unsigned int n			= 0;
    unsigned int N1			= 0;
    unsigned int seed		= 0;
} fec_params_t;

typedef struct _transfer
{
    typedef _transfer* ptr_t;

    unsigned int id				= 0;				// ID of the file we are currently receiving
    uint64_t size				= 0;				// Size of file
    std::string hash;								// SHA256 hash of file
    std::string path;								// Final destination path of file
    unsigned int block_size		= 0;				// Size of blocks
    unsigned int block_count	= 0;				// Total number of blocks in file
    unsigned int block_recv		= 0;				// Blocks received thus far
    unsigned char* block_map;		// Bitmap of data blocks received
    unsigned int blockmap_size	= 0;				// Size of data block map

    fec_params_t fec_params;						// FEC codec specific parameters
    unsigned int fec_count		= 0;				// Count of FEC blocks we expect
    unsigned char* fec_map;			// Bitmap of fec blocks received
    unsigned int fecmap_size	= 0;				// Size of fec block map

    bool priority				= false;			// Priority download ( FW update )
} transfer_t;
typedef std::map< unsigned int, transfer_t::ptr_t > transfer_map_t;		// carousel_id, transfer
transfer_map_t m_transfers;

typedef struct _block
{
    typedef _block* ptr_t;

    unsigned char* data;	// Pointer to frag buffer

    unsigned int frag_size		= 0;		// Size of this set of fragments
    unsigned short next_index	= 0;		// Next index expected

    unsigned short frag_count	= 0;
    unsigned short frag_recv	= 0;
    unsigned short fec_count	= 0;
    unsigned short fec_recv		= 0;
    unsigned char frag_map[ 256 / 8 ];		// bitmap of fragments received
    int fec_i[ 256 ];						// Indicies of data / fec fragments
} block_t;

typedef std::map< unsigned int, block_t::ptr_t > block_map_t;			// carousel_id, block
block_map_t m_blocks;


//    FT ID  /last index/type length (24)/ CRC???      /
// 00 3c 01 / 00 00    /  18  00 00 f0  / 00 01 96 50 / 00 3b 42

