/* include/Utility */
/* Created: 230725 08:27:22 by clem@spectre */
/* Updated: 230725 08:27:22 by clem@spectre */
/* Maintainer: Clément Vidon */

#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <iosfwd>
#include <string>

/**
 * @brief       Various utils.
 */

class Utility {
 private:
  Utility( void );
  Utility( Utility const& src );
  virtual ~Utility( void );
  Utility& operator=( Utility const& rhs );

 public:
  static bool        isSocketNonBlocking(int socket);
  static void        closeFd( int& fd );
  static bool        isFdValid( int fd );
  static int         stringToInt( const std::string& str );
  static std::string intToString( int number );
  static std::string ntop( const struct sockaddr_storage& socket );
  static std::string gaiStrerror( int errorCode );
};

#endif  // UTILITY_HPP_
