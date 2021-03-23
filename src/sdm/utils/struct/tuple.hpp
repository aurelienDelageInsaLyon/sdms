// <tuple> -*- C++ -*-

// Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file include/tuple
  *  This is a Standard C++ Library header.
  */

#pragma once
#include <cstdlib>
#include <sstream>

using namespace std;
#include <utility>

namespace sdm
{
    // Adds a const reference to a non-reference type.
    template <typename _Tp>
    struct __add_c_ref
    {
        typedef const _Tp &type;
    };

    template <typename _Tp>
    struct __add_c_ref<_Tp &>
    {
        typedef _Tp &type;
    };

    // Adds a reference to a non-reference type.
    template <typename _Tp>
    struct __add_ref
    {
        typedef _Tp &type;
    };

    template <typename _Tp>
    struct __add_ref<_Tp &>
    {
        typedef _Tp &type;
    };

    template <std::size_t _Idx, typename _Head, bool _IsEmpty>
    struct _Head_base;

    template <std::size_t _Idx, typename _Head>
    struct _Head_base<_Idx, _Head, true>
        : public _Head
    {
        _Head_base()
            : _Head() {}

        _Head_base(const _Head &__h)
            : _Head(__h) {}

        template <typename _UHead>
        _Head_base(_UHead &&__h)
            : _Head(std::forward<_UHead>(__h)) {}

        _Head &_M_head() { return *this; }
        const _Head &_M_head() const { return *this; }

        void _M_swap_impl(_Head &)
        { /* no-op */
        }
    };

    template <std::size_t _Idx, typename _Head>
    struct _Head_base<_Idx, _Head, false>
    {
        _Head_base()
            : _M_head_impl() {}

        _Head_base(const _Head &__h)
            : _M_head_impl(__h) {}

        template <typename _UHead>
        _Head_base(_UHead &&__h)
            : _M_head_impl(std::forward<_UHead>(__h)) {}

        _Head &_M_head() { return _M_head_impl; }
        const _Head &_M_head() const { return _M_head_impl; }

        void
        _M_swap_impl(_Head &__h)
        {
            using std::swap;
            swap(__h, _M_head_impl);
        }

        _Head _M_head_impl;
    };

    /**
    * Contains the actual implementation of the @c tuple template, stored
    * as a recursive inheritance hierarchy from the first element (most
    * derived class) to the last (least derived class). The @c Idx
    * parameter gives the 0-based index of the element stored at this
    * point in the hierarchy; we use it to implement a constant-time
    * get() operation.
    */
    template <std::size_t _Idx, typename... _Elements>
    struct _Tuple_impl;

    /**
    * Zero-element tuple implementation. This is the basis case for the 
    * inheritance recursion.
    */
    template <std::size_t _Idx>
    struct _Tuple_impl<_Idx>
    {
    protected:
        void _M_swap_impl(_Tuple_impl &)
        { /* no-op */
        }
    };

