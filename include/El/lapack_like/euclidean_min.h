/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_LAPACK_EUCLIDEANMIN_C_H
#define EL_LAPACK_EUCLIDEANMIN_C_H

#include "El/core/DistMatrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* General Linear Model
   ==================== */
/* Solve min_{X,Y} || Y ||_F subject to D = A X + B Y */

EL_EXPORT ElError ElGLM_s
( ElMatrix_s A, ElMatrix_s B, ElMatrix_s D, ElMatrix_s Y );
EL_EXPORT ElError ElGLM_d
( ElMatrix_d A, ElMatrix_d B, ElMatrix_d D, ElMatrix_d Y );
EL_EXPORT ElError ElGLM_c
( ElMatrix_c A, ElMatrix_c B, ElMatrix_c D, ElMatrix_c Y );
EL_EXPORT ElError ElGLM_z
( ElMatrix_z A, ElMatrix_z B, ElMatrix_z D, ElMatrix_z Y );

EL_EXPORT ElError ElGLMDist_s
( ElDistMatrix_s A, ElDistMatrix_s B, ElDistMatrix_s D, ElDistMatrix_s Y );
EL_EXPORT ElError ElGLMDist_d
( ElDistMatrix_d A, ElDistMatrix_d B, ElDistMatrix_d D, ElDistMatrix_d Y );
EL_EXPORT ElError ElGLMDist_c
( ElDistMatrix_c A, ElDistMatrix_c B, ElDistMatrix_c D, ElDistMatrix_c Y );
EL_EXPORT ElError ElGLMDist_z
( ElDistMatrix_z A, ElDistMatrix_z B, ElDistMatrix_z D, ElDistMatrix_z Y );

/* Least squares
   ============= */
/* When height(A) >= width(A), solve

     min_X || A X - B ||_F,

   otherwise, solve

     min_X || X ||_F s.t. A X = B. */

EL_EXPORT ElError ElLeastSquares_s
( ElOrientation orientation, ElMatrix_s A, ElConstMatrix_s B, ElMatrix_s X );
EL_EXPORT ElError ElLeastSquares_d
( ElOrientation orientation, ElMatrix_d A, ElConstMatrix_d B, ElMatrix_d X );
EL_EXPORT ElError ElLeastSquares_c
( ElOrientation orientation, ElMatrix_c A, ElConstMatrix_c B, ElMatrix_c X );
EL_EXPORT ElError ElLeastSquares_z
( ElOrientation orientation, ElMatrix_z A, ElConstMatrix_z B, ElMatrix_z X );

EL_EXPORT ElError ElLeastSquaresDist_s
( ElOrientation orientation, 
  ElDistMatrix_s A, ElConstDistMatrix_s B, ElDistMatrix_s X );
EL_EXPORT ElError ElLeastSquaresDist_d
( ElOrientation orientation, 
  ElDistMatrix_d A, ElConstDistMatrix_d B, ElDistMatrix_d X );
EL_EXPORT ElError ElLeastSquaresDist_c
( ElOrientation orientation, 
  ElDistMatrix_c A, ElConstDistMatrix_c B, ElDistMatrix_c X );
EL_EXPORT ElError ElLeastSquaresDist_z
( ElOrientation orientation, 
  ElDistMatrix_z A, ElConstDistMatrix_z B, ElDistMatrix_z X );

EL_EXPORT ElError ElLeastSquaresSparse_s
( ElOrientation orientation, ElSparseMatrix_s A, 
  ElConstMatrix_s B, ElMatrix_s X );
EL_EXPORT ElError ElLeastSquaresSparse_d
( ElOrientation orientation, ElSparseMatrix_d A, 
  ElConstMatrix_d B, ElMatrix_d X );
EL_EXPORT ElError ElLeastSquaresSparse_c
( ElOrientation orientation, ElSparseMatrix_c A, 
  ElConstMatrix_c B, ElMatrix_c X );
EL_EXPORT ElError ElLeastSquaresSparse_z
( ElOrientation orientation, ElSparseMatrix_z A, 
  ElConstMatrix_z B, ElMatrix_z X );

EL_EXPORT ElError ElLeastSquaresDistSparse_s
( ElOrientation orientation,
  ElConstDistSparseMatrix_s A, ElConstDistMultiVec_s B, ElDistMultiVec_s X );
