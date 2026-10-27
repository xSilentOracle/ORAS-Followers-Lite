#include "../lite/PicaStateRestore.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace PicaPackets;
using namespace PicaStateAudit;
using namespace PicaStateRestore;
int main(){
 Shadow before{};
 std::array<uint32_t,128> stage{};
 Writer game(stage.data(),stage.size());
 assert(game.Write(0x080,0xABCDEF01u));
 assert(game.Write(0x081,0xCAFEBABEu));
 assert(game.Write(0x082,0x88110011u,0xAu));
 assert(Observe(before,stage.data(),game.SizeWords()));
 Writer sprite(stage.data(),stage.size());
 assert(sprite.Write(0x080,0x01020304u,0xFu));
 assert(sprite.Write(0x080,0xFFFFFFFFu,0x3u));
 assert(sprite.Write(0x081,0x11111111u,0xFu));
 assert(sprite.Write(0x082,0x22992200u,0xAu));
 std::array<uint32_t,128> restore{};
 auto result=Build(before,stage.data(),sprite.SizeWords(),restore.data(),restore.size());
 assert(result.ready && result.modified_registers==3);
 assert(result.words%4==0);
 // Replay planned sprite changes followed by the generated restore commands
 // into a shadow. Every affected known byte must return to its baseline.
 Shadow after=before;
 assert(Observe(after,stage.data(),sprite.SizeWords()));
 assert(Observe(after,restore.data(),result.words));
 assert(after.values[0x080]==before.values[0x080]);
 assert(after.values[0x081]==before.values[0x081]);
 assert((after.values[0x082]&0xFF00FF00u)==(before.values[0x082]&0xFF00FF00u));
 std::array<uint32_t,2> tiny{};
 assert(!Build(before,stage.data(),sprite.SizeWords(),tiny.data(),tiny.size()).ready);
 Writer upload(stage.data(),stage.size());
 assert(upload.Write(0x02CC,0x11111111u));
 assert(!Build(before,stage.data(),upload.SizeWords(),restore.data(),restore.size()).ready);
 Writer drawing(stage.data(),stage.size());
 assert(drawing.Write(0x022E,1u));
 assert(!Build(before,stage.data(),drawing.SizeWords(),restore.data(),restore.size()).ready);
 Writer missing(stage.data(),stage.size());
 assert(missing.Write(0x099,0x11111111u));
 assert(!Build(before,stage.data(),missing.SizeWords(),restore.data(),restore.size()).ready);
 std::puts("PICA restore builder: replays masked state, blocks shader and draw side effects");
}