    /**
    * Recursive tuple implementation. Here we store the @c Head element
    * and derive from a @c Tuple_impl containing the remaining elements
    * (which contains the @c Tail).
    */
    template <std::size_t _Idx, typename _Head, typename... _Tail>
    struct _Tuple_impl<_Idx, _Head, _Tail...>
        : public _Tuple_impl<_Idx + 1, _Tail...>,
          private _Head_base<_Idx, _Head, std::is_empty<_Head>::value>
    {
        typedef _Tuple_impl<_Idx + 1, _Tail...> _Inherited;
        typedef _Head_base<_Idx, _Head, std::is_empty<_Head>::value> _Base;

        _Head &_M_head() { return _Base::_M_head(); }
        const _Head &_M_head() const { return _Base::_M_head(); }

        _Inherited &_M_tail() { return *this; }
        const _Inherited &_M_tail() const { return *this; }

        _Tuple_impl()
            : _Inherited(), _Base() {}

        explicit _Tuple_impl(const _Head &__head, const _Tail &...__tail)
            : _Inherited(__tail...), _Base(__head) {}

        template <typename _UHead, typename... _UTail>
        explicit _Tuple_impl(_UHead &&__head, _UTail &&...__tail)
            : _Inherited(std::forward<_UTail>(__tail)...),
              _Base(std::forward<_UHead>(__head)) {}

        _Tuple_impl(const _Tuple_impl &__in)
            : _Inherited(__in._M_tail()), _Base(__in._M_head()) {}

        _Tuple_impl(_Tuple_impl &&__in)
            : _Inherited(std::move(__in._M_tail())),
              _Base(std::forward<_Head>(__in._M_head())) {}

        template <typename... _UElements>
        _Tuple_impl(const _Tuple_impl<_Idx, _UElements...> &__in)
            : _Inherited(__in._M_tail()), _Base(__in._M_head()) {}

        template <typename... _UElements>
        _Tuple_impl(_Tuple_impl<_Idx, _UElements...> &&__in)
            : _Inherited(std::move(__in._M_tail())),
              _Base(std::move(__in._M_head())) {}

        _Tuple_impl &
        operator=(const _Tuple_impl &__in)
        {
            _M_head() = __in._M_head();
            _M_tail() = __in._M_tail();
            return *this;
        }

        _Tuple_impl &
        operator=(_Tuple_impl &&__in)
        {
            _M_head() = std::move(__in._M_head());
            _M_tail() = std::move(__in._M_tail());
            return *this;
        }

        template <typename... _UElements>
        _Tuple_impl &
        operator=(const _Tuple_impl<_Idx, _UElements...> &__in)
        {
            _M_head() = __in._M_head();
            _M_tail() = __in._M_tail();
            return *this;
        }

        template <typename... _UElements>
        _Tuple_impl &
        operator=(_Tuple_impl<_Idx, _UElements...> &&__in)
        {
            _M_head() = std::move(__in._M_head());
            _M_tail() = std::move(__in._M_tail());
            return *this;
        }

    protected:
        void
        _M_swap_impl(_Tuple_impl &__in)
        {
            _Base::_M_swap_impl(__in._M_head());
            _Inherited::_M_swap_impl(__in._M_tail());
        }
    };

    /// tuple
    template <typename... _Elements>
    class Tuple : public _Tuple_impl<0, _Elements...>
    {
        typedef _Tuple_impl<0, _Elements...> _Inherited;

    public:
        Tuple()
            : _Inherited() {}

        explicit Tuple(const _Elements &...__elements)
            : _Inherited(__elements...) {}

        template <typename... _UElements>
        explicit Tuple(_UElements &&...__elements)
            : _Inherited(std::forward<_UElements>(__elements)...) {}

        Tuple(const Tuple &__in)
            : _Inherited(static_cast<const _Inherited &>(__in)) {}

        Tuple(Tuple &&__in)
            : _Inherited(static_cast<_Inherited &&>(__in)) {}

        template <typename... _UElements>
        Tuple(const Tuple<_UElements...> &__in)
            : _Inherited(static_cast<const _Tuple_impl<0, _UElements...> &>(__in))
        {
        }

        template <typename... _UElements>
        Tuple(Tuple<_UElements...> &&__in)
            : _Inherited(static_cast<_Tuple_impl<0, _UElements...> &&>(__in)) {}

        // XXX http://gcc.gnu.org/ml/libstdc++/2008-02/msg00047.html
        template <typename... _UElements>
        Tuple(Tuple<_UElements...> &__in)
            : _Inherited(static_cast<const _Tuple_impl<0, _UElements...> &>(__in))
        {
        }

        Tuple &
        operator=(const Tuple &__in)
        {
            static_cast<_Inherited &>(*this) = __in;
            return *this;
        }

        Tuple &
        operator=(Tuple &&__in)
        {
            static_cast<_Inherited &>(*this) = std::move(__in);
            return *this;
        }

        template <typename... _UElements>
        Tuple &
        operator=(const Tuple<_UElements...> &__in)
        {
            static_cast<_Inherited &>(*this) = __in;
            return *this;
        }