EL_EXPORT ElError ElLeastSquaresDistSparse_d
( ElOrientation orientation,
  ElConstDistSparseMatrix_d A, ElConstDistMultiVec_d B, ElDistMultiVec_d X );
EL_EXPORT ElError ElLeastSquaresDistSparse_c
( ElOrientation orientation,
  ElConstDistSparseMatrix_c A, ElConstDistMultiVec_c B, ElDistMultiVec_c X );
EL_EXPORT ElError ElLeastSquaresDistSparse_z
( ElOrientation orientation,
  ElConstDistSparseMatrix_z A, ElConstDistMultiVec_z B, ElDistMultiVec_z X );

/* Expert versions
   --------------- */
typedef struct {
  float alpha;
  ElRegQSDCtrl_s qsdCtrl;
  bool equilibrate;
  bool progress;
  bool time;
} ElLeastSquaresCtrl_s;
ElError ElLeastSquaresCtrlDefault_s( ElLeastSquaresCtrl_s* ctrl );

typedef struct {
  double alpha;
  ElRegQSDCtrl_d qsdCtrl;
  bool equilibrate;
  bool progress;
  bool time;
} ElLeastSquaresCtrl_d;
ElError ElLeastSquaresCtrlDefault_d( ElLeastSquaresCtrl_d* ctrl );

