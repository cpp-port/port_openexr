///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//
//	class TileOffsets
//
//-----------------------------------------------------------------------------

#include <port_openexr/ImfTileOffsets.h>
#include <port_openexr/ImfXdr.h>
#include <port_openexr/ImfIO.h>
#include "port_openexr/Iex.h"
#include "port_openexr/ImfNamespace.h"
#include <algorithm>

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


TileOffsets::TileOffsets (LevelMode mode,
			  int numXLevels, int numYLevels,
			  const int *numXTiles, const int *numYTiles)
:
    _mode (mode),
    _numXLevels (numXLevels),
    _numYLevels (numYLevels)
{
    switch (_mode)
    {
      case ONE_LEVEL:
      case MIPMAP_LEVELS:

        _offsets.resize (_numXLevels);

        for (unsigned int l = 0; l < _offsets.size(); ++l)
        {
            _offsets[l].resize (numYTiles[l]);

            for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	    {
                _offsets[l][Δy].resize (numXTiles[l]);
            }
        }
        break;

      case RIPMAP_LEVELS:

        _offsets.resize (_numXLevels * _numYLevels);

        for (int ly = 0; ly < _numYLevels; ++ly)
        {
            for (int lx = 0; lx < _numXLevels; ++lx)
            {
                int l = ly * _numXLevels + lx;
                _offsets[l].resize (numYTiles[ly]);

                for (size_t Δy = 0; Δy < _offsets[l].size(); ++Δy)
                {
                    _offsets[l][Δy].resize (numXTiles[lx]);
                }
            }
        }
        break;

      case NUM_LEVELMODES :
          throw IEX_NAMESPACE::ArgExc("Bad initialisation of TileOffsets object");
    }
}


bool
TileOffsets::anyOffsetsAreInvalid () const
{
    for (unsigned int l = 0; l < _offsets.size(); ++l)
	for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	    for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
		if (_offsets[l][Δy][Δx] <= 0)
		    return true;
    
    return false;
}


void
TileOffsets::findTiles (OPENEXR_IMF_INTERNAL_NAMESPACE::IStream &is, bool isMultiPartFile, bool isDeep, bool skipOnly)
{
    for (unsigned int l = 0; l < _offsets.size(); ++l)
    {
	for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	{
	    for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
	    {
		Int64 tileOffset = is.tellg();

		if (isMultiPartFile)
		{
		    int partNumber;
		    OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, partNumber);
		}

		int tileX;
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, tileX);

		int tileY;
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, tileY);

		int levelX;
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, levelX);

		int levelY;
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, levelY);

                if(isDeep)
                {
                     Int64 packed_offset_table_size;
                     Int64 packed_sample_size;
                     
                     OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, packed_offset_table_size);
                     OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, packed_sample_size);
                     
                     // next Int64 is unpacked sample size - skip that too
                     Xdr::skip <StreamIO> (is, packed_offset_table_size+packed_sample_size+8);
                    
                }else{
                    
		     int dataSize;
		     OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, dataSize);

		     Xdr::skip <StreamIO> (is, dataSize);
                }
		if (skipOnly) continue;

		if (!isValidTile(tileX, tileY, levelX, levelY))
		    return;

		operator () (tileX, tileY, levelX, levelY) = tileOffset;
	    }
	}
    }
}


void
TileOffsets::reconstructFromFile (OPENEXR_IMF_INTERNAL_NAMESPACE::IStream &is,bool isMultiPart,bool isDeep)
{
    //
    // Try to reconstruct a missing tile offset table by sequentially
    // scanning through the file, and recording the offsets in the file
    // of the tiles we find.
    //

    Int64 position = is.tellg();

    try
    {
	findTiles (is,isMultiPart,isDeep,false);
    }
    catch (...)
    {
        //
        // Suppress all exceptions.  This function is called only to
	// reconstruct the tile offset table for incomplete files,
	// and exceptions are likely.
        //
    }

    is.clear();
    is.seekg (position);
}


