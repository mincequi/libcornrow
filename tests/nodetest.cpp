#include <coro/audio/AppSource.h>
#include <coro/audio/SbcDecoder.h>

using namespace coro::audio;

int main()
{
    AppSource source;
    SbcDecoder decoder;

    Node::link(source, decoder); // compiles
    //Node::link(node1, dec);
    //Node::link(node1, node3); // fails to compile
    //Node::link(node1, node4); // fails to compile
}