EL_EXPORT ElError ElLeastSquaresXSparse_s
( ElOrientation orientation, ElSparseMatrix_s A, 
  ElConstMatrix_s B, ElMatrix_s X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLeastSquaresXSparse_d
( ElOrientation orientation, ElSparseMatrix_d A, 
  ElConstMatrix_d B, ElMatrix_d X, ElLeastSquaresCtrl_d ctrl );
EL_EXPORT ElError ElLeastSquaresXSparse_c
( ElOrientation orientation, ElSparseMatrix_c A, 
  ElConstMatrix_c B, ElMatrix_c X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLeastSquaresXSparse_z
( ElOrientation orientation, ElSparseMatrix_z A, 
  ElConstMatrix_z B, ElMatrix_z X, ElLeastSquaresCtrl_d ctrl );

EL_EXPORT ElError ElLeastSquaresXDistSparse_s
( ElOrientation orientation,
  ElConstDistSparseMatrix_s A, ElConstDistMultiVec_s B, ElDistMultiVec_s X,
  ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLeastSquaresXDistSparse_d
( ElOrientation orientation,
  ElConstDistSparseMatrix_d A, ElConstDistMultiVec_d B, ElDistMultiVec_d X,
  ElLeastSquaresCtrl_d ctrl );
EL_EXPORT ElError ElLeastSquaresXDistSparse_c
( ElOrientation orientation,
  ElConstDistSparseMatrix_c A, ElConstDistMultiVec_c B, ElDistMultiVec_c X,
  ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLeastSquaresXDistSparse_z
( ElOrientation orientation,
  ElConstDistSparseMatrix_z A, ElConstDistMultiVec_z B, ElDistMultiVec_z X,
  ElLeastSquaresCtrl_d ctrl );

/* Equality-constrained Least Squares
   ================================== */
/* Solves min_X || A X - C ||_F subject to B X = D */

EL_EXPORT ElError ElLSE_s
( ElMatrix_s A, ElMatrix_s B, ElMatrix_s C, ElMatrix_s D, ElMatrix_s X );
EL_EXPORT ElError ElLSE_d
( ElMatrix_d A, ElMatrix_d B, ElMatrix_d C, ElMatrix_d D, ElMatrix_d X );
EL_EXPORT ElError ElLSE_c
( ElMatrix_c A, ElMatrix_c B, ElMatrix_c C, ElMatrix_c D, ElMatrix_c X );
EL_EXPORT ElError ElLSE_z
( ElMatrix_z A, ElMatrix_z B, ElMatrix_z C, ElMatrix_z D, ElMatrix_z X );

EL_EXPORT ElError ElLSEDist_s
( ElDistMatrix_s A, ElDistMatrix_s B, ElDistMatrix_s C, ElDistMatrix_s D, 
  ElDistMatrix_s X );
EL_EXPORT ElError ElLSEDist_d
( ElDistMatrix_d A, ElDistMatrix_d B, ElDistMatrix_d C, ElDistMatrix_d D, 
  ElDistMatrix_d X );
EL_EXPORT ElError ElLSEDist_c
( ElDistMatrix_c A, ElDistMatrix_c B, ElDistMatrix_c C, ElDistMatrix_c D, 
  ElDistMatrix_c X );
EL_EXPORT ElError ElLSEDist_z
( ElDistMatrix_z A, ElDistMatrix_z B, ElDistMatrix_z C, ElDistMatrix_z D, 
  ElDistMatrix_z X );

/* TODO: Expert version which also returns the residual */

EL_EXPORT ElError ElLSESparse_s
( ElConstSparseMatrix_s A, ElConstSparseMatrix_s B,
  ElConstMatrix_s C,       ElConstMatrix_s D,
  ElMatrix_s X );
EL_EXPORT ElError ElLSESparse_d
( ElConstSparseMatrix_d A, ElConstSparseMatrix_d B,
  ElConstMatrix_d C,       ElConstMatrix_d D,
  ElMatrix_d X );
EL_EXPORT ElError ElLSESparse_c
( ElConstSparseMatrix_c A, ElConstSparseMatrix_c B,
  ElConstMatrix_c C,       ElConstMatrix_c D,
  ElMatrix_c X );
EL_EXPORT ElError ElLSESparse_z
( ElConstSparseMatrix_z A, ElConstSparseMatrix_z B,
  ElConstMatrix_z C,       ElConstMatrix_z D,
  ElMatrix_z X );

EL_EXPORT ElError ElLSEDistSparse_s
( ElConstDistSparseMatrix_s A, ElConstDistSparseMatrix_s B,
  ElConstDistMultiVec_s C,     ElConstDistMultiVec_s D,
  ElDistMultiVec_s X );
EL_EXPORT ElError ElLSEDistSparse_d
( ElConstDistSparseMatrix_d A, ElConstDistSparseMatrix_d B,
  ElConstDistMultiVec_d C,     ElConstDistMultiVec_d D,
  ElDistMultiVec_d X );
EL_EXPORT ElError ElLSEDistSparse_c
( ElConstDistSparseMatrix_c A, ElConstDistSparseMatrix_c B,
  ElConstDistMultiVec_c C,     ElConstDistMultiVec_c D,
  ElDistMultiVec_c X );
EL_EXPORT ElError ElLSEDistSparse_z
( ElConstDistSparseMatrix_z A, ElConstDistSparseMatrix_z B,
  ElConstDistMultiVec_z C,     ElConstDistMultiVec_z D,
  ElDistMultiVec_z X );

EL_EXPORT ElError ElLSEXSparse_s
( ElConstSparseMatrix_s A, ElConstSparseMatrix_s B,
  ElConstMatrix_s C,       ElConstMatrix_s D,
  ElMatrix_s X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLSEXSparse_d
( ElConstSparseMatrix_d A, ElConstSparseMatrix_d B,
  ElConstMatrix_d C,       ElConstMatrix_d D,
  ElMatrix_d X, ElLeastSquaresCtrl_d ctrl );
EL_EXPORT ElError ElLSEXSparse_c
( ElConstSparseMatrix_c A, ElConstSparseMatrix_c B,
  ElConstMatrix_c C,       ElConstMatrix_c D,
  ElMatrix_c X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLSEXSparse_z
( ElConstSparseMatrix_z A, ElConstSparseMatrix_z B,
  ElConstMatrix_z C,       ElConstMatrix_z D,
  ElMatrix_z X, ElLeastSquaresCtrl_d ctrl );

EL_EXPORT ElError ElLSEXDistSparse_s
( ElConstDistSparseMatrix_s A, ElConstDistSparseMatrix_s B,
  ElConstDistMultiVec_s C,     ElConstDistMultiVec_s D,
  ElDistMultiVec_s X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLSEXDistSparse_d
( ElConstDistSparseMatrix_d A, ElConstDistSparseMatrix_d B,
  ElConstDistMultiVec_d C,     ElConstDistMultiVec_d D,
  ElDistMultiVec_d X, ElLeastSquaresCtrl_d ctrl );
EL_EXPORT ElError ElLSEXDistSparse_c
( ElConstDistSparseMatrix_c A, ElConstDistSparseMatrix_c B,
  ElConstDistMultiVec_c C,     ElConstDistMultiVec_c D,
  ElDistMultiVec_c X, ElLeastSquaresCtrl_s ctrl );
EL_EXPORT ElError ElLSEXDistSparse_z
( ElConstDistSparseMatrix_z A, ElConstDistSparseMatrix_z B,
  ElConstDistMultiVec_z C,     ElConstDistMultiVec_z D,
  ElDistMultiVec_z X, ElLeastSquaresCtrl_d ctrl );

/* Ridge regression
   ================ */
/* Ridge regression is a special case of Tikhonov regularization with 
   the regularization matrix equal to gamma I */

typedef enum {
  EL_RIDGE_CHOLESKY,
  EL_RIDGE_QR,
  EL_RIDGE_SVD
} ElRidgeAlg;

EL_EXPORT ElError ElRidge_s
( ElOrientation orientation,
  ElConstMatrix_s A, ElConstMatrix_s B, 
  float gamma,       ElMatrix_s X,
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidge_d
( ElOrientation orientation,
  ElConstMatrix_d A, ElConstMatrix_d B, 
  double gamma,      ElMatrix_d X,
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidge_c
( ElOrientation orientation,
  ElConstMatrix_c A, ElConstMatrix_c B, 
  float gamma,       ElMatrix_c X,
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidge_z
( ElOrientation orientation,
  ElConstMatrix_z A, ElConstMatrix_z B, 
  double gamma,      ElMatrix_z X,
  ElRidgeAlg alg );

EL_EXPORT ElError ElRidgeDist_s
( ElOrientation orientation,
  ElConstDistMatrix_s A, ElConstDistMatrix_s B, 
  float gamma,           ElDistMatrix_s X, 
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidgeDist_d
( ElOrientation orientation,
  ElConstDistMatrix_d A, ElConstDistMatrix_d B, 
  double gamma,          ElDistMatrix_d X, 
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidgeDist_c
( ElOrientation orientation,
  ElConstDistMatrix_c A, ElConstDistMatrix_c B, 
  float gamma,           ElDistMatrix_c X, 
  ElRidgeAlg alg );
EL_EXPORT ElError ElRidgeDist_z
( ElOrientation orientation,
  ElConstDistMatrix_z A, ElConstDistMatrix_z B, 
  double gamma,          ElDistMatrix_z X, 
  ElRidgeAlg alg );

EL_EXPORT ElError ElRidgeSparse_s
( ElOrientation orientation,
  ElConstSparseMatrix_s A, ElConstMatrix_s B,
  float gamma,             ElMatrix_s X );
EL_EXPORT ElError ElRidgeSparse_d
( ElOrientation orientation,
  ElConstSparseMatrix_d A, ElConstMatrix_d B,
  double gamma,            ElMatrix_d X );
EL_EXPORT ElError ElRidgeSparse_c
( ElOrientation orientation,
  ElConstSparseMatrix_c A, ElConstMatrix_c B,
  float gamma,             ElMatrix_c X );
EL_EXPORT ElError ElRidgeSparse_z
( ElOrientation orientation,
  ElConstSparseMatrix_z A, ElConstMatrix_z B,
  double gamma,            ElMatrix_z X );

EL_EXPORT ElError ElRidgeDistSparse_s
( ElOrientation orientation,
  ElConstDistSparseMatrix_s A, ElConstDistMultiVec_s B,
  float gamma,                 ElDistMultiVec_s X );
EL_EXPORT ElError ElRidgeDistSparse_d
( ElOrientation orientation,
  ElConstDistSparseMatrix_d A, ElConstDistMultiVec_d B,
  double gamma,                ElDistMultiVec_d X );
EL_EXPORT ElError ElRidgeDistSparse_c
( ElOrientation orientation,
  ElConstDistSparseMatrix_c A, ElConstDistMultiVec_c B,
  float gamma,                 ElDistMultiVec_c X );
EL_EXPORT ElError ElRidgeDistSparse_z
( ElOrientation orientation,
  ElConstDistSparseMatrix_z A, ElConstDistMultiVec_z B,
  double gamma,                ElDistMultiVec_z X );

/* Tikhonov regularization
   ======================= */
/* Defining W = op(A), where op(A) is either A, A^T, or A^H, Tikhonov
   regularization involves the solution of either

   Regularized Least Squares:

     min_X || [W; G] X - [B; 0] ||_F^2

   or

   Regularized Minimum Length:

     min_{X,S} || [X, S] ||_F^
     s.t.      [W, G] [X; S] = B.
*/

typedef enum {
  EL_TIKHONOV_CHOLESKY,
  EL_TIKHONOV_QR
} ElTikhonovAlg;

EL_EXPORT ElError ElTikhonov_s
( ElOrientation orientation,
  ElConstMatrix_s A, ElConstMatrix_s B, 
  ElConstMatrix_s G, ElMatrix_s X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonov_d
( ElOrientation orientation,
  ElConstMatrix_d A, ElConstMatrix_d B, 
  ElConstMatrix_d G, ElMatrix_d X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonov_c
( ElOrientation orientation,
  ElConstMatrix_c A, ElConstMatrix_c B, 
  ElConstMatrix_c G, ElMatrix_c X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonov_z
( ElOrientation orientation,
  ElConstMatrix_z A, ElConstMatrix_z B, 
  ElConstMatrix_z G, ElMatrix_z X, 
  ElTikhonovAlg alg );

EL_EXPORT ElError ElTikhonovDist_s
( ElOrientation orientation,
  ElConstDistMatrix_s A, ElConstDistMatrix_s B, 
  ElConstDistMatrix_s G, ElDistMatrix_s X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonovDist_d
( ElOrientation orientation,
  ElConstDistMatrix_d A, ElConstDistMatrix_d B, 
  ElConstDistMatrix_d G, ElDistMatrix_d X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonovDist_c
( ElOrientation orientation,
  ElConstDistMatrix_c A, ElConstDistMatrix_c B, 
  ElConstDistMatrix_c G, ElDistMatrix_c X, 
  ElTikhonovAlg alg );
EL_EXPORT ElError ElTikhonovDist_z
( ElOrientation orientation,
  ElConstDistMatrix_z A, ElConstDistMatrix_z B, 
  ElConstDistMatrix_z G, ElDistMatrix_z X, 
  ElTikhonovAlg alg );

EL_EXPORT ElError ElTikhonovSparse_s
( ElOrientation orientation,
  ElConstSparseMatrix_s A, ElConstMatrix_s B,
  ElConstSparseMatrix_s G, ElMatrix_s X );
EL_EXPORT ElError ElTikhonovSparse_d
( ElOrientation orientation,
  ElConstSparseMatrix_d A, ElConstMatrix_d B,
  ElConstSparseMatrix_d G, ElMatrix_d X );
EL_EXPORT ElError ElTikhonovSparse_c
( ElOrientation orientation,
  ElConstSparseMatrix_c A, ElConstMatrix_c B,
  ElConstSparseMatrix_c G, ElMatrix_c X );
EL_EXPORT ElError ElTikhonovSparse_z
( ElOrientation orientation,
  ElConstSparseMatrix_z A, ElConstMatrix_z B,
  ElConstSparseMatrix_z G, ElMatrix_z X );

EL_EXPORT ElError ElTikhonovDistSparse_s
( ElOrientation orientation,
  ElConstDistSparseMatrix_s A, ElConstDistMultiVec_s B,
  ElConstDistSparseMatrix_s G, ElDistMultiVec_s X );
EL_EXPORT ElError ElTikhonovDistSparse_d
( ElOrientation orientation,
  ElConstDistSparseMatrix_d A, ElConstDistMultiVec_d B,
  ElConstDistSparseMatrix_d G, ElDistMultiVec_d X );
EL_EXPORT ElError ElTikhonovDistSparse_c
( ElOrientation orientation,
  ElConstDistSparseMatrix_c A, ElConstDistMultiVec_c B,
  ElConstDistSparseMatrix_c G, ElDistMultiVec_c X );
EL_EXPORT ElError ElTikhonovDistSparse_z
( ElOrientation orientation,
  ElConstDistSparseMatrix_z A, ElConstDistMultiVec_z B,
  ElConstDistSparseMatrix_z G, ElDistMultiVec_z X );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ifndef EL_LAPACK_EUCLIDEANMIN_C_H */