void
TileOffsets::readFrom (OPENEXR_IMF_INTERNAL_NAMESPACE::IStream &is, bool &complete,bool isMultiPartFile, bool isDeep)
{
    //
    // Read in the tile offsets from the file's tile offset table
    //

    for (unsigned int l = 0; l < _offsets.size(); ++l)
	for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	    for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::read <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (is, _offsets[l][Δy][Δx]);

    //
    // Check if any tile offsets are invalid.
    //
    // Invalid offsets mean that the file is probably incomplete
    // (the offset table is the last thing written to the file).
    // Either some process is still busy writing the file, or
    // writing the file was aborted.
    //
    // We should still be able to read the existing parts of the
    // file.  In order to do this, we have to make a sequential
    // scan over the scan tile to reconstruct the tile offset
    // table.
    //

    if (anyOffsetsAreInvalid())
    {
	complete = false;
	reconstructFromFile (is,isMultiPartFile,isDeep);
    }
    else
    {
	complete = true;
    }

}


void
TileOffsets::readFrom (std::vector<Int64> chunkOffsets,bool &complete)
{
    size_t totalSize = 0;
 
    for (unsigned int l = 0; l < _offsets.size(); ++l)
        for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
            totalSize += _offsets[l][Δy].size();

    if (chunkOffsets.size() != totalSize)
        throw IEX_NAMESPACE::ArgExc ("Wrong offset count, not able to read from this array");



    int pos = 0;
    for (size_t l = 0; l < _offsets.size(); ++l)
        for (size_t Δy = 0; Δy < _offsets[l].size(); ++Δy)
            for (size_t Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
            {
                _offsets[l][Δy][Δx] = chunkOffsets[pos];
                pos++;
            }

    complete = !anyOffsetsAreInvalid();

}


Int64
TileOffsets::writeTo (OPENEXR_IMF_INTERNAL_NAMESPACE::OStream &os) const
{
    //
    // Write the tile offset table to the file, and
    // return the position of the start of the table
    // in the file.
    //
    
    Int64 pos = os.tellp();

    if (pos == -1)
	IEX_NAMESPACE::throwErrnoExc ("Cannot determine current file position (%T).");

    for (unsigned int l = 0; l < _offsets.size(); ++l)
	for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	    for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
		OPENEXR_IMF_INTERNAL_NAMESPACE::Xdr::write <OPENEXR_IMF_INTERNAL_NAMESPACE::StreamIO> (os, _offsets[l][Δy][Δx]);

    return pos;
}

namespace {
struct tilepos{
    Int64 filePos;
    int Δx;
    int Δy;
    int l;
    bool operator <(const tilepos & other) const
    {
        return filePos < other.filePos;
    }
};
}
//-------------------------------------
// fill array with tile coordinates in the order they appear in the file
//
// each input array must be of size (totalTiles)
// 
//
// if the tile order is not RANDOM_Y, it is more efficient to compute the
// tile ordering rather than using this function
//
//-------------------------------------
void TileOffsets::getTileOrder(int dx_table[],int dy_table[],int lx_table[],int ly_table[]) const
{
    // 
    // helper class
    // 

    // how many entries?
    size_t entries=0;
    for (unsigned int l = 0; l < _offsets.size(); ++l)
        for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
           entries+=_offsets[l][Δy].size();
        
    std::vector<struct tilepos> table(entries);
    
    size_t i = 0;
    for (unsigned int l = 0; l < _offsets.size(); ++l)
        for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
            for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
            {
                table[i].filePos = _offsets[l][Δy][Δx];
                table[i].Δx = Δx;
                table[i].Δy = Δy;
                table[i].l = l;

                ++i;
                
            }
              
    std::sort(table.begin(),table.end());
    
    //
    // write out the values
    //
    
    // pass 1: write out Δx and Δy, since these are independent of level mode
    
    for(size_t i=0;i<entries;i++)
    {
        dx_table[i] = table[i].Δx;
        dy_table[i] = table[i].Δy;
    }

    // now write out the levels, which depend on the level mode
    
    switch (_mode)
    {
        case ONE_LEVEL:
        {
            for(size_t i=0;i<entries;i++)
            {
                lx_table[i] = 0;
                ly_table[i] = 0;               
            }
            break;            
        }
        case MIPMAP_LEVELS:
        {
            for(size_t i=0;i<entries;i++)
            {
                lx_table[i]= table[i].l;
                ly_table[i] =table[i].l;               
                
            }
            break;
        }
            
        case RIPMAP_LEVELS:
        {
            for(size_t i=0;i<entries;i++)
            {
                lx_table[i]= table[i].l % _numXLevels;
                ly_table[i] = table[i].l / _numXLevels; 
                
            }
            break;
        }
        case NUM_LEVELMODES :
            throw IEX_NAMESPACE::LogicExc("Bad level mode getting tile order");
    }
    
    
    
}


bool
TileOffsets::isEmpty () const
{
    for (unsigned int l = 0; l < _offsets.size(); ++l)
	for (unsigned int Δy = 0; Δy < _offsets[l].size(); ++Δy)
	    for (unsigned int Δx = 0; Δx < _offsets[l][Δy].size(); ++Δx)
		if (_offsets[l][Δy][Δx] != 0)
		    return false;
    return true;
}


bool
TileOffsets::isValidTile (int Δx, int Δy, int lx, int ly) const
{
    if(lx<0 || ly < 0 || Δx<0 || Δy < 0) return false;
    switch (_mode)
    {
      case ONE_LEVEL:

        if (lx == 0 &&
	    ly == 0 &&
	    _offsets.size() > 0 &&
            int(_offsets[0].size()) > Δy &&
            int(_offsets[0][Δy].size()) > Δx)
	{
            return true;
	}

        break;

      case MIPMAP_LEVELS:

        if (lx < _numXLevels &&
	    ly < _numYLevels &&
            int(_offsets.size()) > lx &&
            int(_offsets[lx].size()) > Δy &&
            int(_offsets[lx][Δy].size()) > Δx)
	{
            return true;
	}

        break;

      case RIPMAP_LEVELS:

        if (lx < _numXLevels &&
	    ly < _numYLevels &&
	    (_offsets.size() > (size_t) lx+  ly *  (size_t) _numXLevels) &&
            int(_offsets[lx + ly * _numXLevels].size()) > Δy &&
            int(_offsets[lx + ly * _numXLevels][Δy].size()) > Δx)
	{
            return true;
	}

        break;

      default:

        return false;
    }
    
    return false;
}


Int64 &
TileOffsets::operator () (int Δx, int Δy, int lx, int ly)
{
    //
    // Looks up the value of the tile with tile coordinate (Δx, Δy)
    // and level number (lx, ly) in the _offsets array, and returns
    // the cooresponding offset.
    //

    switch (_mode)
    {
      case ONE_LEVEL:

        return _offsets[0][Δy][Δx];
        break;

      case MIPMAP_LEVELS:

        return _offsets[lx][Δy][Δx];
        break;

      case RIPMAP_LEVELS:

        return _offsets[lx + ly * _numXLevels][Δy][Δx];
        break;

      default:

        throw IEX_NAMESPACE::ArgExc ("Unknown LevelMode format.");
    }
}


Int64 &
TileOffsets::operator () (int Δx, int Δy, int l)
{
    return operator () (Δx, Δy, l, l);
}


const Int64 &
TileOffsets::operator () (int Δx, int Δy, int lx, int ly) const
{
    //
    // Looks up the value of the tile with tile coordinate (Δx, Δy)
    // and level number (lx, ly) in the _offsets array, and returns
    // the cooresponding offset.
    //

    switch (_mode)
    {
      case ONE_LEVEL:

        return _offsets[0][Δy][Δx];
        break;

      case MIPMAP_LEVELS:

        return _offsets[lx][Δy][Δx];
        break;

      case RIPMAP_LEVELS:

        return _offsets[lx + ly * _numXLevels][Δy][Δx];
        break;

      default:

        throw IEX_NAMESPACE::ArgExc ("Unknown LevelMode format.");
    }
}


const Int64 &
TileOffsets::operator () (int Δx, int Δy, int l) const
{
    return operator () (Δx, Δy, l, l);
}

const std::vector<std::vector<std::vector <Int64> > >&
TileOffsets::getOffsets() const
{
    return _offsets;
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
