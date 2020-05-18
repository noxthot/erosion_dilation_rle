/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Mar 24, 2009
 * @par    Copyright: &copy; Datacon 2008-2009
 *              all rights reserved
 *
 * $Id: gapiterator.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Iteratoradapter für einen Iterator mit Gap
 *
 ********************************************************************/
#ifndef IPL_GAPITERATOR_HH
#define IPL_GAPITERATOR_HH

#include "ipl/config.hh"

#include <boost/concept_check.hpp>

#include "ipl/validable.hh"

IPL_NS_BEGIN

///@cond developer_docu

//! Ein Iteratoradapter, dessen Inkrementierung um ein fixes Gap avanciert.
/*! Dieser Iterator adaptiert den zugrunde liegenden Iterator @a RanIt so, dass
 * ein Increment/Decrement @a RanIt um das Gap erhöht oder erniedrigt.
 * Dieser Iterator dient vor allem dazu, um über eine Spalte eine Bildes zu
 * iterieren; so liefert etwa PictImage::colRange genau so ein Iteratorpaar.
 * @tparam RanIt der zugrundeliegende <tt>random access iterator</tt>
 * @sa PictImage::colRange
 */
template<typename RanIt>
class GapIterator
: public std::iterator<typename std::iterator_traits<RanIt>::iterator_category,
                       typename std::iterator_traits<RanIt>::value_type,
                       typename std::iterator_traits<RanIt>::difference_type,
                       typename std::iterator_traits<RanIt>::pointer,
                       typename std::iterator_traits<RanIt>::reference>
{
public:
    //! typedefs für die Iterator-Traits
    //@{
    typedef RanIt                                                 iterator_type;
    typedef typename std::iterator_traits<RanIt>::value_type      value_type;
    typedef typename std::iterator_traits<RanIt>::difference_type difference_type;
    typedef typename std::iterator_traits<RanIt>::reference       reference;
    typedef typename std::iterator_traits<RanIt>::pointer         pointer;
    //@}
    //! Concept Check, brauche hier einen random acess iterator
    BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<RanIt>));

    //! default Ctr
    GapIterator() :
        current_(),
        gap_(1)
        {}
    //! Ctr aus dem ursprünglichen Iterator
    /*! Erlaubt ist nur ein positives Gap, @a gap>0
     */
    GapIterator(iterator_type it, difference_type gap) :
        current_(it),
        gap_(gap)
        {
            IPL_ASSERT(gap > 0);
        }
    //! Cpy-Ctr
    GapIterator(GapIterator const & rhs) :
        current_(rhs.current_),
        gap_(rhs.gap_)
        {}
    //! Dereferenzierung
    reference
    operator*() const {
        return *current_;
    }
    //! Dereferenzierung
    pointer
    operator->() const {
        return &(operator*());
    }
    //! Prefix-Increment
    GapIterator &
    operator++() {
        current_ += gap_;
        return *this;
    }
    //! Postfix-Increment
    GapIterator
    operator++(int) {
        GapIterator tmp(*this);
        ++*this;
        return tmp;
    }
    //! Prefix-Decrement
    GapIterator &
    operator--() {
        current_ -= gap_;
        return *this;
    }
    //! Postfix-Decrement
    GapIterator
    operator--(int) {
        GapIterator tmp(*this);
        --*this;
        return tmp;
    }
    //! Advance
    GapIterator
    operator+(difference_type n) const {
        return GapIterator(current_ + n*gap_, gap_);
    }
    //! Advance
    GapIterator &
    operator+=(difference_type n) {
        current_ += n*gap_;
        return *this;
    }
    //! Advance
    GapIterator
    operator-(difference_type n) const {
        return GapIterator(current_ - n*gap_, gap_);
    }
    //! Advance
    GapIterator &
    operator-=(difference_type n) {
        current_ -= n*gap_;
        return *this;
    }
    //! Elementzugriff
    reference
    operator[](difference_type n) const {
        return *(*this + n);
    }
    //! Distance
    difference_type
    operator-(GapIterator const & rhs) {
        IPL_ASSERT(gap_ == rhs.gap_);
        return (current_ - rhs.current_) / gap_;
    }
    //! Gleichheit
    bool
    operator==(GapIterator const & rhs) {
        IPL_ASSERT(gap_ == rhs.gap_);
        return current_ == rhs.current_;
    }
    //! Ungleichheit
    bool
    operator!=(GapIterator const & rhs) {
        return !(*this == rhs);
    }
    //! Vergleich
    bool
    operator<(GapIterator const & rhs) {
        IPL_ASSERT(gap_ > 0);
        return current_ < rhs.current_;
    }
private:
    //! Der zugrunde liegende Iterator
    iterator_type current_;
    //! Gap, um das weitergesprungen wird
    difference_type gap_;
};

///@endcond

IPL_NS_END

#endif // IPL_GAPITERATOR_HH
