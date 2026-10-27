#include "../native/NativePokemonIndex.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace NativePokemonIndex;
int main(){
 std::vector<SpeciesEntry> species(kNationalDexCount);
 for(unsigned i=0;i<kNationalDexCount;++i)
   species[i]={uint16_t(i*3),3,0};
 const uint32_t max=(kFirstVariantFile+8u*(3u*kNationalDexCount)+8u);
 for(unsigned id=1;id<=kNationalDexCount;++id){
   auto a=Resolve(species.data(),species.size(),{uint16_t(id),0,false,false},max);
   auto b=Resolve(species.data(),species.size(),{uint16_t(id),0,false,true},max);
   assert(a.valid&&b.valid);
   assert(a.model==b.model);
   assert(b.texture==a.texture+1);
   assert(a.model < max&&b.texture < max);
 }
 assert(!Resolve(species.data(),species.size(),{0,0,false,false},max).valid);
 assert(!Resolve(species.data(),species.size(),{722,0,false,false},max).valid);
 assert(!Resolve(species.data(),species.size(),{721,0,false,false},7).valid);
 species[24]={2000,3,6}; // Pikachu: female variant and form
 auto pikachu=Resolve(species.data(),species.size(),{25,0,true,false},max);
 assert(pikachu.valid&&pikachu.pack_start==(2001u*8u+3u));
 auto form=Resolve(species.data(),species.size(),{25,1,false,false},max);
 assert(form.valid&&form.pack_start==(2002u*8u+3u));
 auto fallback=Resolve(species.data(),species.size(),{25,99,false,false},max);
 assert(fallback.valid&&fallback.pack_start==(2000u*8u+3u));
 species[24].variant_count=0;
 assert(!Resolve(species.data(),species.size(),{25,0,false,false},max).valid);
 std::puts("721 Pokémon archive model IDs: validated species, forms, gender, shiny, bounds.");
}
