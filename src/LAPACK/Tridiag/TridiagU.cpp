/*
   Copyright 2009-2010 Jack Poulson

   This file is part of Elemental.

   Elemental is free software: you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 3 of the License, or 
   (at your option) any later version.

   Elemental is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Elemental. If not, see <http://www.gnu.org/licenses/>.
*/
#include "ElementalLAPACKInternal.h"
using namespace std;
using namespace Elemental;

template<typename R>
void
Elemental::LAPACK::Internal::TridiagU
( DistMatrix<R,MC,MR  >& A,
  DistMatrix<R,MD,Star>& d,
  DistMatrix<R,MD,Star>& e,
  DistMatrix<R,MD,Star>& t )
{
#ifndef RELEASE
    PushCallStack("LAPACK::Internal::TridiagU");
#endif
    const Grid& grid = A.GetGrid();
#ifndef RELEASE
    if( A.GetGrid() != d.GetGrid() ||
        d.GetGrid() != e.GetGrid() ||
        e.GetGrid() != t.GetGrid()   )
    {
        if( grid.VCRank() == 0 )
            cerr << "A, d, e, and t must be distributed over the same grid."
                 << endl;
        DumpCallStack();
        throw exception();
    }
    if( A.Height() != A.Width() )
    {
        if( grid.VCRank() == 0 )
            cerr << "A must be square." << endl;
        DumpCallStack();
    }
    if( d.Height() != A.Height() || d.Width() != 1 )
    {
        if( grid.VCRank() == 0 )
            cerr << "d must be a column vector of the same length as A's width."
                 << endl;
        DumpCallStack();
        throw exception();
    }
    if( e.Height() != A.Height()-1 || e.Width() != 1 )
    {
        if( grid.VCRank() == 0 )
            cerr << "e must be a column vector of length one less than the "
                 << "width of A." << endl;
        DumpCallStack();
        throw exception();
    }
    if( t.Height() != A.Height()-1 || t.Width() != 1 )
    {
        if( grid.VCRank() == 0 )
            cerr << "t must be a column vector of length one less than the "
                 << "width of A." << endl;
        DumpCallStack();
        throw exception();
    }
    if( d.ColAlignment() != A.ColAlignment() + A.RowAlignment()*grid.Height() )
    {
        if( grid.VCRank() == 0 )
            cerr << "d is not aligned with A." << endl;
        DumpCallStack();
        throw exception();
    }
    if( e.ColAlignment() != A.ColAlignment() +
                            ((A.RowAlignment()+1)%grid.Width())*grid.Height() )
    {
        if( grid.VCRank() == 0 )
            cerr << "e is not aligned with A." << endl;
        DumpCallStack();
        throw exception();
    }
    if( t.ColAlignment() != A.ColAlignment() + A.RowAlignment()*grid.Height() )
    {
        if( grid.VCRank() == 0 )
            cerr << "t is not aligned with A." << endl;
        DumpCallStack();
        throw exception();
    }
#endif

    // Matrix views 
    DistMatrix<R,MC,MR> 
        ATL(grid), ATR(grid),  A00(grid), A01(grid), A02(grid),  
        ABL(grid), ABR(grid),  A10(grid), A11(grid), A12(grid),
                               A20(grid), A21(grid), A22(grid),
        A11_expanded(grid);
    DistMatrix<R,MD,Star> dT(grid),  d0(grid), 
                          dB(grid),  d1(grid),
                                     d2(grid);
    DistMatrix<R,MD,Star> eT(grid),  e0(grid),
                          eB(grid),  e1(grid), 
                                     e2(grid);
    DistMatrix<R,MD,Star> tT(grid),  t0(grid), 
                          tB(grid),  t1(grid),
                                     t2(grid);

    // Temporary distributions
    Matrix<R> A11_Trans;
    DistMatrix<R,Star,Star> A11_Star_Star(grid);
    DistMatrix<R,Star,Star> d1_Star_Star(grid);
    DistMatrix<R,Star,Star> e1_Star_Star(grid);
    DistMatrix<R,Star,Star> t1_Star_Star(grid);
    DistMatrix<R,MC,  MR  > W11(grid), W12(grid),  WPan(grid);

    PartitionDownDiagonal( A, ATL, ATR,
                              ABL, ABR );
    PartitionDown( d, dT,
                      dB );
    PartitionDown( e, eT,
                      eB );
    PartitionDown( t, tT,
                      tB );
    while( ATL.Height() < A.Height() )
    {
        RepartitionDownDiagonal( ATL, /**/ ATR,  A00, /**/ A01, A02,
                                /*************/ /******************/
                                      /**/       A10, /**/ A11, A12,
                                 ABL, /**/ ABR,  A20, /**/ A21, A22 );

        RepartitionDown( dT,  d0,
                        /**/ /**/
                              d1,
                         dB,  d2 );

        RepartitionDown( eT,  e0,
                        /**/ /**/
                              e1,
                         eB,  e2 );

        RepartitionDown( tT,  t0,
                        /**/ /**/
                              t1,
                         tB,  t2 );

        if( A22.Height() > 0 )
        {
            A11_expanded.View( ABR, 0, 0, A11.Height()+1, A11.Width()+1 );
            WPan.AlignWith( ABR );
            WPan.ResizeTo( A11.Height(), ABR.Width() );
            WPan.SetToZero();
            PartitionRight( WPan, W11, W12, A11.Width() );
            //----------------------------------------------------------------//
            LAPACK::Internal::PanelTridiagU( ABR, WPan, e1, t1 );
            BLAS::Syr2k( Upper, Transpose, 
                         (R)-1, A12, W12, (R)1, A22 );
            A11_expanded.SetDiagonal( e1, 1 );
            A11.GetDiagonal( d1 );
            //----------------------------------------------------------------//
            WPan.FreeConstraints();
        }
        else
        {
            A11_Star_Star = A11;
            d1_Star_Star = d1;
            e1_Star_Star = e1;
            t1_Star_Star = t1;

            // LAPACK traverses up the diagonal in upper Tridiag, but we 
            // traverse down, so transpose to and from to call the lower Tridiag
            BLAS::Trans( A11_Star_Star.LockedLocalMatrix(), A11_Trans );

            LAPACK::Tridiag( Lower, A11_Trans,         
                             d1_Star_Star.LocalMatrix(),
                             e1_Star_Star.LocalMatrix(),
                             t1_Star_Star.LocalMatrix() );
            
            BLAS::Trans( A11_Trans, A11_Star_Star.LocalMatrix() );

            A11 = A11_Star_Star;
            d1 = d1_Star_Star;
            e1 = e1_Star_Star;
            t1 = t1_Star_Star;
        }

        SlidePartitionDownDiagonal( ATL, /**/ ATR,  A00, A01, /**/ A02,
                                         /**/       A10, A11, /**/ A12,
                                   /*************/ /******************/
                                    ABL, /**/ ABR,  A20, A21, /**/ A22 );

        SlidePartitionDown( dT,  d0,
                                 d1,
                           /**/ /**/
                            dB,  d2 );

        SlidePartitionDown( eT,  e0,
                                 e1,
                           /**/ /**/
                            eB,  e2 );

        SlidePartitionDown( tT,  t0,
                                 t1,
                           /**/ /**/
                            tB,  t2 );
    }
        
#ifndef RELEASE
    PopCallStack();
#endif
}

template void Elemental::LAPACK::Internal::TridiagU
( DistMatrix<float,MC,MR  >& A,
  DistMatrix<float,MD,Star>& d,
  DistMatrix<float,MD,Star>& e,
  DistMatrix<float,MD,Star>& t );

template void Elemental::LAPACK::Internal::TridiagU
( DistMatrix<double,MC,MR  >& A,
  DistMatrix<double,MD,Star>& d,
  DistMatrix<double,MD,Star>& e,
  DistMatrix<double,MD,Star>& t );