        template <typename... _UElements>
        Tuple &
        operator=(Tuple<_UElements...> &&__in)
        {
            static_cast<_Inherited &>(*this) = std::move(__in);
            return *this;
        }

        void
        swap(Tuple &__in)
        {
            _Inherited::_M_swap_impl(__in);
        }

        std::string str() const
        {
            std::ostringstream res;
            res << "Tuple{";
            res << get<0>(*this) << " " << get<1>(*this);
            res << "}";
            return res.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const Tuple &my_tuple)
        {
            os << my_tuple.str();
            return os;
        }
    };

    template <>
    class Tuple<>
    {
    public:
        void swap(Tuple &)
        { /* no-op */
        }
    };

    /// tuple (2-element), with construction and assignment from a pair.
    template <typename _T1, typename _T2>
    class Tuple<_T1, _T2> : public _Tuple_impl<0, _T1, _T2>
    {
        typedef _Tuple_impl<0, _T1, _T2> _Inherited;

    public:
        Tuple()
            : _Inherited() {}

        explicit Tuple(const _T1 &__a1, const _T2 &__a2)
            : _Inherited(__a1, __a2) {}

        template <typename _U1, typename _U2>
        explicit Tuple(_U1 &&__a1, _U2 &&__a2)
            : _Inherited(std::forward<_U1>(__a1), std::forward<_U2>(__a2)) {}

        Tuple(const Tuple &__in)
            : _Inherited(static_cast<const _Inherited &>(__in)) {}

        Tuple(Tuple &&__in)
            : _Inherited(static_cast<_Inherited &&>(__in)) {}

        template <typename _U1, typename _U2>
        Tuple(const Tuple<_U1, _U2> &__in)
            : _Inherited(static_cast<const _Tuple_impl<0, _U1, _U2> &>(__in)) {}

        template <typename _U1, typename _U2>
        Tuple(Tuple<_U1, _U2> &&__in)
            : _Inherited(static_cast<_Tuple_impl<0, _U1, _U2> &&>(__in)) {}

        template <typename _U1, typename _U2>
        Tuple(const pair<_U1, _U2> &__in)
            : _Inherited(__in.first, __in.second) {}

        template <typename _U1, typename _U2>
        Tuple(pair<_U1, _U2> &&__in)
            : _Inherited(std::forward<_U1>(__in.first),
                         std::forward<_U2>(__in.second)) {}

        Tuple &
        operator=(const Tuple &__in)
        {
            static_cast<_Inherited &>(*this) = __in;
            return *this;
        }

        Tuple &
        operator=(Tuple &&__in)
        {
            static_cast<_Inherited &>(*this) = std::move(__in);
            return *this;
        }

        template <typename _U1, typename _U2>
        Tuple &
        operator=(const Tuple<_U1, _U2> &__in)
        {
            static_cast<_Inherited &>(*this) = __in;
            return *this;
        }

        template <typename _U1, typename _U2>
        Tuple &
        operator=(Tuple<_U1, _U2> &&__in)
        {
            static_cast<_Inherited &>(*this) = std::move(__in);
            return *this;
        }

        template <typename _U1, typename _U2>
        Tuple &
        operator=(const pair<_U1, _U2> &__in)
        {
            this->_M_head() = __in.first;
            this->_M_tail()._M_head() = __in.second;
            return *this;
        }

        template <typename _U1, typename _U2>
        Tuple &
        operator=(pair<_U1, _U2> &&__in)
        {
            this->_M_head() = std::move(__in.first);
            this->_M_tail()._M_head() = std::move(__in.second);
            return *this;
        }

        void
        swap(Tuple &__in)
        {
            using std::swap;
            swap(this->_M_head(), __in._M_head());
            swap(this->_M_tail()._M_head(), __in._M_tail()._M_head());
        }

        std::string str() const
        {
            std::ostringstream res;
            res << "Tuple{";
            res << get<0>(*this) << ", " << get<1>(*this);
            res << " }";
            return res.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const Tuple &my_tuple)
        {
            os << my_tuple.str();
            return os;
        }
    };

