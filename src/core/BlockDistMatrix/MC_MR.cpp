/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

#define COLDIST MC
#define ROWDIST MR

#include "./setup.hpp"

namespace El {

// Public section
// ##############

// Assignment and reconfiguration
// ==============================

template<typename T>
BDM& BDM::operator=( const BDM& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MC,MR]"))
    copy::Translate( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,MC,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MC,STAR]"))
    copy::RowFilter( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,MR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,MR]"))
    copy::ColFilter( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,MD,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MD,STAR]"))
    // TODO: More efficient implementation?
    BlockDistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,MD>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,MD]"))
    // TODO: More efficient implementation?
    BlockDistMatrix<T,STAR,STAR> A_STAR_STAR( A );
    *this = A_STAR_STAR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,MR,MC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MR,MC]"))
    LogicError("This routine is not yet written");
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,MR,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [MR,STAR]"))
    auto A_VR_STAR = MakeUnique<BlockDistMatrix<T,VR,STAR>>( A );
    auto A_VC_STAR = MakeUnique<BlockDistMatrix<T,VC,STAR>>( this->Grid() );
    A_VC_STAR->AlignWith( *this );
    *A_VC_STAR = *A_VR_STAR;
    A_VR_STAR.reset();
    *this = *A_VC_STAR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,MC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,MC]"))
    auto A_STAR_VC = MakeUnique<BlockDistMatrix<T,STAR,VC>>( A );
    auto A_STAR_VR = MakeUnique<BlockDistMatrix<T,STAR,VR>>( this->Grid() );
    A_STAR_VR->AlignWith( *this );
    *A_STAR_VR = *A_STAR_VC;
    A_STAR_VC.reset(); 
    *this = *A_STAR_VR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,VC,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [VC,STAR]"))
    copy::ColAllToAllPromote( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,VC>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,VC]"))
    BlockDistMatrix<T,STAR,VR> A_STAR_VR(this->Grid());
    A_STAR_VR.AlignWith( *this );
    A_STAR_VR = A;
    *this = A_STAR_VR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,VR,STAR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [VR,STAR]"))
    BlockDistMatrix<T,VC,STAR> A_VC_STAR(this->Grid());
    A_VC_STAR.AlignWith( *this );
    A_VC_STAR = A;
    *this = A_VC_STAR;
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,VR>& A )
{ 
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,VR]"))
    copy::RowAllToAllPromote( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,STAR,STAR>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [STAR,STAR]"))
    copy::Filter( A, *this );
    return *this;
}

template<typename T>
BDM& BDM::operator=( const BlockDistMatrix<T,CIRC,CIRC>& A )
{
    DEBUG_ONLY(CallStackEntry cse("[MC,MR] = [CIRC,CIRC]"))
    LogicError("This routine is not yet written");
    return *this;
}

template<typename T>
BDM& BDM::operator=( const AbstractBlockDistMatrix<T>& A )
{
    DEBUG_ONLY(CallStackEntry cse("BDM = ABDM"))
    #define GUARD(CDIST,RDIST) \
      A.DistData().colDist == CDIST && A.DistData().rowDist == RDIST
    #define PAYLOAD(CDIST,RDIST) \
      auto& ACast = dynamic_cast<const BlockDistMatrix<T,CDIST,RDIST>&>(A); \
      *this = ACast;
    #include "El/macros/GuardAndPayload.h"
    return *this;
}

// Basic queries
// =============

template<typename T>
mpi::Comm BDM::DistComm() const { return this->grid_->VCComm(); }
template<typename T>
mpi::Comm BDM::CrossComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm BDM::RedundantComm() const { return mpi::COMM_SELF; }
template<typename T>
mpi::Comm BDM::ColComm() const { return this->grid_->MCComm(); }
template<typename T>
mpi::Comm BDM::RowComm() const { return this->grid_->MRComm(); }

template<typename T>
Int BDM::ColStride() const { return this->grid_->MCSize(); }
template<typename T>
Int BDM::RowStride() const { return this->grid_->MRSize(); }
template<typename T>
Int BDM::DistSize() const { return this->grid_->VCSize(); }
template<typename T>
Int BDM::CrossSize() const { return 1; }
template<typename T>
Int BDM::RedundantSize() const { return 1; }

// Instantiate {Int,Real,Complex<Real>} for each Real in {float,double}
// ####################################################################

#define SELF(T,U,V) \
  template BlockDistMatrix<T,COLDIST,ROWDIST>::BlockDistMatrix \
  ( const BlockDistMatrix<T,U,V>& A );
#define OTHER(T,U,V) \
  template BlockDistMatrix<T,COLDIST,ROWDIST>::BlockDistMatrix \
  ( const DistMatrix<T,U,V>& A ); \
  template BlockDistMatrix<T,COLDIST,ROWDIST>& \
           BlockDistMatrix<T,COLDIST,ROWDIST>::operator= \
           ( const DistMatrix<T,U,V>& A )
#define BOTH(T,U,V) \
  SELF(T,U,V); \
  OTHER(T,U,V)
#define PROTO(T) \
  template class BlockDistMatrix<T,COLDIST,ROWDIST>; \
  BOTH( T,CIRC,CIRC); \
  OTHER(T,MC,  MR  ); \
  BOTH( T,MC,  STAR); \
  BOTH( T,MD,  STAR); \
  BOTH( T,MR,  MC  ); \
  BOTH( T,MR,  STAR); \
  BOTH( T,STAR,MC  ); \
  BOTH( T,STAR,MD  ); \
  BOTH( T,STAR,MR  ); \
  BOTH( T,STAR,STAR); \
  BOTH( T,STAR,VC  ); \
  BOTH( T,STAR,VR  ); \
  BOTH( T,VC,  STAR); \
  BOTH( T,VR,  STAR);

#include "El/macros/Instantiate.h"

} // namespace El
