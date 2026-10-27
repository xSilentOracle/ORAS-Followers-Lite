#include "../lite/PicaStateAudit.hpp"
#include <cassert>
#include <cstdio>
#include <array>
using namespace PicaStateAudit;
using namespace PicaPackets;
int main(){
 Shadow game{};
 std::array<uint32_t,64> b{};
 Writer base(b.data(),b.size());
 assert(base.Write(0x80,0x11223344));
 assert(base.Write(0x81,0xAABBCCDD));
 assert(base.Write(0x82,0xFF000000u,0x8));
 assert(Observe(game,b.data(),base.SizeWords()));
 assert(game.values[0x80]==0x11223344u);
 assert(game.known_bits[0x80]==0xFFFFFFFFu);
 assert(game.known_bits[0x82]==0xFF000000u);
 Writer sprite(b.data(),b.size());
 assert(sprite.Write(0x80,0xABCDEF12u));
 assert(sprite.Write(0x82,0xBB000000u,0x8));
 auto safe=AnalyzeRestore(game,b.data(),sprite.SizeWords());
 assert(safe.parsed && safe.all_registers_known && safe.restorable);
 assert(!safe.needs_shader_reupload && !safe.has_irreversible_effects);
 Writer missing(b.data(),b.size());
 assert(missing.Write(0x99,0x10101010u));
 auto unknown=AnalyzeRestore(game,b.data(),missing.SizeWords());
 assert(unknown.parsed && !unknown.all_registers_known);
 assert(unknown.first_unknown_register==0x99 && !unknown.restorable);
 Writer shader(b.data(),b.size());
 assert(shader.Write(0x02CC,0x1234u));
 auto upload=AnalyzeRestore(game,b.data(),shader.SizeWords());
 assert(upload.needs_shader_reupload && !upload.restorable);
 Writer trigger(b.data(),b.size());
 assert(trigger.Write(0x022E,1));
 auto draw=AnalyzeRestore(game,b.data(),trigger.SizeWords());
 assert(draw.has_irreversible_effects && !draw.restorable);
 // An incorrectly truncated packet is rejected without partial state changes.
 Shadow before=game;
 uint32_t bad[]={0xDEADBEEFu, Header(0x80,0xFu,2,true),0x11111111u};
 assert(!Observe(game,bad,3));
 assert(game.values[0x80]==before.values[0x80]);
 assert(game.known_bits[0x80]==before.known_bits[0x80]);
 uint32_t invalid[]={0x123u,Header(0x3FF,0xFu,1,true),0x456u,0u};
 assert(!Observe(game,invalid,4));
 // Masked writes only restore the explicitly known bytes.
 Writer partial(b.data(),b.size());
 assert(partial.Write(0x82,0x000000FFu,0x1));
 assert(Observe(game,b.data(),partial.SizeWords()));
 assert(game.known_bits[0x82]==0xFF0000FFu);
 assert(game.values[0x82]==0xFF0000FFu);
 // Read-back shadow is not proof of actual GPU state.
 std::puts("PICA state audit: masked tracking, atomic parse and unsafe restore checks pass");
}