    /// Gives the type of the ith element of a given tuple type.
    template <std::size_t __i, typename _Tp>
    struct tuple_element;

    /**
    * Recursive case for tuple_element: strip off the first element in
    * the tuple and retrieve the (i-1)th element of the remaining tuple.
    */
    template <std::size_t __i, typename _Head, typename... _Tail>
    struct tuple_element<__i, Tuple<_Head, _Tail...>>
        : tuple_element<__i - 1, Tuple<_Tail...>>
    {
    };

    /**
    * Basis case for tuple_element: The first element is the one we're seeking.
    */
    template <typename _Head, typename... _Tail>
    struct tuple_element<0, Tuple<_Head, _Tail...>>
    {
        typedef _Head type;
    };

    /// Finds the size of a given tuple type.
    template <typename _Tp>
    struct tuple_size;

    /// class tuple_size
    template <typename... _Elements>
    struct tuple_size<Tuple<_Elements...>>
    {
        static const std::size_t value = sizeof...(_Elements);
    };

    template <typename... _Elements>
    const std::size_t tuple_size<Tuple<_Elements...>>::value;

    template <std::size_t __i, typename _Head, typename... _Tail>
    inline typename __add_ref<_Head>::type
    __get_helper(_Tuple_impl<__i, _Head, _Tail...> &__t)
    {
        return __t._M_head();
    }

    template <std::size_t __i, typename _Head, typename... _Tail>
    inline typename __add_c_ref<_Head>::type
    __get_helper(const _Tuple_impl<__i, _Head, _Tail...> &__t)
    {
        return __t._M_head();
    }

    // Return a reference (const reference) to the ith element of a tuple.
    // Any const or non-const ref elements are returned with their original type.
    template <std::size_t __i, typename... _Elements>
    inline typename __add_ref<
        typename tuple_element<__i, Tuple<_Elements...>>::type>::type
    get(Tuple<_Elements...> &__t)
    {
        return __get_helper<__i>(__t);
    }

    template <std::size_t __i, typename... _Elements>
    inline typename __add_c_ref<
        typename tuple_element<__i, Tuple<_Elements...>>::type>::type
    get(const Tuple<_Elements...> &__t)
    {
        return __get_helper<__i>(__t);
    }

    // This class helps construct the various comparison operations on tuples
    template <std::size_t __check_equal_size, std::size_t __i, std::size_t __j,
              typename _Tp, typename _Up>
    struct __tuple_compare;

    template <std::size_t __i, std::size_t __j, typename _Tp, typename _Up>
    struct __tuple_compare<0, __i, __j, _Tp, _Up>
    {
        static bool __eq(const _Tp &__t, const _Up &__u)
        {
            return (get<__i>(__t) == get<__i>(__u) &&
                    __tuple_compare<0, __i + 1, __j, _Tp, _Up>::__eq(__t, __u));
        }

        static bool __less(const _Tp &__t, const _Up &__u)
        {
            return ((get<__i>(__t) < get<__i>(__u)) || !(get<__i>(__u) < get<__i>(__t)) &&
                                                           __tuple_compare<0, __i + 1, __j, _Tp, _Up>::__less(__t, __u));
        }
    };

    template <std::size_t __i, typename _Tp, typename _Up>
    struct __tuple_compare<0, __i, __i, _Tp, _Up>
    {
        static bool __eq(const _Tp &, const _Up &)
        {
            return true;
        }

        static bool __less(const _Tp &, const _Up &)
        {
            return false;
        }
    };

    template <typename... _TElements, typename... _UElements>
    bool
    operator==(const Tuple<_TElements...> &__t,
               const Tuple<_UElements...> &__u)
    {
        typedef Tuple<_TElements...> _Tp;
        typedef Tuple<_UElements...> _Up;
        return (__tuple_compare<tuple_size<_Tp>::value - tuple_size<_Up>::value,
                                0, tuple_size<_Tp>::value, _Tp, _Up>::__eq(__t, __u));
    }

