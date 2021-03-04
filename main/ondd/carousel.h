#ifndef __CAROUSEL_H
#define __CAROUSEL_H
//-----------------------------------------------------------------------------
// carousel.h
//-----------------------------------------------------------------------------
#include <stdint.h>

#include <list>
#include <vector>
#include <map>
#include <string>

#include "fd.h"
#include "pointer.h"

//-----------------------------------------------------------------------------
// class def
//-----------------------------------------------------------------------------
class carousel : public virtual wpt::ref
{
	public:
		typedef wpt::ref_ptr< carousel > ptr_t;

		carousel();
		virtual ~carousel();

		const std::string &cache_path = m_cache_path;
		const std::string &output_path = m_output_path;
		bool set_output_path( const std::string &output_path );

		typedef std::map< std::string, std::string > file_index_t;		// path, hash
		const file_index_t &index = m_file_index;
		typedef std::list< std::string > path_list_t;
		void index_erase( const std::string &path );
		void index_erase( const path_list_t &paths );

		const std::string &server_id	= m_server_id;

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
		const signaling_t &signaling 	= m_signaling;

		typedef struct
		{
			std::string path;
			std::string hash;
			unsigned int block_count	= 0;
			unsigned int block_recv		= 0;
			bool complete				= false;
		} xfer_t;
		typedef std::list< xfer_t > xfer_list_t;
		typedef std::map< unsigned int, xfer_t > xfer_map_t;
		void query_transfers( xfer_map_t &transfers, const std::string &only_path ) const;
		void query_cache( xfer_list_t &cache ) const;

		class callback
		{
			public:
				virtual ~callback() { }
				typedef enum
				{
					kDiscard,
					kDownload,
					kPriority,
				} decision_t;
				// virtual decision_t filter_file( const file_t &file ) =0;
				virtual void fileComplete( const std::string &path ) =0;
				virtual void processFile(unsigned int, unsigned int) =0;
				// virtual bool validate_cert( X509 *cert ) =0;
		};

		bool init( const std::string &path, callback *cb );

		void consume( const unsigned char *data, unsigned int length );

	private:
		callback *m_callback		= 0;

		std::string m_cache_path	= "/files/tmp";
		std::string m_output_path	= "/files";
		bool m_need_copy			= false;		// If the cache and output paths are different volumes we will copy.

		std::string m_server_id;
		signaling_t m_signaling;

		file_index_t m_file_index;
		bool load_index();
		void save_index() const;

		typedef struct _block : public wpt::ref
		{
			typedef wpt::ref_ptr< _block > ptr_t;

			wpt::heap_ptr< unsigned char > data;	// Pointer to frag buffer

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

		typedef struct
		{
			unsigned int k			= 0;
			unsigned int n			= 0;
			unsigned int N1			= 0;
			unsigned int seed		= 0;
		} fec_params_t;

		typedef struct _transfer : public wpt::ref
		{
			typedef wpt::ref_ptr< _transfer > ptr_t;

			unsigned int id				= 0;				// ID of the file we are currently receiving
			uint64_t size				= 0;				// Size of file
			std::string hash;								// SHA256 hash of file
			std::string path;								// Final destination path of file
			unsigned int block_size		= 0;				// Size of blocks
			unsigned int block_count	= 0;				// Total number of blocks in file
			unsigned int block_recv		= 0;				// Blocks received thus far
			wpt::heap_ptr< unsigned char > block_map;		// Bitmap of data blocks received
			unsigned int blockmap_size	= 0;				// Size of data block map

			fec_params_t fec_params;						// FEC codec specific parameters
			unsigned int fec_count		= 0;				// Count of FEC blocks we expect
			wpt::heap_ptr< unsigned char > fec_map;			// Bitmap of fec blocks received
			unsigned int fecmap_size	= 0;				// Size of fec block map

			bool priority				= false;			// Priority download ( FW update )
			int fd						= -1;										// Open cache file
		} transfer_t;
		typedef std::map< unsigned int, transfer_t::ptr_t > transfer_map_t;		// carousel_id, transfer
		transfer_map_t m_transfers;

		inline bool bit_isset( const unsigned char *map, unsigned int index ) const
		{
			return ( map[ index >> 3 ] & ( 1 << ( index & 0x7 ) ) );
		}
		inline void bit_set( unsigned char *map, unsigned int index ) const
		{
			map[ index >> 3 ] |= ( 1 << ( index & 0x7 ) );
		}

		void frag_accum_recover( unsigned int carousel_id, block_t &block );
		void frag_accum_reset( block_t &block ) const;

		void process_block_1( unsigned int carousel_id, const unsigned char *block, unsigned int length );
		void process_block_2( unsigned int carousel_id, const unsigned char *block );
		void process_block( unsigned int carousel_id, unsigned char block_type, const unsigned char *data, unsigned int length );
		void process_ident( unsigned int carousel_id, const unsigned char *block_data, unsigned int block_length );
		void process_signaling( unsigned int carousel_id, const unsigned char *block_data, unsigned int block_length );
		void process_file_meta( unsigned int carousel_id, const unsigned char *block_data, unsigned int block_length );
		void process_data( unsigned int carousel_id, const unsigned char *block_data, unsigned int block_length );
		void process_fec( unsigned int carousel_id, const unsigned char *block_data, unsigned int block_length );
		bool load_transfer( transfer_t &transfer, unsigned int transfer_id );
		void open_transfer( transfer_t &transfer, const file_t &file ) const;
		void flush_transfer( transfer_t &transfer );
		void close_transfer( transfer_t &transfer ) const;
		void complete_transfer( transfer_t &transfer );
		bool read_block( transfer_t &transfer, off_t offset, unsigned char *data, unsigned int size ) const;
		bool write_block( transfer_t &transfer, off_t offset, const unsigned char *data, unsigned int size ) const;

		typedef std::list< unsigned short > matrix_row_t;
		typedef std::vector< matrix_row_t > matrix_t;
		inline bool matrix_has_entry( const matrix_t &m, int row, int col ) const
		{
			for( matrix_row_t::const_iterator iter = m[ row ].begin(); iter != m[ row ].end(); iter++ )
			{
				if( *iter == col )
					return true;
			}
			return false;
		}
		inline int matrix_row_degree( const matrix_t &m, int row ) const
		{
			return m[ row ].size();
		}
		inline void matrix_insert_entry( matrix_t &m, int row, int col ) const
		{
			m[ row ].push_back( col );
		}
		void fec_init_matrix( matrix_t &m, const fec_params_t &params ) const;
		void fec_do_xor( void *dst, const void *src, unsigned int length ) const;
};

//-----------------------------------------------------------------------------
#endif
