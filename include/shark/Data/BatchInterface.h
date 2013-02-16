/**
*
*  \brief Defines the Batch Interface for a type, e.g., for every type a container with optimal structure.
*
*  \author O.Krause
*  \date 2012
*
*
*  <BR><HR>
*  This file is part of Shark. This library is free software;
*  you can redistribute it and/or modify it under the terms of the
*  GNU General Public License as published by the Free Software
*  Foundation; either version 3, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this library; if not, see <http://www.gnu.org/licenses/>.
*
*/
#ifndef SHARK_DATA_BATCHINTERFACE_H
#define SHARK_DATA_BATCHINTERFACE_H

#include <shark/LinAlg/Base.h>
#include <shark/Core/utility/Iterators.h>

#include <boost/preprocessor.hpp>

#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/mpl/if.hpp>

namespace shark{

namespace detail{

/// \brief default implementation of the Batch which maps it's type on std::vector<T>
template<class T>
struct DefaultBatch{
	/// \brief Type of a single element.
	typedef T& reference;
	/// \brief Type of a single immutable element.
	typedef T const& const_reference;
	/// \brief Type of a batch of elements.
	typedef std::vector<T> type;
	
	/// \brief The type of the elements stored in the batch 
	typedef T value_type;
	
	/// \brief the iterator type of the object
	typedef typename type::iterator iterator;
	/// \brief the const_iterator type of the object
	typedef typename type::const_iterator const_iterator;
	
	
	///\brief creates a batch able to store elements of the structure of input (e.g. same dimensionality)
	static type createBatch(T const& input, std::size_t size = 1){
		return type(size,input);
	}
	
	///\brief creates a batch storing the elements referenced by the provided range
	template<class Range>
	static type createBatchFromRange(Range const& range){
		return type(range.begin(),range.end());
	}
	
	static void resize(type& batch, std::size_t batchSize, std::size_t elements){
		batch.resize(batchSize);
	}
};

/// \brief default implementation of the Batch for arithmetic types, which are mapped on boost::numeric::ublas::vector<T>
template<class T>
struct ArithmeticBatch{
	/// \brief Type of a single element.
	typedef T& reference;
	/// \brief Type of a single immutable element.
	typedef T const& const_reference;
	/// \brief Type of a batch of elements.
	typedef boost::numeric::ublas::vector<T> type;
	
	/// \brief The type of the elements stored in the batch 
	typedef T value_type;
	
	/// \brief the iterator type of the object
	typedef typename type::iterator iterator;
	/// \brief the const_iterator type of the object
	typedef typename type::const_iterator const_iterator;
	
	
	///\brief creates a batch which can storne size numbers of type T
	static type createBatch(T const& input, std::size_t size = 1){
		return type(size);
	}
	///\brief creates a batch storing the elements referenced by the provided range
	template<class Range>
	static type createBatchFromRange(Range const& range){
		type batch(range.size());
		std::copy(range.begin(),range.end(),batch.begin());
		return batch;
	}
	
	static void resize(type& batch, std::size_t batchSize, std::size_t elements){
		ensureSize(batch,batchSize);
	}
};

}

///\brief class which helps using different batch types
///
/// e.g. creating a batch of a single element or returning a single element
/// when the element type is arithmetic, like int,double, std::complex,...
/// the return value will be a linear algebra compatible vector, like RealVector.
/// If it is not, for example a string, the return value will be a std::vector<T>. 
template<class T>
//see detail above for implementations, we just choose the correct implementations based on
//whether T is arithmetic or not
struct Batch
:public boost::mpl::if_<
	boost::is_arithmetic<T>,
	detail::ArithmeticBatch<T>,
	detail::DefaultBatch<T>
>::type{};
	
	
///\brief creates a batch from a range of inputs
template<class T, class Range>
typename Batch<T>::type createBatch(Range const& range){
	return Batch<T>::createBatchFromRange(range);
}

/// \brief specialization for ublas vectors which should be matrices in batch mode!
template<class T>
struct Batch<blas::vector<T> >{
	/// \brief Type of a batch of elements.
	typedef boost::numeric::ublas::matrix<T> type;
	/// \brief The type of the elements stored in the batch 
	typedef blas::vector<T> value_type;
	
	
	/// \brief Reference to a single element.
	//typedef boost::numeric::ublas::matrix_row<type> reference;
	typedef MatrixRowReference<type,value_type> reference;
	/// \brief Reference to a single immutable element.
	//typedef boost::numeric::ublas::matrix_row<const type> const_reference;
	typedef MatrixRowReference<const type,value_type> const_reference;
	
	
	/// \brief the iterator type of the object
	typedef ProxyIterator<type, value_type,reference > iterator;
	/// \brief the const_iterator type of the object
	typedef ProxyIterator<const type, value_type, const_reference > const_iterator;
	
