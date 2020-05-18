/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: iplerr.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for all error classes
 *
 ********************************************************************/

/*! @page errhandling Error Handling
 *
 * Errors  are always  handled via  <em>exceptions</em>. @c  IPLplus  provides a
 * hierarchy  of  exception  classes  anchored  on  ipl::IplError  and  IplError
 * inherits from @c std::exception.
 *
 * All  IPLplus-functions  throw  such  an  IplError or  a  std::exception  like
 * <tt>std::bad_alloc</tt> in <em>out of memory</em> situations.
 *
 * The  library guarantees  only  a <em>basic  exception  safety</em>, i.e.  all
 * resources can be destroyed after  an exception occurred, but we can't provide
 * the guarantee that a resource is left unchanged in such a situation. This has
 * performance reasons.  Let's assume, we filter  an image inplace  and an error
 * happens in the middle of this  process. Than the image has partially changed,
 * but we  can call the destructor  without problems. Keeping such  a process in
 * safe, would require  to copy first the whole image, process  it and then copy
 * back  the processed  image to  the  originally one.  This is  simply to  much
 * effort.
 *
 * The only exception from this exception handling is the serialization. Loading
 * a object from a stream or a file is a transaction, see @ref serialization.
 *
 * @cond developer_docu
 * @section RAII The RAII-idiom
 *
 * C++   knows  the   idiom  <em><b>r</b>esource   <b>a</b>cquisition  <b>i</b>s
 * <b>i</b>nitialization</em>. This means that  the constructor acquires all the
 * required resources. A splitting of this process in two parts
 * @code
 *    SomeObj o;
 *    if (!o.init())
 *        //...
 * @endcode
 * is not allowed. The constructor must  construct the object, so that it can be
 * used without any  further function calls, or it throws  an exception, if some
 * resources can't be  acquired. This avoids that the user  has an incomplete or
 * invalid object. A method and code like
 * @code
 *     if (obj.isValid())
 *         //
 *     else
 *         //
 * @endcode
 * is unnecessary and error prune, avoid it with RAII.
 *
 * @section leaks Avoiding Memory Leaks
 *
 * Coding a constructor in the presence  of exceptions is not a trivial task, if
 * our class has member pointers. Remember,  that for a object a destructor will
 * be called only,  if the object was constructed successfully.  But this is not
 * the case, if a constructor throws an exception.
 *
 * The following code leaks, if the second @c new throws.
 * @code
 *    class B {
 *        //...
 *    };
 *    class A {
 *    public:
 *        A()
 *            : a_(new B),
 *              b_(new B)
 *        {}
 *        ~A() {
 *            delete b_;
 *            delete a_;
 *        }
 *    private:
 *        B * a_;
 *        B * b_;
 *    };
 * @endcode
 *
 * To make the  above code safe, we must catch the  exception in the constructor
 * and rethrow it after deleting the first pointer
 * @code
 *     A::A()
 *         : a_(0),
 *           b_(0)
 *     try {
 *         a_ = new B;
 *         b_ = new B;
 *     } catch (...) {
 *         delete_ a;
 *         delete_ b;
 *         throw;
 *     }
 * @endcode
 *
 * Simpler, better and more robust is the use of smart pointers
 * @code
 *     class A {
 *         public:
 *         A()
 *             : a_(new B),
 *               b_(new B)
 *         {}
 *         ~A()
 *         {}
 *     private:
 *         boost::scoped_ptr<B> a_;
 *         boost::scopde_ptr<B> b_;
 *     };
 * @endcode
 * If we need a member to hold dynamically allocated memory we should use a
 * @c std::vector or some other STL-container.
 *
 * @endcond
 */

#ifndef IPL_ERROR_HH
#define IPL_ERROR_HH

#include "ipl/config.hh"

#include <stdexcept>
#include <string>

#include "ipl/ipltypes.hh"

IPL_NS_BEGIN

//! Basis für alle Fehler der Library
/*! @note out of memory wird weiter über den Standardweg gehandelt
 */
class IplError : public std::exception
{
public:

    //! ctr mit einer Fehlerbeschreibung
    explicit IplError(std::string const & msg);

    //! dtr
    virtual ~IplError() throw();

    //! Returns a C-style character string
    /*! describing the general cause of the current error
     */
    virtual const char* what() const throw();

protected:

    //! string mit der Errormeldung
    std::string msg_;
};

//! Fehler für falsche Parameter in Funktionsaufrufen
class ParameterError : public IplError
{
public:

    //! ctr mit einer Fehlerbeschreibung
    /*! @a nr ist die Nummer des Parameters in der Aufrufliste, der
      zum Fehler führte. Typischerweise wird als @a msg der Name der
      Funktion, in der der Fehler passierte, mitgegeben.
      @code
         if (aParam < 0)
             throw ParameterError(2, IPL_FNC_NAME);
      @endcode
      @todo Doku verbessern, string sollte auch den Parameter mit falschem Wert
      erwähnen
    */
    explicit ParameterError(N32 nr,
                            std::string const & msg = std::string(""));
    //! dtr virtual
    virtual ~ParameterError() throw();
};

//! Noch nicht implementiert Funktionalität
/*! Markiert Funktionen in Entwicklung, die noch nicht oder noch nicht
  vollständig implementiert sind. Mit einer eigenen Fehlerklasse dafür lassen sich
  solche unfertigen Dinger besser markieren.
  Vor allem in den Unitstests kann
  besser darauf reagiert werden, wenn explizit die Exception gefordert wird; so
  existiert für die Funktionalität schon ein Test und er muss nur mehr ausgebaut
  werden (<em>write tests first!</em>).
 */
class NotImplementedYetError : public IplError
{
public:
    //! ctr
    explicit NotImplementedYetError(std::string const & msg);

    //! dtr
    virtual ~NotImplementedYetError() throw();
};

//! Fehler beim lesen/schreiben von Files/Streams
class IoError : public IplError
{
public:

    //! ctr mit einer Fehlerbeschreibung
    explicit IoError(std::string const & msg);

    //! dtr
    virtual ~IoError() throw();
};

//! Fehler, weil Operationen auf einem leeren Bild ausgeführt wurden
class EmptyImgError : public IplError
{
public:

    //! ctr mit einer Fehlerbeschreibung
    explicit EmptyImgError(std::string const & msg);

    //! dtr
    virtual ~EmptyImgError() throw();
};

//! Fehler, weil Operationen auf einer leeren Region ausgeführt wurden.
/*! Diese Fehlerklasse wird geworfen wenn auf einer leeren Region eine
    Operation ausgeführt wird, die dort nicht definiert ist, wie etwa
    Schwerpunkts- oder Momentenberechnung
 */
class EmptyRegionError: public IplError
{
public:

    //! ctr mit einer Fehlerbeschreibung
    explicit EmptyRegionError(std::string const & msg);

    //! dtr
    virtual ~EmptyRegionError() throw();
};



//! Fehler in einem Processing Schritt
/*! Diese Fehlerklasse wird geworfen, wenn eine Berechnung nicht durchgeführt
    werden kann (Division durch 0, negative Quadratwurzel etc.)
 */
class ProcessingError: public IplError
{
public:

    //! ctr mit einer Fehlerbeschreibung
    explicit ProcessingError(std::string const & msg);

    //! dtr
    virtual ~ProcessingError() throw();
};

IPL_NS_END

#endif
