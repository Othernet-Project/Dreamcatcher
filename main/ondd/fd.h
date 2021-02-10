#ifndef __FD_H
#define __FD_H
// ----------------------------------------------------------------------------
// fd.h
// ----------------------------------------------------------------------------
#include <unistd.h>

namespace wpt {
// ----------------------------------------------------------------------------
// File desc wrapper
// ----------------------------------------------------------------------------
class fd
{
	public:
		fd( int _fd = -1 ) { m_fd = _fd; }
		fd( fd &_fd ) { m_fd = _fd.m_fd; _fd.m_fd = -1; }
		~fd() { close(); }
		fd &operator=( int _fd ) { close(); m_fd = _fd; return *this; }
		fd &operator=( fd &_fd ) { close(); m_fd = _fd.m_fd; _fd.m_fd = -1; return *this; }
		bool operator==( int _fd ) const { return m_fd == _fd; }
		operator int() const { return m_fd; }
		operator bool() const { return ( -1 != m_fd ); }
		void close() { if( m_fd != -1 ) ::close( m_fd ); m_fd = -1; }
		int get() const { return m_fd; }

	private:
		int m_fd;
};

// ----------------------------------------------------------------------------
}
#endif
