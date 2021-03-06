//===========================================================================
/*!
 * 
 *
 * \brief       -
 *
 * \author      O. Krause
 * \date        2016
 *
 *
 * \par Copyright 1995-2015 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://image.diku.dk/shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================
#ifndef REMORA_KERNELS_CLBLAST_SYRK_HPP
#define REMORA_KERNELS_CLBLAST_SYRK_HPP

#include "../../expression_types.hpp"
#include "../../detail/traits.hpp"
#include <clblast.h>
namespace remora{ namespace kernels{

// C <- C + alpha * A * A^T
template <bool Upper, typename MatA, typename MatC>
void syrk(
	matrix_expression<MatA, gpu_tag> const& A,
	matrix_expression<MatC, gpu_tag>& C, 
	typename MatC::value_type const& alpha
) {
	REMORA_SIZE_CHECK(A().size1() == C().size1());
	REMORA_SIZE_CHECK(C().size1()== C().size2());
	
	static_assert(std::is_same<typename MatA::value_type, typename MatC::value_type>::value, "[syrk] Arguments do not have same element type");
	static_assert(std::is_same<typename MatA::evaluation_category::tag, dense_tag>::value, "[syrk] A is not dense");
	static_assert(std::is_same<typename MatC::storage_type::storage_tag, dense_tag>::value, "[syrk] C does not have dense storage layout");
	
	//pre-evaluate A into a temporary if necessary
	auto const& Aeval = eval_expression(A);
		
	using namespace clblast;
	
	//obtain geometry information
	auto transA = std::is_same<typename MatA::orientation,typename MatC::orientation>::value? Transpose::kNo : Transpose::kYes;
	auto layout = std::is_same<typename MatC::orientation::orientation, row_major>::value? Layout::kRowMajor: Layout::kColMajor; 
	auto triangular = Upper? Triangle::kUpper : Triangle::kLower; 
	std::size_t n = C().size1();
	std::size_t k = A().size2();

	//obtain matrix storage
	auto storageA = Aeval.raw_storage();
	auto storageC = C().raw_storage();
	
	//call
	cl_event* event = nullptr;//todo: store events for out-of-order queues 
	auto code =  Syrk(layout, triangular, transA,
                n, k, alpha,
		storageA.buffer.get(), storageA.offset, storageA.leading_dimension,
                typename MatC::value_type(1),
		storageC.buffer.get(), storageC.offset, storageC.leading_dimension,
                &C().queue().get(), event
	);
		
	assert(code == StatusCode::kSuccess);
}

}}

#endif
