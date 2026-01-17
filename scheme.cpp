#include "scheme.h"
#include "parser.h"

#include <complex>
#include <sstream>
#include <error.h>
#include <numeric>
std::string Serialize(Object* result);
Object* Eval(Object* expr, Enviromnent& env) {
    if (!expr) {
        throw RuntimeError("");
    }
    return expr->Eval(env);
}
template <class Func>
Object* Accumulate(const std::vector<Object*>& args, Enviromnent& env, int64_t start,
                   int start_index, Func func) {
    if (args.empty()) {
        throw RuntimeError("");
    }
    int64_t acc = start;
    for (int i = start_index; i < args.size(); ++i) {
        Object* val = Eval(args[i], env);
        Number* n = As<Number>(val);
        if (!n) {
            throw RuntimeError("");
        }
        acc = func(acc, n->GetValue());
    }
    return env.heap_->Make<Number>(acc);
}
template <class Func>
Object* AccumulateBool(const std::vector<Object*>& args, Enviromnent& env, Func func) {
    Object* first_val = Eval(args[0], env);
    Number* first_num = As<Number>(first_val);
    if (!first_num) {
        throw RuntimeError("");
    }
    int64_t prev = first_num->GetValue();
    for (int i = 1; i < args.size(); ++i) {
        Object* val = Eval(args[i], env);
        Number* n = As<Number>(val);
        if (!n) {
            throw RuntimeError("");
        }
        int64_t cur = n->GetValue();
        if (!func(prev, cur)) {
            return env.heap_->Make<Boolean>(false);
        }
        prev = cur;
    }

    return env.heap_->Make<Boolean>(true);
}
Object* Plus(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Number>(0);
    }
    return Accumulate(args, env, 0, 0, std::plus<int64_t>());
}
Object* Mul(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Number>(1);
    }
    return Accumulate(args, env, 1, 0, std::multiplies<int64_t>());
}
Object* Minus(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        throw RuntimeError("");
    }
    Object* first_val = Eval(args[0], env);
    Number* first_num = As<Number>(first_val);
    if (!first_num) {
        throw RuntimeError("");
    }
    int64_t start = first_num->GetValue();
    if (args.size() == 1) {
        return env.heap_->Make<Number>(-start);
    }
    return Accumulate(args, env, start, 1, std::minus<int64_t>());
}
Object* Del(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        throw RuntimeError("");
    }
    Object* first_val = Eval(args[0], env);
    Number* first_num = As<Number>(first_val);
    if (!first_num) {
        throw RuntimeError("");
    }
    int64_t start = first_num->GetValue();
    if (args.size() == 1) {
        return env.heap_->Make<Number>(1 / start);
    }
    return Accumulate(args, env, start, 1, std::divides<int64_t>());
}
Object* Max(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        throw RuntimeError("");
    }
    Number* first = As<Number>(Eval(args[0], env));
    if (!first) {
        throw RuntimeError("");
    }
    int64_t best = first->GetValue();
    for (int i = 1; i < args.size(); ++i) {
        Object* val = Eval(args[i], env);
        Number* n = As<Number>(val);
        if (!n) {
            throw RuntimeError("");
        }
        if (n->GetValue() > best) {
            best = n->GetValue();
        }
    }
    return env.heap_->Make<Number>(best);
}
Object* Min(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        throw RuntimeError("");
    }
    Number* first = As<Number>(Eval(args[0], env));
    if (!first) {
        throw RuntimeError("");
    }
    int64_t best = first->GetValue();
    for (int i = 1; i < args.size(); ++i) {
        Number* n = As<Number>(Eval(args[i], env));
        if (!n) {
            throw RuntimeError("");
        }
        if (n->GetValue() < best) {
            best = n->GetValue();
        }
    }
    return env.heap_->Make<Number>(best);
}
Object* Abs(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Number* n = As<Number>(Eval(args[0], env));
    if (!n) {
        throw RuntimeError("");
    }
    return env.heap_->Make<Number>(std::abs(n->GetValue()));
}
Object* IsNumber(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Number* n = As<Number>(Eval(args[0], env));
    if (!n) {
        return env.heap_->Make<Boolean>(false);
    }
    return env.heap_->Make<Boolean>(true);
}
Object* Equal(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    return AccumulateBool(args, env, std::equal_to<int64_t>());
}
Object* Greater(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    return AccumulateBool(args, env, std::greater<int64_t>());
}
Object* Less(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    return AccumulateBool(args, env, std::less<int64_t>());
}
Object* GreaterEqual(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    return AccumulateBool(args, env, std::greater_equal<int64_t>());
}
Object* LessEqual(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    return AccumulateBool(args, env, std::less_equal<int64_t>());
}
Object* IsBool(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Boolean* n = As<Boolean>(Eval(args[0], env));
    if (!n) {
        return env.heap_->Make<Boolean>(false);
    }
    return env.heap_->Make<Boolean>(true);
}
Object* Quote(const std::vector<Object*>& args, Enviromnent&) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    return args[0];
}
bool IsTrue(Object* val) {
    if (Boolean* b = As<Boolean>(val)) {
        return b->GetValue();
    }
    return true;
}
Object* Not(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    bool flag;
    if (Boolean* b = As<Boolean>(val)) {
        flag = b->GetValue();
    } else {
        flag = true;
    }
    return env.heap_->Make<Boolean>(!flag);
}
Object* And(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(true);
    }
    Object* val;
    for (Object* expr : args) {
        val = Eval(expr, env);
        if (!IsTrue(val)) {
            return val;
        }
    }
    return val;
}
Object* Or(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.empty()) {
        return env.heap_->Make<Boolean>(false);
    }
    Object* val;
    for (Object* expr : args) {
        val = Eval(expr, env);
        if (IsTrue(val)) {
            return val;
        }
    }
    return val;
}
Object* List(const std::vector<Object*>& args, Enviromnent& env) {
    Object* ans = nullptr;
    for (std::vector<Object*>::const_reverse_iterator it = args.rbegin(); it != args.rend(); ++it) {
        Object* val = Eval(*it, env);
        ans = env.heap_->Make<Cell>(val, ans);
    }
    return ans;
}
Object* ListRef(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    Object* cnt = Eval(args[1], env);
    Number* num = As<Number>(cnt);
    if (!num) {
        throw RuntimeError("");
    }
    int64_t k = num->GetValue();
    if (k < 0) {
        throw RuntimeError("");
    }
    Object* cur = val;
    for (int64_t i = 0; i < k; ++i) {
        Cell* cell = As<Cell>(cur);
        if (!cell) {
            throw RuntimeError("");
        }
        cur = cell->GetSecond();
    }
    Cell* cell = As<Cell>(cur);
    if (!cell) {
        throw RuntimeError("");
    }
    return cell->GetFirst();
}