    template <typename... _TElements, typename... _UElements>
    bool
    operator<(const Tuple<_TElements...> &__t,
              const Tuple<_UElements...> &__u)
    {
        typedef Tuple<_TElements...> _Tp;
        typedef Tuple<_UElements...> _Up;
        return (__tuple_compare<tuple_size<_Tp>::value - tuple_size<_Up>::value,
                                0, tuple_size<_Tp>::value, _Tp, _Up>::__less(__t, __u));
    }

    template <typename... _TElements, typename... _UElements>
    inline bool
    operator!=(const Tuple<_TElements...> &__t,
               const Tuple<_UElements...> &__u)
    {
        return !(__t == __u);
    }

    template <typename... _TElements, typename... _UElements>
    inline bool
    operator>(const Tuple<_TElements...> &__t,
              const Tuple<_UElements...> &__u)
    {
        return __u < __t;
    }

    template <typename... _TElements, typename... _UElements>
    inline bool
    operator<=(const Tuple<_TElements...> &__t,
               const Tuple<_UElements...> &__u)
    {
        return !(__u < __t);
    }

    template <typename... _TElements, typename... _UElements>
    inline bool
    operator>=(const Tuple<_TElements...> &__t,
               const Tuple<_UElements...> &__u)
    {
        return !(__t < __u);
    }

    // NB: DR 705.
    template <typename... _Elements>
    inline Tuple<typename __decay_and_strip<_Elements>::__type...>
    make_tuple(_Elements &&...__args)
    {
        typedef Tuple<typename __decay_and_strip<_Elements>::__type...>
            __result_type;
        return __result_type(std::forward<_Elements>(__args)...);
    }

    template <std::size_t...>
    struct __index_holder
    {
    };

    template <std::size_t __i, typename _IdxHolder, typename... _Elements>
    struct __index_holder_impl;

    template <std::size_t __i, std::size_t... _Indexes, typename _IdxHolder,
              typename... _Elements>
    struct __index_holder_impl<__i, __index_holder<_Indexes...>,
                               _IdxHolder, _Elements...>
    {
        typedef typename __index_holder_impl<__i + 1,
                                             __index_holder<_Indexes..., __i>,
                                             _Elements...>::type type;
    };

    template <std::size_t __i, std::size_t... _Indexes>
    struct __index_holder_impl<__i, __index_holder<_Indexes...>>
    {
        typedef __index_holder<_Indexes...> type;
    };

    template <typename... _Elements>
    struct __make_index_holder
        : __index_holder_impl<0, __index_holder<>, _Elements...>
    {
    };

    template <typename... _TElements, std::size_t... _TIdx,
              typename... _UElements, std::size_t... _UIdx>
    inline Tuple<_TElements..., _UElements...>
    __tuple_cat_helper(const Tuple<_TElements...> &__t,
                       const __index_holder<_TIdx...> &,
                       const Tuple<_UElements...> &__u,
                       const __index_holder<_UIdx...> &)
    {
        return Tuple<_TElements..., _UElements...>(get<_TIdx>(__t)...,
                                                   get<_UIdx>(__u)...);
    }

    template <typename... _TElements, std::size_t... _TIdx,
              typename... _UElements, std::size_t... _UIdx>
    inline Tuple<_TElements..., _UElements...>
    __tuple_cat_helper(tuple<_TElements...> &&__t,
                       const __index_holder<_TIdx...> &,
                       const Tuple<_UElements...> &__u,
                       const __index_holder<_UIdx...> &)
    {
        return Tuple<_TElements..., _UElements...>(std::move(get<_TIdx>(__t))..., get<_UIdx>(__u)...);
    }

    template <typename... _TElements, std::size_t... _TIdx,
              typename... _UElements, std::size_t... _UIdx>
    inline Tuple<_TElements..., _UElements...>
    __tuple_cat_helper(const Tuple<_TElements...> &__t,
                       const __index_holder<_TIdx...> &,
                       Tuple<_UElements...> &&__u,
                       const __index_holder<_UIdx...> &)
    {
        return Tuple<_TElements..., _UElements...>(get<_TIdx>(__t)..., std::move(get<_UIdx>(__u))...);
    }

