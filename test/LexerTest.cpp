/** \file LexerTest.cpp
 * Lexer test module
 *
 * \author Filip Smola
 */
#define BOOST_TEST_MODULE "LexerModule"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( tautology ) {
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE( contradiction ) {
    BOOST_CHECK(false);
}