Object* ListTail(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    Object* cnt = Eval(args[1], env);
    Number* num = As<Number>(cnt);
    if (!num) {
        throw RuntimeError("");
    }
    int64_t k = num->GetValue();
    if (k < 0) {
        throw RuntimeError("");
    }
    Object* cur = val;
    for (int64_t i = 0; i < k; ++i) {
        Cell* cell = As<Cell>(cur);
        if (!cell) {
            throw RuntimeError("");
        }
        cur = cell->GetSecond();
    }
    return cur;
}

Object* IsNull(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    if (val == nullptr) {
        return env.heap_->Make<Boolean>(true);
    }
    return env.heap_->Make<Boolean>(false);
}
Object* IsPair(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    if (Is<Cell>(val)) {
        return env.heap_->Make<Boolean>(true);
    }
    return env.heap_->Make<Boolean>(false);
}
Object* Cons(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw RuntimeError("");
    }
    Object* first = Eval(args[0], env);
    Object* second = Eval(args[1], env);
    return env.heap_->Make<Cell>(first, second);
}
Object* Car(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    std::vector<Object*> tmp;
    tmp.push_back(args[0]);
    tmp.push_back(env.heap_->Make<Number>(0));
    return ListRef(tmp, env);
}
Object* Cdr(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    std::vector<Object*> tmp;
    tmp.push_back(args[0]);
    tmp.push_back(env.heap_->Make<Number>(1));
    return ListTail(tmp, env);
}
Object* IsList(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Object* val = Eval(args[0], env);
    if (val == nullptr) {
        return env.heap_->Make<Boolean>(true);
    }
    while (Is<Cell>(val)) {
        Cell* valnew = As<Cell>(val);
        val = valnew->GetSecond();
        if (val == nullptr) {
            return env.heap_->Make<Boolean>(true);
        }
    }
    return env.heap_->Make<Boolean>(false);
}
Object* IsSymbol(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 1) {
        throw RuntimeError("");
    }
    Symbol* n = As<Symbol>(Eval(args[0], env));
    if (!n) {
        return env.heap_->Make<Boolean>(false);
    }
    return env.heap_->Make<Boolean>(true);
}

