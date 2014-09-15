#include "uint128.ih"

namespace Dedekind
{
	UInt128 &UInt128::operator*=(uint_fast64_t other)
	{
		UInt128 temp(*this);
		d_lo =0; d_hi=0;
		for(uint_fast64_t i=0;i<64;i++){
			if((other&1)!=0){
				*this+= (temp.d_lo<<i);
				d_hi+= (temp.d_hi<<i);
				if ( i!= 0) //can't bitshift 64 bits
					d_hi+= (temp.d_lo>>(64-i));
			}
			other>>=1;	
		}
		return *this;
	}
}
