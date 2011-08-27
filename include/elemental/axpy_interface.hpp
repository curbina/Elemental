/*
   Copyright (c) 2009-2011, Jack Poulson
   Copyright (c) 2011, The University of Texas at Austin
   All rights reserved.

   This file is part of Elemental.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    - Neither the name of the owner nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

   Authors:
   This interface is mainly due to Martin Schatz, but it was put into its
   current form by Jack Poulson.
*/
#ifndef ELEMENTAL_AXPY_INTERFACE_HPP
#define ELEMENTAL_AXPY_INTERFACE_HPP 1

/* 
   Template conventions:
     G: general datatype
  
     T: any ring, e.g., the (Gaussian) integers and the real/complex numbers
     Z: representation of a real ring, e.g., the integers or real numbers
     std::complex<Z>: representation of a complex ring, e.g. Gaussian integers
                      or complex numbers

     F: representation of real or complex number
     R: representation of real number
     std::complex<R>: representation of complex number
*/

namespace elemental {

enum AxpyType { LOCAL_TO_GLOBAL, GLOBAL_TO_LOCAL };

template<typename T>
class AxpyInterface
{   
    static const int DATA_TAG=1, EOM_TAG=2, 
                     DATA_REQUEST_TAG=3, DATA_REPLY_TAG=4;

    bool _attachedForLocalToGlobal, _attachedForGlobalToLocal;
    char _sendDummy, _recvDummy;
    DistMatrix<T,MC,MR>* _localToGlobalMat;
    const DistMatrix<T,MC,MR>* _globalToLocalMat;

    std::vector<bool> _sentEomTo, _haveEomFrom;
    std::vector<char> _recvVector;
    std::vector<imports::mpi::Request> _eomSendRequests;

    std::vector<std::vector<std::vector<char> > >
        _dataVectors, _requestVectors, _replyVectors;
    std::vector<std::vector<bool> > 
        _sendingData, _sendingRequest, _sendingReply;
    std::vector<std::vector<imports::mpi::Request> > 
        _dataSendRequests, _requestSendRequests, _replySendRequests;

    struct LocalToGlobalHeader
    {
        int i, j, height, width;
        T alpha;
    };

    struct GlobalToLocalRequestHeader
    {
        int i, j, height, width;
    };

    struct GlobalToLocalReplyHeader
    {
        int processRow, processCol;
    };

    // Check if we are done with this attachment's work
    bool Finished();

    // Progress functions
    void UpdateRequestStatuses();
    void HandleEoms();
    void HandleLocalToGlobalData();
    void HandleGlobalToLocalRequest();
    void StartSendingEoms();
    void FinishSendingEoms();

    void AxpyLocalToGlobal( T alpha, const Matrix<T>& X, int i, int j );
    void AxpyGlobalToLocal( T alpha,       Matrix<T>& Y, int i, int j );

    int ReadyForSend
    ( int sendSize,
      std::vector<std::vector<char> >& sendVectors,
      std::vector<imports::mpi::Request>& requests, 
      std::vector<bool>& requestStatuses );

public:
    AxpyInterface( AxpyType axpyType, DistMatrix<T,MC,MR>& Z );
    void Attach( AxpyType axpyType, DistMatrix<T,MC,MR>& Z ); 
    void Axpy( T alpha, Matrix<T>& Z, int i, int j );

    // Even though axpyType == LOCAL_TO_GLOBAL is illegal for these 
    // routines, this approach was chosen to keep the calling code consistent
    // and will throw an error with the illegal option.
    AxpyInterface( AxpyType axpyType, const DistMatrix<T,MC,MR>& Z ); 
    void Attach( AxpyType axpyType, const DistMatrix<T,MC,MR>& Z ); 
    void Axpy( T alpha, const Matrix<T>& Z, int i, int j );

    AxpyInterface();
    ~AxpyInterface();
    void Detach();
};

//----------------------------------------------------------------------------//
// Implementation begins here                                                 //
//----------------------------------------------------------------------------//

template<typename T>
inline bool
AxpyInterface<T>::Finished()
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::Finished");
    if( !_attachedForLocalToGlobal && !_attachedForGlobalToLocal )
        throw std::logic_error("Not attached!");
#endif
    const Grid& g = ( _attachedForLocalToGlobal ? 
                      _localToGlobalMat->Grid() : 
                      _globalToLocalMat->Grid() );
    const int p = g.Size();