Object* Set(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw SyntaxError("");
    }
    Symbol* symb = As<Symbol>(args[0]);
    if (!symb) {
        throw SyntaxError("");
    }

    Object* new_value = Eval(args[1], env);
    const std::string& name = symb->GetName();

    Enviromnent* cur = &env;
    while (cur) {
        auto it = cur->table.find(name);
        if (it != cur->table.end()) {
            Object* old_value = it->second;
            Number* old_num = As<Number>(old_value);
            Number* new_num = As<Number>(new_value);
            if (old_num && new_num) {
                old_num->SetValue(new_num->GetValue());
                return old_num;
            }
            it->second = new_value;
            return new_value;
        }
        cur = cur->parent_;
    }

    throw NameError("");
}

Object* SetCar(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw SyntaxError("");
    }
    Cell* symb = As<Cell>(Eval(args[0], env));
    if (!symb) {
        throw SyntaxError("");
    }
    Object* value = Eval(args[1], env);
    symb->SetFirst(value);
    return env.heap_->Make<Boolean>(true);
}
Object* SetCdr(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() != 2) {
        throw SyntaxError("");
    }
    Cell* symb = As<Cell>(Eval(args[0], env));
    if (!symb) {
        throw SyntaxError("");
    }
    Object* value = Eval(args[1], env);
    symb->SetSecond(value);
    return env.heap_->Make<Boolean>(true);
    ;
}
Object* If(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() == 2) {
        if (IsTrue(Eval(args[0], env))) {
            return Eval(args[1], env);
        }
        return nullptr;
    } else if (args.size() == 3) {
        if (IsTrue(Eval(args[0], env))) {
            return Eval(args[1], env);
        } else {
            return Eval(args[2], env);
        }
    }
    throw SyntaxError("");
}
Object* Lambda(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() < 2) {
        throw SyntaxError("");
    }
    Cell* params_list = As<Cell>(args[0]);
    if (!params_list && args[0] != nullptr) {
        throw SyntaxError("");
    }
    std::vector<std::string> params;
    Object* cur = args[0];
    while (cur) {
        Cell* cell = As<Cell>(cur);
        if (!cell) {
            throw SyntaxError("");
        }
        Symbol* s = As<Symbol>(cell->GetFirst());
        if (!s) {
            throw SyntaxError("");
        }
        params.push_back(s->GetName());
        cur = cell->GetSecond();
    }
    std::vector<Object*> body;
    for (int i = 1; i < args.size(); ++i) {
        body.push_back(args[i]);
    }
    return env.heap_->Make<LambdaFunction>(params, body, &env);
}
Object* Define(const std::vector<Object*>& args, Enviromnent& env) {
    if (args.size() < 2) {
        throw SyntaxError("");
    }
    Symbol* first = As<Symbol>(args[0]);
    if (first) {
        if (args.size() != 2) {
            throw SyntaxError("");
        }
        Object* value = Eval(args[1], env);
        Number* num = As<Number>(value);
        if (num) {
            Object* stored = value->Clone(*env.heap_);
            env.Set(first->GetName(), stored);
        } else {
            env.Set(first->GetName(), value);
        }
        return env.heap_->Make<Boolean>(true);
    }
    Cell* pair = As<Cell>(args[0]);
    if (!pair) {
        throw SyntaxError("");
    }
    Symbol* fname = As<Symbol>(pair->GetFirst());
    if (!fname) {
        throw SyntaxError("");
    }
    std::vector<Object*> lambda_args;
    lambda_args.push_back(pair->GetSecond());
    for (int i = 1; i < static_cast<int>(args.size()); ++i) {
        lambda_args.push_back(args[i]);
    }
    Object* func = Lambda(lambda_args, env);
    env.Set(fname->GetName(), func);
    if (auto* lambda = As<LambdaFunction>(func)) {
        lambda->BindSelf(fname->GetName(), func);
    }
    return env.heap_->Make<Boolean>(true);
}

void Heap::Collect(Enviromnent& global_env) {
    for (int i = 0; i < objects_.size(); ++i) {
        objects_[i]->Unmark();
    }

    MarkFromEnv(global_env);

    int j = 0;
    for (int i = 0; i < objects_.size(); ++i) {
        if (!objects_[i]->IsMarked()) {
            delete objects_[i];
        } else {
            objects_[j] = objects_[i];
            ++j;
        }
    }
    objects_.resize(j);
}

void Heap::MarkFromEnv(Enviromnent& env) {
    Enviromnent* cur = &env;
    while (cur) {
        for (auto it = cur->table.begin(); it != cur->table.end(); ++it) {
            Object* obj = it->second;
            if (obj) {
                obj->Mark();
            }
        }
        cur = cur->parent_;
    }
}

