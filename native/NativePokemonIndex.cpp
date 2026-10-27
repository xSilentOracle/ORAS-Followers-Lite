// SPDX-License-Identifier: MIT
#include "NativePokemonIndex.hpp"
#include <cstdint>
namespace NativePokemonIndex {
ModelFiles Resolve(const SpeciesEntry* entries,size_t count,Request q,
                   uint32_t archive_file_count) {
 ModelFiles out{};
 if(!entries||count<kNationalDexCount||q.species==0 ||
    q.species>kNationalDexCount)return out;
 const SpeciesEntry& sp=entries[q.species-1];
 if(!sp.variant_count)return out;
 const bool sex=(sp.flags&2u)!=0;
 const bool forms=(sp.flags&4u)!=0;
 uint32_t variant=0;
 if(forms&&q.form!=0u)variant=q.form+(sex?1u:0u);
 else if(sex&&q.female)variant=1u;
 if(variant>=sp.variant_count)variant=0u;
 const uint32_t index=uint32_t(sp.first_variant)+variant;
 // Prevent arithmetic wrap and any reference outside a verified game archive.
 if(index>(UINT32_MAX-kFirstVariantFile)/kPackFilesPerVariant)return out;
 const uint32_t start=kFirstVariantFile+kPackFilesPerVariant*index;
 constexpr uint32_t kNormalTextureOffset=2u,kShinyTextureOffset=3u;
 const uint32_t tex=start+(q.shiny?kShinyTextureOffset:kNormalTextureOffset);
 if(start>=archive_file_count||tex>=archive_file_count)return out;
 out.model=start;
 out.texture=tex;
 out.pack_start=start;
 out.valid=true;
 return out;
}
}
