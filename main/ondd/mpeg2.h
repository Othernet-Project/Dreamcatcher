#ifndef __MPEG2_H
#define __MPEG2_H
// ----------------------------------------------------------------------------
// mpeg2.h
// ----------------------------------------------------------------------------
#include <list>
#include <map>

namespace mpeg2 {
// ----------------------------------------------------------------------------
// MPEG structures and constants
// ----------------------------------------------------------------------------
const unsigned int  MAX_SECTION_SIZE		= 4096;
const unsigned int  MAX_SECTION_PAYLOAD		= 4093;
const unsigned int  SECTION_HEADER_SIZE		= 12;
const unsigned int  TSPACKET_SIZE_BYTES		= 188;
const unsigned int  TSPACKET_SIZE_BITS		= 1504;
const unsigned char SYNC_BYTE				= 0x47;

// Offset from GPS time to real time
//  Unix time was zero on midnight January 1, 1970
//  GPS time was zero at midnight January 6, 1980 
//  Conversion adds 10 years, 2 days for leap years (1972, 1976) and 5 days from the 1st to the 6th
const unsigned int GPS_CONVERSION	= (( 60 * 60 * 24 * 365 * 10 ) + ( 60 * 60 * 24 * 7 ));

namespace bits
{
	const unsigned char TEI		= 0x80;
	const unsigned char PUSI	= 0x40;
	const unsigned char PRIO	= 0x20;
	const unsigned char ADAP	= 0x20;
	const unsigned char PAYLOAD	= 0x10;
}

namespace adaptation
{
	const unsigned int DISCONTINUITY	= 0x80;
	const unsigned int RANDOM_ACCESS	= 0x40;
	const unsigned int PRIORITY			= 0x20;
	const unsigned int PCR				= 0x10;
	const unsigned int OPCR				= 0x08;
	const unsigned int SPLICNG_POINT	= 0x04;
	const unsigned int PRIVATE_DATA		= 0x02;
	const unsigned int EXTENSION		= 0x01;
}

namespace pid
{
	const unsigned short PAT	= 0x0000;
	const unsigned short CAT	= 0x0001;
	const unsigned short NIT	= 0x0010;
	const unsigned short BAT	= 0x0011;
	const unsigned short SDT	= 0x0011;
	const unsigned short EIT	= 0x0012;
	const unsigned short TDT	= 0x0014;
	const unsigned short TOT	= 0x0014;
	const unsigned short STT	= 0x1FFB;
	const unsigned short VCT	= 0x1FFB;
	const unsigned short NUL	= 0x1FFF;
}

namespace table_id
{
	// MPEG transport stream table id numbers
	const unsigned char PAT			= 0x00;		// Program Allocation Table on PID 0x0000
	const unsigned char CAT			= 0x01;		// Conditional Access Table on PID 0x0001
	const unsigned char PMT			= 0x02;		// Program Map Table
	const unsigned char TSDT		= 0x03;		// Transport Stream Descriptor Table on PID 0x0002
	const unsigned char NIT			= 0x40;		// DVB Network Information Table on PID 0x0010
	const unsigned char SDT			= 0x42;		// DVB Service Description Table on PID 0x0011
	const unsigned char BAT			= 0x4A;		// DVB Bouquet Association Table on PID 0x0011
	const unsigned char EIT_PF		= 0x4E;		// DVB Event Information Table on PID 0x0012
	const unsigned char TDT			= 0x70;		// DVB Time and Date Table on pid 0x0014
	const unsigned char TOT			= 0x73;		// DVB Time Offset Table on pid 0x0014
	const unsigned char MGT			= 0xC7;		// Master Guide Table on pid 0x1FFB
	const unsigned char TVCT		= 0xC8;		// Terrestrial Virtual Channel Table on pid 0x1FFB
	const unsigned char CVCT		= 0xC9;		// Cable Virtual Channel Table
	const unsigned char RRT			= 0xCA;		// Rating Region Table
	const unsigned char EIT			= 0xCB;		// Event Information Table
	const unsigned char ETT			= 0xCC;		// Extended Text Table
	const unsigned char STT			= 0xCD;		// System Time Table on pid 0x1FFB
	const unsigned char MPE_DVB		= 0x3E;		// MPE table id ( ip encapsulation ) DVB spec
	const unsigned char MPE_ATSC	= 0x3F;		// MPE table id ( ip encapsulation ) ATSC spec
	const unsigned char WPT_CONFIG	= 0xB0;		// WPT configuration data
	const unsigned char WPT_CONTROL	= 0xB1;		// WPT control scripts
	const unsigned char WPT_OTA		= 0xB2;		// WPT OTA update
	const unsigned char WPT_COMMAND	= 0xB3;		// WPT command
	const unsigned char WPT_SIGNAL	= 0xBE;		// WPT signaling table id
	const unsigned char WPT_DATA	= 0xBF;		// WPT data
}

namespace tsdt {
	const unsigned char CONFIG		= 0x01;
	const unsigned char IPE			= 0x02;
	const unsigned char IDENT		= 0x03;
	const unsigned char COMMAND		= 0x04;
	const unsigned char CAROUSEL	= 0x05;
}

typedef unsigned char tspkt_t[ TSPACKET_SIZE_BYTES ];
typedef unsigned char section_t[ MAX_SECTION_SIZE ];

// ----------------------------------------------------------------------------
// Descriptors
// ----------------------------------------------------------------------------
typedef struct descriptor_block_t
{
	unsigned short length;
	unsigned char data[ 1024 ];
	
	descriptor_block_t();
} descriptor_block_t;

typedef struct descriptor_t
{
	unsigned char tag;
	unsigned char length;
	unsigned char data[ 256 ];
} descriptor_t;
typedef std::list< descriptor_t > descriptor_list_t;

// ----------------------------------------------------------------------------
// Section parsing helper functions
// ----------------------------------------------------------------------------
namespace section {
	unsigned int crc32( const void *buffer, unsigned int size, unsigned int crc = 0xffffffff );
	unsigned short length( const unsigned char *section );
	bool validate( const unsigned char *section );
}

// ----------------------------------------------------------------------------
// Signaling
// ----------------------------------------------------------------------------
namespace pat
{
	typedef std::map< unsigned short, unsigned short > table_t;
	typedef struct pat_t
	{
		unsigned short transport_id;
		unsigned char version;
		bool valid;
		table_t table;
		
		pat_t() : transport_id( 0xFFFF ), version( 0xFF ), valid( false ) { }
	} pat_t;
	bool parse( const unsigned char *section, pat_t &pat );
}

namespace pmt
{
	typedef struct element_t
	{
		unsigned char stream_type;
		unsigned short pid;
		descriptor_block_t descriptors;
		
		bool operator<( const element_t &e ) { return pid < e.pid; }
		
		element_t() : stream_type( 0 ), pid( 0 ) { }
	} element_t;
	typedef std::list< element_t > element_list_t;

	typedef struct pmt_t
	{
		unsigned short number;
		unsigned char version;
		bool valid;
		unsigned short pcr_pid;
		
		descriptor_block_t descriptors;
		element_list_t elements;
		
		pmt_t() : number( 0 ), version( 0xFF ), valid( false ), pcr_pid( 0x1FFF ) { }
	} pmt_t;
	bool parse( const unsigned char *section, pmt_t &program );

	bool is_element_audio( const element_t &element );
	bool is_element_video( const element_t &element );
	const char *element_type( const element_t &element );
}

// ----------------------------------------------------------------------------
} /* namespace mpeg2 */
#endif
