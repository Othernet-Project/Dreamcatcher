#ifndef __POINTER_H
#define __POINTER_H
// ----------------------------------------------------------------------------
// pointer.h
//	
// ----------------------------------------------------------------------------

namespace wpt {
// ----------------------------------------------------------------------------
// heap_ptr
// ----------------------------------------------------------------------------
template< typename _Ty >
class heap_ptr
{
	public:
		_Ty *&ptr;
		explicit heap_ptr( _Ty *p = 0 ) : ptr( m_p ), m_p( p ) { }
		heap_ptr( heap_ptr &p ) : ptr( m_p ), m_p( 0 ) { reset(); m_p = p.m_p; p.m_p = 0; }
		~heap_ptr() { reset(); }
		void operator=( _Ty *p ) { reset(); m_p = p; }
		void operator=( _Ty &p ) { reset(); m_p = p.m_p; p.m_p = 0; }
		bool operator==( _Ty *p ) const { return m_p == p; }
		operator _Ty *() { return m_p; }
		operator const _Ty *() const { return m_p; }

	private:
		void reset() { if( 0 != m_p ) free( m_p ); m_p = 0; }
		_Ty *m_p;
};

// ----------------------------------------------------------------------------
// ref_ptr
// ----------------------------------------------------------------------------
class ref
{
	public:
		ref() : ref_count( 0 ) { }

	private:
		template< class _Tx >
		friend class ref_ptr;
		int ref_count;
};

template< class _Tx >
class ref_ptr
{
	public:
		ref_ptr( _Tx *p = 0 ) : ptr( m_ptr ), m_ptr( 0 ) { attach( p ); }
		ref_ptr( const ref_ptr &p ) : ptr( m_ptr ), m_ptr( 0 ) { attach( p ); }
		~ref_ptr() { detach(); }
		ref_ptr &operator=( const ref_ptr &p )
		{
			attach( p );
			return (*this);
		}
		ref_ptr &operator=( _Tx *p )
		{
			attach( p );
			return (*this);
		}
		bool operator==( const ref_ptr &p ) const { return m_ptr == p.m_ptr; }
		operator bool() const { return ( 0 != m_ptr ); }
		operator _Tx *() { return m_ptr; }

		_Tx &operator*() { return *m_ptr; }
		_Tx *operator->() { return m_ptr; }
		const _Tx &operator*() const { return *m_ptr; }
		const _Tx *operator->() const { return m_ptr; }

		_Tx *&ptr;

	private:
		_Tx *m_ptr;
		
		void detach()
		{
			if( !m_ptr ) return;
			m_ptr->ref_count--;
			if( 0 == m_ptr->ref_count )
				delete m_ptr;
			m_ptr = 0;
		}

		void attach( _Tx *p )
		{
			detach();
			if( !p ) return;

			m_ptr = p;
			m_ptr->ref_count++;
		}

		void attach( const ref_ptr &p )
		{
			detach();
			if( !p.m_ptr ) return;
			m_ptr = p.m_ptr;
			m_ptr->ref_count++;
		}
};

// ----------------------------------------------------------------------------
}
#endif
