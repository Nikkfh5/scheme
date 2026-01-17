#pragma once

#include <memory>

#include <object.h>
#include <tokenizer.h>
Object* ReadListWrap(Tokenizer* tokenizer,Heap& heap);
Object* ReadWrap(Tokenizer* tokenizer,Heap& heap);
Object* Read(Tokenizer* tokenizer);
void SetCurrentHeap(Heap* h);
