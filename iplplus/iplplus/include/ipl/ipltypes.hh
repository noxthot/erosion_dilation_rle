/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: ipltypes.hh 15 2012-09-05 12:57:52Z fesc $
 *
 * @brief  Typedefs for intrinsic types
 *
 ********************************************************************/

#ifndef IPL_IPLTYPES_HH
#define IPL_IPLTYPES_HH

#include "ipl/config.hh"

/*! @defgroup IplTypes typedefs for intrinsic types
  Use this typedefs for all native types

  @internal Extend this list if you really need more types.
*/
//@{
typedef char  N8;               ///< iNt 8-bit signed
typedef short N16;              ///< iNt 16-bit signed
typedef int   N32;              ///< iNt 32-bit signed
typedef long long N64;          ///< signed iNt 64-bit

typedef double F64;             ///< Float 64-bit, double precision

typedef unsigned char  UN8;     ///< Unsigned iNt 8-bit
typedef unsigned short UN16;    ///< Unsigned iNt 16-bit
typedef unsigned int   UN32;    ///< Unsigned iNt 32-bit
typedef unsigned long long UN64; ///< Unsigned iNt 64-bit
//@}

IPL_NS_BEGIN

//! Object Color
/*! Often we need information which object we are looking for in a grayvalue image.
 * This enum defines the constants for this color.
 */
struct ObjectColor {
    //! Enum Tor the color of an object in the image
    enum Color {
        //! The object is dark
        Dark      = 1 << 0,
        //! The object is bright
        Bright    = 1 << 1,
        //! Don't know
        Arbitrary = Dark | Bright,
    };
    //! Textual description of the Color @a c
    static char const * toString(Color c);
    //! Validate the enum.
    /*! Mostly used when constant is passed from a serialization process.
     */
    static bool isValid(Color c) {
        return c >= Dark && c <= Arbitrary;
    }
};

//! Accuracy mode
struct AccuracyMode {
    //! Enum for the modes.
    enum Mode {
        //! Normal, a compromise between @a Fast and @a Accurate.
        Normal = 0,
        //! Be fast but less accurate.
        Fast = 1,
        //! Be accurate but maybe slower.
        Accurate = 2,
    };
    //! Textual description of the accuracy mode @a m.
    static char const * toString(Mode m);
    //! Validate the enum.
    /*! Mostly used when constant is passed from a serialization process.
     */
    static bool isValid(Mode m) {
        return m >= Normal && m <= Accurate;
    }
};

/*! @defgroup HelperTemplates Helper Templates
 * This is a collection of small helper templates, mostly used for type
 * manipulation in other templates.
 */
//@{

///@cond developer_docu
//! Template returns the name of a builtin type.
template<typename T>
struct NameType {
    //! Returns the name
    static char const * toString() { return "UNK"; }
};

//! Specialization
template<>
struct NameType<UN8> {
    //! Returns the name
    static char const * toString() { return "UN8"; }
};

//! Specialization
template<>
struct NameType<N16> {
    //! Returns the name
    static char const * toString() { return "N16"; }
};

//! Specialization
template<>
struct NameType<UN16> {
    //! Returns the name
    static char const * toString() { return "U16"; }
};

//! Specialization
template<>
struct NameType<N32> {
    //! Returns the name
    static char const * toString() { return "N32"; }
};

//! Specialization
template<>
struct NameType<UN32> {
    //! Returns the name
    static char const * toString() { return "U32"; }
};

//! Specialization
template<>
struct NameType<N64> {
    //! Returns the name
    static char const * toString() { return "N64"; }
};

//! Specialization
template<>
struct NameType<UN64> {
    //! Returns the name
    static char const * toString() { return "U64"; }
};

//! Specialization
template<>
struct NameType<F64> {
    //! Returns the name
    static char const * toString() { return "F64"; }
};
///@endcond developer_docu

//@}

IPL_NS_END

#endif //IPL_IPLTYPES_HH