    bool finished = true; 
    for( int rank=0; rank<p; ++rank )
    {
        if( !_sentEomTo[rank] || !_haveEomFrom[rank] )
        {
            finished = false;
            break;
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
    return finished;
}

template<typename T>
inline void
AxpyInterface<T>::HandleEoms()
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::HandleEoms");
#endif
    const Grid& g = ( _attachedForLocalToGlobal ? 
                      _localToGlobalMat->Grid() : 
                      _globalToLocalMat->Grid() );
    const int p = g.Size();

    UpdateRequestStatuses();

    // Try to progress our EOM sends
    for( int i=0; i<p; ++i )
    {
        if( !_sentEomTo[i] )
        {
            bool shouldSendEom = true;
            for( int j=0; j<_sendingData[i].size(); ++j )
            {
                if( _sendingData[i][j] )
                {
                    shouldSendEom = false;
                    break;
                }
            }
            for( int j=0; j<_sendingRequest[i].size(); ++j )
            {
                if( !shouldSendEom || _sendingRequest[i][j] )
                {
                    shouldSendEom = false; 
                    break;
                }
            }
            for( int j=0; j<_sendingReply[i].size(); ++j )
            {
                if( !shouldSendEom || _sendingReply[i][j] )
                {
                    shouldSendEom = false;
                    break;
                }
            }
            if( shouldSendEom )
            {
                imports::mpi::Request& request = _eomSendRequests[i];
                imports::mpi::ISSend
                ( &_sendDummy, 1, i, EOM_TAG, g.VCComm(), request );
                _sentEomTo[i] = true;
            }
        }
    }

    int haveEom;
    imports::mpi::Status status;
    imports::mpi::IProbe
    ( imports::mpi::ANY_SOURCE, EOM_TAG, g.VCComm(), haveEom, status );

    if( haveEom )
    {
        const int source = status.MPI_SOURCE;
        imports::mpi::Recv( &_recvDummy, 1, source, EOM_TAG, g.VCComm() );
        _haveEomFrom[source] = true;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
AxpyInterface<T>::HandleLocalToGlobalData()
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::HandleLocalToGlobalData");
#endif
    using namespace elemental::imports;
    using namespace elemental::utilities;

    DistMatrix<T,MC,MR>& Y = *_localToGlobalMat;
    const Grid& g = Y.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int myRow = g.MCRank();
    const int myCol = g.MRRank();

    int haveData;
    mpi::Status status;
    mpi::IProbe( mpi::ANY_SOURCE, DATA_TAG, g.VCComm(), haveData, status );

    if( haveData )
    {
        // Message exists, so recv and pack    
        const int count = mpi::GetCount<char>( status );
        const int source = status.MPI_SOURCE;
        _recvVector.resize( count );
        char* recvBuffer = &_recvVector[0];
        mpi::Recv( recvBuffer, count, source, DATA_TAG, g.VCComm() );

        // Extract the header
        LocalToGlobalHeader header;
        std::memcpy( &header, recvBuffer, sizeof(LocalToGlobalHeader) );
        const int i = header.i;
        const int j = header.j;
        const int height = header.height;
        const int width = header.width;
#ifndef RELEASE
        if( height < 0 || width < 0 )
            throw std::logic_error("Unpacked heights were negative");
        if( i < 0 || j < 0 )
            throw std::logic_error("Offsets must be non-negative");
        if( i+height > Y.Height() || j+width > Y.Width() )
            throw std::logic_error("Submatrix out of bounds of global matrix");
#endif

        // Update Y
        const T* XBuffer = (const T*)&recvBuffer[sizeof(LocalToGlobalHeader)];
        const int colAlignment = (Y.ColAlignment()+i) % r;
        const int rowAlignment = (Y.RowAlignment()+j) % c;
        const int colShift = Shift( myRow, colAlignment, r );
        const int rowShift = Shift( myCol, rowAlignment, c );

        const int localHeight = LocalLength( height, colShift, r );
        const int localWidth = LocalLength( width, rowShift, c );
        const int iLocalOffset = LocalLength( header.i, Y.ColShift(), r );
        const int jLocalOffset = LocalLength( header.j, Y.RowShift(), c );

        const T alpha = header.alpha;
        for( int t=0; t<localWidth; ++t )
        {
            T* YCol = Y.LocalBuffer(iLocalOffset,jLocalOffset+t);
            const T* XCol = &XBuffer[t*localHeight];
            for( int s=0; s<localHeight; ++s )
                YCol[s] += alpha*XCol[s];
        }

        // Free the memory for the recv buffer
        _recvVector.clear();
    }
#ifndef RELEASE
    PopCallStack();
#endif
}
    
template<typename T>
inline void
AxpyInterface<T>::HandleGlobalToLocalRequest()
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::HandleGlobalToLocalRequest");
#endif
    using namespace elemental::imports;
    using namespace elemental::utilities;

    const DistMatrix<T,MC,MR>& X = *_globalToLocalMat;
    const Grid& g = X.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int myRow = g.MCRank();
    const int myCol = g.MRRank();

    int haveRequest;
    mpi::Status status;
    mpi::IProbe
    ( mpi::ANY_SOURCE, DATA_REQUEST_TAG, g.VCComm(), haveRequest, status );

    if( haveRequest )
    {
        // Request exists, so recv
        const int source = status.MPI_SOURCE;
        _recvVector.resize( sizeof(GlobalToLocalRequestHeader) );
        char* recvBuffer = &_recvVector[0];
        mpi::Recv
        ( recvBuffer, sizeof(GlobalToLocalRequestHeader),
          source, DATA_REQUEST_TAG, g.VCComm() );

        // Extract the header
        GlobalToLocalRequestHeader requestHeader;
        std::memcpy
        ( &requestHeader, recvBuffer, sizeof(GlobalToLocalRequestHeader) );
        const int i = requestHeader.i;
        const int j = requestHeader.j;
        const int height = requestHeader.height;
        const int width = requestHeader.width;

        const int colAlignment = (X.ColAlignment()+i) % r;
        const int rowAlignment = (X.RowAlignment()+j) % c;
        const int colShift = Shift( myRow, colAlignment, r );
        const int rowShift = Shift( myCol, rowAlignment, c );

        const int iLocalOffset = LocalLength( i, X.ColShift(), r );
        const int jLocalOffset = LocalLength( j, X.RowShift(), c );
        const int localHeight = LocalLength( height, colShift, r );
        const int localWidth = LocalLength( width, rowShift, c );
        const int numEntries = localHeight*localWidth;

        GlobalToLocalReplyHeader replyHeader;
        replyHeader.processRow = myRow;
        replyHeader.processCol = myCol;
        const int bufferSize = 
            numEntries*sizeof(T) + sizeof(GlobalToLocalReplyHeader);

        const int index = 
            ReadyForSend
            ( bufferSize, _replyVectors[source], 
              _replySendRequests[source], _sendingReply[source] );

        // Pack the reply header
        char* sendBuffer = &_replyVectors[source][index][0];
        std::memcpy
        ( sendBuffer, &replyHeader, sizeof(GlobalToLocalReplyHeader) );

        // Pack the payload
        T* sendData = (T*)&sendBuffer[sizeof(GlobalToLocalReplyHeader)];
        for( int t=0; t<localWidth; ++t )
        {
            T* sendCol = &sendData[t*localHeight];
            const T* XCol = X.LockedLocalBuffer(iLocalOffset,jLocalOffset+t);
            std::memcpy( sendCol, XCol, localHeight*sizeof(T) );
        }

        // Fire off non-blocking send
        mpi::ISSend
        ( sendBuffer, bufferSize, source, DATA_REPLY_TAG, g.VCComm(), 
          _replySendRequests[source][index] );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline
AxpyInterface<T>::AxpyInterface()
: _attachedForLocalToGlobal(false), _attachedForGlobalToLocal(false), 
  _localToGlobalMat(0), _globalToLocalMat(0)
{ }

template<typename T>
inline
AxpyInterface<T>::AxpyInterface( AxpyType axpyType, DistMatrix<T,MC,MR>& Z )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::AxpyInterface");
#endif
    if( axpyType == LOCAL_TO_GLOBAL )
    {
        _attachedForLocalToGlobal = true;
        _attachedForGlobalToLocal = false;
        _localToGlobalMat = &Z;
        _globalToLocalMat = 0;
    }
    else
    {
        _attachedForLocalToGlobal = false;
        _attachedForGlobalToLocal = true;
        _localToGlobalMat = 0;
        _globalToLocalMat = &Z;
    }

    const int p = Z.Grid().Size();
    _sentEomTo.resize( p, false );
    _haveEomFrom.resize( p, false );

    _sendingData.resize( p );
    _sendingRequest.resize( p );
    _sendingReply.resize( p );

    _dataVectors.resize( p );
    _requestVectors.resize( p );
    _replyVectors.resize( p );

    _dataSendRequests.resize( p );
    _requestSendRequests.resize( p );
    _replySendRequests.resize( p );

    _eomSendRequests.resize( p );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline
AxpyInterface<T>::AxpyInterface
( AxpyType axpyType, const DistMatrix<T,MC,MR>& X )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::AxpyInterface");
#endif
    if( axpyType == LOCAL_TO_GLOBAL )
    {
        throw std::logic_error("Cannot update a constant matrix");
    }
    else
    {
        _attachedForLocalToGlobal = false;
        _attachedForGlobalToLocal = true;
        _localToGlobalMat = 0;
        _globalToLocalMat = &X;
    }

    const int p = X.Grid().Size();
    _sentEomTo.resize( p, false );
    _haveEomFrom.resize( p, false );

    _sendingData.resize( p );
    _sendingRequest.resize( p );
    _sendingReply.resize( p );

    _dataVectors.resize( p );
    _requestVectors.resize( p );
    _replyVectors.resize( p );

    _dataSendRequests.resize( p );
    _requestSendRequests.resize( p );
    _replySendRequests.resize( p );

    _eomSendRequests.resize( p );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline
AxpyInterface<T>::~AxpyInterface()
{ 
    if( _attachedForLocalToGlobal || _attachedForGlobalToLocal )
    {
        if( std::uncaught_exception() )
        {
           const Grid& g = ( _attachedForLocalToGlobal ? 
                             _localToGlobalMat->Grid() : 
                             _globalToLocalMat->Grid() );
           std::ostringstream os;
           os << g.VCRank()
              << "Uncaught exception detected during AxpyInterface destructor "
                 "that required a call to Detach. Instead of allowing for the "
                 "possibility of Detach throwing another exception and "
                 "resulting in a 'terminate', we instead immediately dump the "
                 "call stack (if not in RELEASE mode) since the program will "
                 "likely hang:" << std::endl;
           std::cerr << os.str();
#ifndef RELEASE
           DumpCallStack();
#endif
        }
        else
        {
            Detach(); 
        }
    }
}

template<typename T>
inline void
AxpyInterface<T>::Attach( AxpyType axpyType, DistMatrix<T,MC,MR>& Z )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::Attach");
#endif
    if( _attachedForLocalToGlobal || _attachedForGlobalToLocal )
        throw std::logic_error("Must detach before reattaching.");

    if( axpyType == LOCAL_TO_GLOBAL )
    {
        _attachedForLocalToGlobal = true;
        _localToGlobalMat = &Z;
    }
    else
    {
        _attachedForGlobalToLocal = true;
        _globalToLocalMat = &Z;
    }

    const int p = Z.Grid().Size();
    _sentEomTo.resize( p, false );
    _haveEomFrom.resize( p, false );

    _sendingData.resize( p );
    _sendingRequest.resize( p );
    _sendingReply.resize( p );

    _dataVectors.resize( p );
    _requestVectors.resize( p );
    _replyVectors.resize( p );

    _dataSendRequests.resize( p );
    _requestSendRequests.resize( p );
    _replySendRequests.resize( p );

    _eomSendRequests.resize( p );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
AxpyInterface<T>::Attach( AxpyType axpyType, const DistMatrix<T,MC,MR>& X )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::Attach");
#endif
    if( _attachedForLocalToGlobal || _attachedForGlobalToLocal )
        throw std::logic_error("Must detach before reattaching.");

    if( axpyType == LOCAL_TO_GLOBAL )
    {
        throw std::logic_error("Cannot update a constant matrix");
    }
    else
    {
        _attachedForGlobalToLocal = true;
        _globalToLocalMat = &X;
    }

    const int p = X.Grid().Size();
    _sentEomTo.resize( p, false );
    _haveEomFrom.resize( p, false );

    _sendingData.resize( p );
    _sendingRequest.resize( p );
    _sendingReply.resize( p );

    _dataVectors.resize( p );
    _requestVectors.resize( p );
    _replyVectors.resize( p );

    _dataSendRequests.resize( p );
    _requestSendRequests.resize( p );
    _replySendRequests.resize( p );

    _eomSendRequests.resize( p );
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void 
AxpyInterface<T>::Axpy( T alpha, Matrix<T>& Z, int i, int j )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::Axpy");
#endif
    if( _attachedForLocalToGlobal )
        AxpyLocalToGlobal( alpha, Z, i, j );
    else if( _attachedForGlobalToLocal )
        AxpyGlobalToLocal( alpha, Z, i, j );
    else
        throw std::logic_error("Cannot axpy before attaching.");
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void 
AxpyInterface<T>::Axpy( T alpha, const Matrix<T>& Z, int i, int j )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::Axpy");
#endif
    if( _attachedForLocalToGlobal )
        AxpyLocalToGlobal( alpha, Z, i, j );
    else if( _attachedForGlobalToLocal )
        throw std::logic_error("Cannot update a constant matrix.");
    else
        throw std::logic_error("Cannot axpy before attaching.");
#ifndef RELEASE
    PopCallStack();
#endif
}

// Update Y(i:i+height-1,j:j+width-1) += alpha X, where X is height x width
template<typename T>
inline void
AxpyInterface<T>::AxpyLocalToGlobal
( T alpha, const Matrix<T>& X, int i, int j )
{
#ifndef RELEASE
    PushCallStack("axpy_interface::AxpyLocalToGlobal");
#endif
    using namespace elemental::imports;
    using namespace elemental::utilities;

    DistMatrix<T,MC,MR>& Y = *_localToGlobalMat;
    if( i < 0 || j < 0 )
        throw std::logic_error("Submatrix offsets must be non-negative");
    if( i+X.Height() > Y.Height() || j+X.Width() > Y.Width() )
        throw std::logic_error("Submatrix out of bounds of global matrix");

    const Grid& g = Y.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int p = g.Size();
    const int myProcessRow = g.MCRank();
    const int myProcessCol = g.MRRank();
    const int colAlignment = (Y.ColAlignment() + i) % r;
    const int rowAlignment = (Y.RowAlignment() + j) % c;

    const int height = X.Height();
    const int width = X.Width();

    // Fill the header
    LocalToGlobalHeader header;
    header.i = i;
    header.j = j;
    header.height = height;
    header.width = width;
    header.alpha = alpha;

    int receivingRow = myProcessRow;
    int receivingCol = myProcessCol;
    for( int step=0; step<p; ++step )
    {
        const int colShift = Shift( receivingRow, colAlignment, r );
        const int rowShift = Shift( receivingCol, rowAlignment, c );
        const int localHeight = LocalLength( height, colShift, r );
        const int localWidth = LocalLength( width, rowShift, c );
        const int numEntries = localHeight*localWidth;

        if( numEntries != 0 )
        {
            const int destination = receivingRow + r*receivingCol;
            const int bufferSize = 
                sizeof(LocalToGlobalHeader) + numEntries*sizeof(T);

            const int index = 
                ReadyForSend
                ( bufferSize, _dataVectors[destination], 
                  _dataSendRequests[destination], _sendingData[destination] );

            // Pack the header
            char* sendBuffer = &_dataVectors[destination][index][0];
            std::memcpy( sendBuffer, &header, sizeof(LocalToGlobalHeader) );

            // Pack the payload
            T* sendData = (T*)&sendBuffer[sizeof(LocalToGlobalHeader)];
            const T* XBuffer = X.LockedBuffer();
            const int XLDim = X.LDim();
            for( int t=0; t<localWidth; ++t )
            {
                T* thisSendCol = &sendData[t*localHeight];
                const T* thisXCol = &XBuffer[(rowShift+t*c)*XLDim];
                for( int s=0; s<localHeight; ++s )
                    thisSendCol[s] = thisXCol[colShift+s*r];
            }

            // Fire off the non-blocking send
            mpi::ISSend
            ( sendBuffer, bufferSize, destination, DATA_TAG, g.VCComm(), 
              _dataSendRequests[destination][index] );
        }

        receivingRow = (receivingRow + 1) % r;
        if( receivingRow == 0 )
            receivingCol = (receivingCol + 1) % c;
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Update Y += alpha X(i:i+height-1,j:j+width-1), where X is the dist-matrix
template<typename T>
inline void
AxpyInterface<T>::AxpyGlobalToLocal
( T alpha, Matrix<T>& Y, int i, int j )
{
#ifndef RELEASE
    PushCallStack("axpy_interface::AxpyGlobalToLocal");
#endif
    using namespace elemental::imports;
    using namespace elemental::utilities;

    const DistMatrix<T,MC,MR>& X = *_globalToLocalMat;

    const int height = Y.Height();
    const int width = Y.Width();
    if( i+height > X.Height() || j+width > X.Width() )
        throw std::logic_error("Invalid AxpyGlobalToLocal submatrix");

    const Grid& g = X.Grid();
    const int r = g.Height();
    const int c = g.Width();
    const int p = g.Size();

    // Fill the request header
    GlobalToLocalRequestHeader requestHeader;
    requestHeader.i = i;
    requestHeader.j = j;
    requestHeader.height = height;
    requestHeader.width = width;

    // Send out the requests to all processes in the grid
    for( int rank=0; rank<p; ++rank )
    {
        const int bufferSize = sizeof(GlobalToLocalRequestHeader);
        const int index = 
            ReadyForSend
            ( bufferSize, _requestVectors[rank], 
              _requestSendRequests[rank], _sendingRequest[rank] );

        // Copy the request header into the send buffer
        char* sendBuffer = &_requestVectors[rank][index][0];
        std::memcpy( sendBuffer, &requestHeader, bufferSize );

        // Begin the non-blocking send
        mpi::ISSend
        ( sendBuffer, bufferSize, rank, DATA_REQUEST_TAG, g.VCComm(), 
          _requestSendRequests[rank][index] );
    }

    // Receive all of the replies
    int numReplies = 0;
    while( numReplies < p )
    {
        HandleGlobalToLocalRequest();

        int haveReply;
        mpi::Status status;
        mpi::IProbe
        ( mpi::ANY_SOURCE, DATA_REPLY_TAG, g.VCComm(), haveReply, status );

        if( haveReply )
        {
            const int source = status.MPI_SOURCE;

            // Ensure that we have a recv buffer
            const int count = mpi::GetCount<char>( status );
            _recvVector.resize( count );
            char* recvBuffer = &_recvVector[0];

            // Receive the data
            mpi::Recv( recvBuffer, count, source, DATA_REPLY_TAG, g.VCComm() );

            // Unpack the reply header
            GlobalToLocalReplyHeader replyHeader;
            std::memcpy
            ( &replyHeader, recvBuffer, sizeof(GlobalToLocalReplyHeader) );
            const int row = replyHeader.processRow;
            const int col = replyHeader.processCol;
            const T* recvData = 
                (const T*)&recvBuffer[sizeof(GlobalToLocalReplyHeader)];

            // Compute the local heights and offsets
            const int colAlignment = (X.ColAlignment()+i) % r;
            const int rowAlignment = (X.RowAlignment()+j) % c;
            const int colShift = Shift( row, colAlignment, r );
            const int rowShift = Shift( col, rowAlignment, c );
            const int localHeight = LocalLength( height, colShift, r );
            const int localWidth = LocalLength( width, rowShift, c );

            // Unpack the local matrix
            for( int t=0; t<localWidth; ++t )
            {
                T* YCol = Y.Buffer(0,rowShift+t*c);
                const T* XCol = &recvData[t*localHeight];
                for( int s=0; s<localHeight; ++s )
                    YCol[colShift+s*r] += alpha*XCol[s];
            }

            ++numReplies;
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline int
AxpyInterface<T>::ReadyForSend
( int sendSize,
  std::vector<std::vector<char> >& sendVectors,
  std::vector<imports::mpi::Request>& requests, 
  std::vector<bool>& requestStatuses )
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::ReadyForSend");
#endif
    const int numCreated = sendVectors.size();
#ifndef RELEASE
    if( numCreated != requests.size() || numCreated != requestStatuses.size() )
        throw std::logic_error("size mismatch");
#endif
    for( int i=0; i<numCreated; ++i )
    {
        // If this request is still running, test to see if it finished.
        if( requestStatuses[i] )
        {
            const bool finished = imports::mpi::Test( requests[i] );
            requestStatuses[i] = !finished;
        }

        if( !requestStatuses[i] )    
        {
            requestStatuses[i] = true;
            sendVectors[i].resize( sendSize );
#ifndef RELEASE
            PopCallStack();
#endif
            return i;
        }
    }

    sendVectors.resize( numCreated+1 );
    sendVectors[numCreated].resize( sendSize );
    requests.resize( numCreated+1 );
    requestStatuses.push_back( true );

#ifndef RELEASE
    PopCallStack();
#endif
    return numCreated;
}

template<typename T>
inline void
AxpyInterface<T>::UpdateRequestStatuses()
{
#ifndef RELEASE
    PushCallStack("AxpyInterface::UpdateRequestStatuses");
#endif
    const Grid& g = ( _attachedForLocalToGlobal ? 
                      _localToGlobalMat->Grid() : 
                      _globalToLocalMat->Grid() );
    const int p = g.Size();

    for( int i=0; i<p; ++i )
    {
        for( int j=0; j<_dataSendRequests[i].size(); ++j )
            if( _sendingData[i][j] )
                _sendingData[i][j] = 
                    !imports::mpi::Test( _dataSendRequests[i][j] );
        for( int j=0; j<_requestSendRequests[i].size(); ++j )
            if( _sendingRequest[i][j] )
                _sendingRequest[i][j] = 
                    !imports::mpi::Test( _requestSendRequests[i][j] );
        for( int j=0; j<_replySendRequests[i].size(); ++j )
            if( _sendingReply[i][j] )
                _sendingReply[i][j] = 
                    !imports::mpi::Test( _replySendRequests[i][j] );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template<typename T>
inline void
AxpyInterface<T>::Detach()
{
#ifndef RELEASE    
    PushCallStack("AxpyInterface::Detach");
#endif
    if( !_attachedForLocalToGlobal && !_attachedForGlobalToLocal )
        throw std::logic_error("Must attach before detaching.");

    const Grid& g = ( _attachedForLocalToGlobal ? 
                      _localToGlobalMat->Grid() : 
                      _globalToLocalMat->Grid() );

    while( !Finished() )
    {
        if( _attachedForLocalToGlobal )
            HandleLocalToGlobalData();
        else
            HandleGlobalToLocalRequest();
        HandleEoms();
    }

    imports::mpi::Barrier( g.VCComm() );

    _attachedForLocalToGlobal = false;
    _attachedForGlobalToLocal = false;
    _recvVector.clear();
    _sentEomTo.clear();
    _haveEomFrom.clear();

    _sendingData.clear();
    _sendingRequest.clear();
    _sendingReply.clear();

    _dataVectors.clear();
    _requestVectors.clear();
    _replyVectors.clear();
    
    _dataSendRequests.clear();
    _requestSendRequests.clear();
    _replySendRequests.clear();

    _eomSendRequests.clear();
#ifndef RELEASE
    PopCallStack();
#endif
}

} // elemental

#endif  /* ELEMENTAL_AXPY_INTERFACE_HPP */