Enviromnent MakeGlobalEnv(Heap* heap) {
    Enviromnent env(heap);
    env.Set("+", env.heap_->Make<BuildFunction>(&Plus));
    env.Set("-", env.heap_->Make<BuildFunction>(&Minus));
    env.Set("*", env.heap_->Make<BuildFunction>(&Mul));
    env.Set("/", env.heap_->Make<BuildFunction>(&Del));
    env.Set("max", env.heap_->Make<BuildFunction>(&Max));
    env.Set("min", env.heap_->Make<BuildFunction>(&Min));
    env.Set("abs", env.heap_->Make<BuildFunction>(&Abs));
    env.Set("number?", env.heap_->Make<BuildFunction>(&IsNumber));
    env.Set("=", env.heap_->Make<BuildFunction>(&Equal));
    env.Set(">", env.heap_->Make<BuildFunction>(&Greater));
    env.Set("<", env.heap_->Make<BuildFunction>(&Less));
    env.Set(">=", env.heap_->Make<BuildFunction>(&GreaterEqual));
    env.Set("<=", env.heap_->Make<BuildFunction>(&LessEqual));
    env.Set("boolean?", env.heap_->Make<BuildFunction>(&IsBool));
    env.Set("quote", env.heap_->Make<BuildFunction>(&Quote));
    env.Set("not", env.heap_->Make<BuildFunction>(&Not));
    env.Set("and", env.heap_->Make<BuildFunction>(&And));
    env.Set("or", env.heap_->Make<BuildFunction>(&Or));
    env.Set("list", env.heap_->Make<BuildFunction>(&List));
    env.Set("list-ref", env.heap_->Make<BuildFunction>(&ListRef));
    env.Set("list-tail", env.heap_->Make<BuildFunction>(&ListTail));
    env.Set("null?", env.heap_->Make<BuildFunction>(&IsNull));
    env.Set("pair?", env.heap_->Make<BuildFunction>(&IsPair));
    env.Set("list?", env.heap_->Make<BuildFunction>(&IsList));

    env.Set("cons", env.heap_->Make<BuildFunction>(&Cons));
    env.Set("car", env.heap_->Make<BuildFunction>(&Car));
    env.Set("cdr", env.heap_->Make<BuildFunction>(&Cdr));
    env.Set("symbol?", env.heap_->Make<BuildFunction>(&IsSymbol));
    env.Set("define", env.heap_->Make<BuildFunction>(&Define));
    env.Set("set!", env.heap_->Make<BuildFunction>(&Set));
    env.Set("set-car!", env.heap_->Make<BuildFunction>(&SetCar));
    env.Set("set-cdr!", env.heap_->Make<BuildFunction>(&SetCdr));
    env.Set("if", env.heap_->Make<BuildFunction>(&If));
    env.Set("lambda", env.heap_->Make<BuildFunction>(&Lambda));

    return env;
};
std::string SerializeList(Object* cell) {
    std::string res = "(";
    Object* cur = cell;
    bool first_elem = true;
    while (true) {
        Cell* cons = As<Cell>(cur);
        if (!cons) {
            res += " . ";
            res += Serialize(cur);
            break;
        }
        if (!first_elem) {
            res += " ";
        }
        res += Serialize(cons->GetFirst());
        first_elem = false;
        cur = cons->GetSecond();
        if (!cur) {
            break;
        }
        if (!As<Cell>(cur)) {
            res += " . ";
            res += Serialize(cur);
            break;
        }
    }
    res += ")";
    return res;
}
std::string Serialize(Object* result) {
    if (!result) {
        return "()";
    }
    if (Number* n = As<Number>(result)) {
        return std::to_string(n->GetValue());
    }
    if (Boolean* b = As<Boolean>(result)) {
        return b->GetValue() ? "#t" : "#f";
    }
    if (Symbol* s = As<Symbol>(result)) {
        return s->GetName();
    }
    if (Cell* c = As<Cell>(result)) {
        return SerializeList(c);
    }
    throw RuntimeError("");
}
struct CurrentHeapGuard {
    CurrentHeapGuard(Heap* heap) {
        SetCurrentHeap(heap);
    }

    ~CurrentHeapGuard() {
        SetCurrentHeap(nullptr);
    }
};
std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss(str);
    Tokenizer tokenizer(&ss);
    CurrentHeapGuard heap_guard(&heap_);
    HeapGuard guard(heap_, env_);
    Object* expr = Read(&tokenizer);
    Object* result = Eval(expr, env_);
    std::string s = Serialize(result);
    return s;
}