    template <typename... _TElements, std::size_t... _TIdx,
              typename... _UElements, std::size_t... _UIdx>
    inline Tuple<_TElements..., _UElements...>
    __tuple_cat_helper(Tuple<_TElements...> &&__t,
                       const __index_holder<_TIdx...> &,
                       Tuple<_UElements...> &&__u,
                       const __index_holder<_UIdx...> &)
    {
        return Tuple<_TElements..., _UElements...>(std::move(get<_TIdx>(__t))..., std::move(get<_UIdx>(__u))...);
    }

    template <typename... _TElements, typename... _UElements>
    inline Tuple<_TElements..., _UElements...>
    tuple_cat(const Tuple<_TElements...> &__t, const Tuple<_UElements...> &__u)
    {
        return __tuple_cat_helper(__t, typename __make_index_holder<_TElements...>::type(),
                                  __u, typename __make_index_holder<_UElements...>::type());
    }

    template <typename... _TElements, typename... _UElements>
    inline Tuple<_TElements..., _UElements...>
    tuple_cat(Tuple<_TElements...> &&__t, const Tuple<_UElements...> &__u)
    {
        return __tuple_cat_helper(std::move(__t), typename __make_index_holder<_TElements...>::type(),
                                  __u, typename __make_index_holder<_UElements...>::type());
    }

    template <typename... _TElements, typename... _UElements>
    inline Tuple<_TElements..., _UElements...>
    tuple_cat(const Tuple<_TElements...> &__t, Tuple<_UElements...> &&__u)
    {
        return __tuple_cat_helper(__t, typename __make_index_holder<_TElements...>::type(),
                                  std::move(__u), typename __make_index_holder<_UElements...>::type());
    }

    template <typename... _TElements, typename... _UElements>
    inline Tuple<_TElements..., _UElements...>
    tuple_cat(Tuple<_TElements...> &&__t, Tuple<_UElements...> &&__u)
    {
        return __tuple_cat_helper(std::move(__t), typename __make_index_holder<_TElements...>::type(),
                                  std::move(__u), typename __make_index_holder<_UElements...>::type());
    }

    template <typename... _Elements>
    inline Tuple<_Elements &...>
    tie(_Elements &...__args)
    {
        return Tuple<_Elements &...>(__args...);
    }

    template <typename... _Elements>
    inline void
    swap(Tuple<_Elements...> &__x, Tuple<_Elements...> &__y)
    {
        __x.swap(__y);
    }

    // A class (and instance) which can be used in 'tie' when an element
    // of a tuple is not required
    struct _Swallow_assign
    {
        template <class _Tp>
        _Swallow_assign &
        operator=(const _Tp &)
        {
            return *this;
        }
    };

    // TODO: Put this in some kind of shared file.
    namespace
    {
        _Swallow_assign ignore;
    }; // anonymous namespace
}
namespace std{
    namespace
    {

        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     http://stackoverflow.com/questions/4948780

        template <class T>
        inline void hash_combine(std::size_t& seed, T const& v)
        {
            seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }

        // Recursive template code derived from Matthieu M.
        template <class Tuple, size_t Index = sdm::tuple_size<Tuple>::value - 1>
        struct HashValueImpl
        {
          static void apply(size_t& seed, Tuple const& tuple)
          {
            HashValueImpl<Tuple, Index-1>::apply(seed, tuple);
            sdm::hash_combine(seed, sdm::get<Index>(tuple));
          }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple,0>
        {
          static void apply(size_t& seed, Tuple const& tuple)
          {
            sdm::hash_combine(seed, sdm::get<0>(tuple));
          }
        };
    }

    template <typename ... TT>
    struct hash<sdm::Tuple<TT...>> 
    {
        size_t
        operator()(sdm::Tuple<TT...> const& tt) const
        {                                              
            size_t seed = 0;                             
            HashValueImpl<sdm::Tuple<TT...> >::apply(seed, tt);    
            return seed;                                 
        }                                              

    };
}