	///\brief creates a batch with input as size blueprint
	template<class Element>
	static type createBatch(Element const& input, std::size_t size = 1){
		return type(size,input.size());
	}
	///\brief creates a batch storing the elements referenced by the provided range
	template<class Range>
	static type createBatchFromRange(Range const& range){
		type batch(range.size(),range.begin()->size());
		std::copy(range.begin(),range.end(),boost::begin(batch));
		return batch;
	}
	
	static void resize(type& batch, std::size_t batchSize, std::size_t elements){
		ensureSize(batch,batchSize,elements);
	}
};
/// \brief specialization for ublas compressed vectors which are compressed matrices in batch mode!
template<class T>
struct Batch<boost::numeric::ublas::compressed_vector<T> >{
	/// \brief Type of a batch of elements.
	typedef boost::numeric::ublas::compressed_matrix<T> type;
	
	/// \brief The type of the elements stored in the batch 
	typedef boost::numeric::ublas::compressed_vector<T> value_type;
	
	
	/// \brief Type of a single element.
	//typedef boost::numeric::ublas::matrix_row<type> reference;
	typedef MatrixRowReference<type,value_type> reference;
	/// \brief Type of a single immutable element.
	//typedef boost::numeric::ublas::matrix_row<const type> const_reference;
	typedef MatrixRowReference<const type,value_type> const_reference;
	
	
	/// \brief the iterator type of the object
	typedef ProxyIterator<type, value_type, reference > iterator;
	/// \brief the const_iterator type of the object
	typedef ProxyIterator<const type, value_type, const_reference > const_iterator;
	
	///\brief creates a batch with input as size blueprint
	template<class Element>
	static type createBatch(Element const& input, std::size_t size = 1){
		return type(size,input.size());
	}
	///\brief creates a batch storing the elements referenced by the provided range
	template<class Range>
	static type createBatchFromRange(Range const& range){
		//before creating the batch, we need the number of nonzero elements
		std::size_t nnz = 0;
		for(typename Range::const_iterator pos = range.begin(); pos != range.end(); ++pos){
			nnz += nonzeroElements(*pos);
		}
		
		type batch(range.size(),range.begin()->size(),nnz);
		std::copy(range.begin(),range.end(),boost::begin(batch));
		return batch;
	}
	
	
	static void resize(type& batch, std::size_t batchSize, std::size_t elements){
		ensureSize(batch,batchSize,elements);
	}
};
}

//template specialization for boost::matrices so that they offer the iterator interface

