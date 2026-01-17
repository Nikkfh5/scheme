#include <parser.h>
static Heap default_heap;
static Heap* current_heap = &default_heap;

void SetCurrentHeap(Heap* h) {
    if (h) {
        current_heap = h;
    } else {
        current_heap = &default_heap;
    }
}
Object* ReadListWrap(Tokenizer* tokenizer, Heap& heap) {
    Token token = tokenizer->GetToken();
    if (BracketToken* t = std::get_if<BracketToken>(&token)) {
        if (*t == BracketToken::CLOSE) {
            tokenizer->Next();
            return nullptr;
        }
    }
    Object* first = ReadWrap(tokenizer,heap);
    Token next_token = tokenizer->GetToken();
    if (std::get_if<DotToken>(&next_token)) {
        tokenizer->Next();
        Object* second = ReadWrap(tokenizer, heap);
        Token closing = tokenizer->GetToken();
        BracketToken* x = std::get_if<BracketToken>(&closing);
        if (!x || *x != BracketToken::CLOSE) {
            throw SyntaxError("");
        }
        tokenizer->Next();
        return heap.Make<Cell>(first, second);
    } else {
        Object* next = ReadListWrap(tokenizer, heap);
        return heap.Make<Cell>(first, next);
    }
};
Object* ReadWrap(Tokenizer* tokenizer, Heap& heap) {
    Token token = tokenizer->GetToken();
    if (ConstantToken* c = std::get_if<ConstantToken>(&token)) {
        tokenizer->Next();
        return heap.Make<Number>(c->value);
    }

    if (SymbolToken* s = std::get_if<SymbolToken>(&token)) {
        tokenizer->Next();

        return heap.Make<Symbol>(s->name);
    }
    if (BooleanToken* b = std::get_if<BooleanToken>(&token)) {
        tokenizer->Next();
        return heap.Make<Boolean>(b->f);
    }
    if (std::get_if<QuoteToken>(&token)) {
        tokenizer->Next();
        Object* quote = ReadWrap(tokenizer,heap);

        Symbol* first = heap.Make<Symbol>("quote");
        Cell* second = heap.Make<Cell>(quote, nullptr);
        return heap.Make<Cell>(first, second);
    }
    if (BracketToken* t = std::get_if<BracketToken>(&token)) {
        tokenizer->Next();
        if (*t == BracketToken::OPEN) {
            return ReadListWrap(tokenizer,heap);
        }
        throw SyntaxError("");
    }
    throw SyntaxError("");
};
Object* Read(Tokenizer* tokenizer) {
    return ReadWrap(tokenizer, *current_heap);
}