namespace boost{
//first the typedefs which tell boost::range which iterators to use. this needs to be done for all
//supported matrix types separately as well as for the matrix_container/matrix_expression base type
	
//dense matrix
template< class T >
struct range_mutable_iterator< boost::numeric::ublas::matrix<T> >{
	typedef boost::numeric::ublas::vector<T> Vector;
	typedef typename shark::Batch<Vector>::iterator type;
};

template< class T >
struct range_const_iterator< boost::numeric::ublas::matrix<T> >{
	typedef boost::numeric::ublas::vector<T> Vector;
	typedef typename shark::Batch<Vector>::const_iterator type;
};
//compressed matrix
template< class T >
struct range_mutable_iterator< boost::numeric::ublas::compressed_matrix<T> >{
	typedef boost::numeric::ublas::compressed_vector<T> Vector;
	typedef typename shark::Batch<Vector>::iterator type;
};

template< class T >
struct range_const_iterator< boost::numeric::ublas::compressed_matrix<T> >{
	typedef boost::numeric::ublas::compressed_vector<T> Vector;
	typedef typename shark::Batch<Vector>::const_iterator type;
};

//matrix container
template< class M >
struct range_mutable_iterator< boost::numeric::ublas::matrix_container<M> >{
	typedef typename range_mutable_iterator<M>::type type;
};

template< class M >
struct range_const_iterator< boost::numeric::ublas::matrix_container<M> >{
	typedef typename range_const_iterator<M>::type type;
};

//matrix expression
template< class M >
struct range_mutable_iterator< boost::numeric::ublas::matrix_expression<M> >{
	typedef typename range_mutable_iterator<M>::type type;
};

template< class M >
struct range_const_iterator< boost::numeric::ublas::matrix_expression<M> >{
	typedef typename range_const_iterator<M>::type type;
};

namespace numeric{ namespace ublas{

//dense matrix
template< class T >
typename range_iterator<matrix<T> const>::type
range_begin( matrix<T> const& m )
{
	typedef typename range_iterator<matrix<T> const>::type Iter;
	return Iter(m,0);
}
template< class T >
typename range_iterator<matrix<T> >::type
range_begin( matrix<T>& m )
{
	typedef typename range_iterator<matrix<T> >::type Iter;
	return Iter(m,0);
}

template< class T >
typename range_iterator<matrix<T> const>::type
range_end( matrix<T> const& m )
{
	typedef typename range_iterator<matrix<T> const>::type Iter;
	return Iter(m,m.size1());
}
template< class T >
typename range_iterator<matrix<T> >::type
range_end( matrix<T>& m )
{
	typedef typename range_iterator<matrix<T> >::type Iter;
	return Iter(m,m.size1());
}

//compressed matrix
template< class T >
typename range_iterator<compressed_matrix<T> const>::type
range_begin( compressed_matrix<T> const& m )
{
	typedef typename range_iterator<compressed_matrix<T> const>::type Iter;
	return Iter(m,0);
}
template< class T >
typename range_iterator<compressed_matrix<T> >::type
range_begin( compressed_matrix<T>& m )
{
	typedef typename range_iterator<compressed_matrix<T> >::type Iter;
	return Iter(m,0);
}

template< class T >
typename range_iterator<compressed_matrix<T> const>::type
range_end( compressed_matrix<T> const& m )
{
	typedef typename range_iterator<compressed_matrix<T> const>::type Iter;
	return Iter(m,m.size1());
}
template< class T >
typename range_iterator<compressed_matrix<T> >::type
range_end( compressed_matrix<T>& m )
{
	typedef typename range_iterator<compressed_matrix<T> >::type Iter;
	return Iter(m,m.size1());
}


//matrix_container
template< class M >
typename range_iterator<M const>::type
range_begin( matrix_container<M> const& m )
{
	return range_begin(m());
}
template< class M >
typename range_iterator<M>::type
range_begin( matrix_container<M>& m )
{
	return range_begin(m());
}

template< class M >
typename range_iterator<M const>::type
range_end( matrix_container<M> const& m )
{
	return range_end(m());
}
template< class M >
typename range_iterator<M>::type
range_end( matrix_container<M>& m )
{
	return range_end(m());
}

//matrix_expression
template< class M >
typename range_iterator<M const>::type
range_begin( matrix_expression<M> const& m )
{
	return range_begin(m());
}
template< class M >
typename range_iterator<M>::type
range_begin( matrix_expression<M>& m )
{
	return range_begin(m());
}

template< class M >
typename range_iterator<M const>::type
range_end( matrix_expression<M> const& m )
{
	return range_end(m());
}
template< class M >
typename range_iterator<M>::type
range_end( matrix_expression<M>& m )
{
	return range_end(m());
}
}}}

//#include "BatchInterfaceAdaptStruct.h"
#endif