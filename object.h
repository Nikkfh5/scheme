#pragma once

#include <memory>
#include <functional>
#include <error.h>
struct Enviromnent;
class Heap;
class Object {
public:
    virtual ~Object() = default;
    virtual Object* Eval(Enviromnent& env) = 0;
    virtual Object* Clone(Heap& heap) = 0;

    bool IsMarked() const {
        return marked_;
    }

    virtual void Mark() {
        if (marked_) {
            return;
        }
        marked_ = true;
        for (Object* dep : dependencies_) {
            if (dep) {
                dep->Mark();
            }
        }
    }

    void Unmark() {
        marked_ = false;
    }

protected:
    Object() : marked_(false) {
    }

    void AddDependency(Object* obj) {
        if (obj) {
            dependencies_.push_back(obj);
        }
    }

    void RemoveDependency(Object* obj) {
        for (int i = 0; i < dependencies_.size(); ++i) {
            if (dependencies_[i] == obj) {
                dependencies_.erase(dependencies_.begin() + i);
                return;
            }
        }
    }

private:
    bool marked_;
    std::vector<Object*> dependencies_;
};

class Heap {
public:
    Heap() = default;
    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
    ~Heap() {
        for (Object* obj : objects_) {
            delete obj;
        }
    }
    template <class T, class... Args>
    T* Make(Args&&... args) {
        static_assert(std::is_base_of<Object, T>::value, "нужно наследование от Object");
        T* obj = new T(std::forward<Args>(args)...);
        objects_.push_back(obj);
        return obj;
    }

    void Collect(Enviromnent& global_env);
    void MarkFromEnv(Enviromnent& env);

private:
    std::vector<Object*> objects_;
};
struct HeapGuard {
    Heap& heap;
    Enviromnent& env;

    HeapGuard(Heap& h, Enviromnent& e) : heap(h), env(e) {
    }

    ~HeapGuard() {
        heap.Collect(env);
    }
};
///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
};

template <class T>
bool Is(Object* obj) {
    auto cast = dynamic_cast<T*>(obj);
    if (cast == nullptr) {
        return false;
    }
    return true;
}
struct Enviromnent {
    std::unordered_map<std::string, Object*> table;
    Enviromnent* parent_;
    Heap* heap_;
    Enviromnent(Heap* h, Enviromnent* parent = nullptr) : parent_(parent), heap_(h) {
    }
    void Set(std::string name, Object* value) {
        table[name] = std::move(value);
    }
    Object* Get(std::string name) {
        if (table.find(name) != table.end()) {
            return table[name];
        }
        if (parent_) {
            return parent_->Get(name);
        }
        throw NameError("");
    }
    void Assign(const std::string& name, Object* value) {
        Enviromnent* cur = this;
        while (cur) {
            auto it = cur->table.find(name);
            if (it != cur->table.end()) {
                it->second = std::move(value);
                return;
            }
            cur = cur->parent_;
        }
        throw NameError("");
    }
};

struct Callable : Object {
    virtual Object* Apply(const std::vector<Object*>& args, Enviromnent& env) = 0;
    Object* Eval(Enviromnent& env) override {
        return this;
    }
};
class BuildFunction : public Callable {
public:
    using FuncPtr = Object* (*)(const std::vector<Object*>&, Enviromnent&);
    explicit BuildFunction(FuncPtr func) : func_(func) {
    }

    Object* Apply(const std::vector<Object*>& args, Enviromnent& env) override {
        return func_(args, env);
    }

    Object* Eval(Enviromnent& env) override {
        return this;
    }
    Object* Clone(Heap& heap) override {
        return heap.Make<BuildFunction>(func_);
    }

private:
    FuncPtr func_;
};
class LambdaFunction : public Callable {
public:
    LambdaFunction(const std::vector<std::string>& params, const std::vector<Object*>& body,
                   Enviromnent* env)
        : params_(params), body_(body) {
        if (env->parent_ == nullptr) {
            env_ = env;
        } else {
            owned_env_ = std::make_unique<Enviromnent>(*env);
            owned_env_->heap_ = env->heap_;
            env_ = owned_env_.get();
        }
        for (int i = 0; i < static_cast<int>(body_.size()); ++i) {
            AddDependency(body_[i]);
        }
    }

    void BindSelf(const std::string& name, Object* self) {
        env_->Set(name, self);
    }

    Object* Apply(const std::vector<Object*>& args, Enviromnent& call_env) override {
        if (args.size() != params_.size()) {
            throw RuntimeError("");
        }

        Enviromnent local_env(env_->heap_, env_);

        for (int i = 0; i < static_cast<int>(params_.size()); ++i) {
            Object* v = args[i]->Eval(call_env);
            local_env.Set(params_[i], v);
        }

        Object* result = nullptr;
        for (int i = 0; i < static_cast<int>(body_.size()); ++i) {
            result = body_[i]->Eval(local_env);
        }
        return result;
    }

    Object* Eval(Enviromnent&) override {
        return this;
    }

    Object* Clone(Heap& heap) override {
        std::vector<Object*> new_body;
        for (int i = 0; i < static_cast<int>(body_.size()); ++i) {
            new_body.push_back(body_[i]->Clone(heap));
        }
        return heap.Make<LambdaFunction>(params_, new_body, env_);
    }

    void Mark() override {
        if (IsMarked()) {
            return;
        }
        Object::Mark();

        Enviromnent* cur = env_;
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

private:
    Enviromnent* env_;
    std::unique_ptr<Enviromnent> owned_env_;
    std::vector<std::string> params_;
    std::vector<Object*> body_;
};

class Number : public Object {
    int64_t value_;

public:
    explicit Number(int64_t value) : value_(value) {
    }

    int64_t GetValue() const {
        return value_;
    }

    void SetValue(int64_t value) {
        value_ = value;
    }

    Object* Eval(Enviromnent&) override {
        return this;
    }

    Object* Clone(Heap& heap) override {
        return heap.Make<Number>(value_);
    }

    void Mark() override {
        if (IsMarked()) {
            return;
        }
        Object::Mark();
    }
};

class Boolean : public Object {
    bool value_;

public:
    explicit Boolean(bool value) : value_(value) {
    }

    bool GetValue() const {
        return value_;
    }

    Object* Eval(Enviromnent&) override {
        return this;
    }

    Object* Clone(Heap& heap) override {
        return heap.Make<Boolean>(value_);
    }

    void Mark() override {
        if (IsMarked()) {
            return;
        }
        Object::Mark();
    }
};

class Symbol : public Object {
    std::string name_;

public:
    explicit Symbol(const std::string& name) : name_(name) {
    }

    const std::string& GetName() const {
        return name_;
    }

    Object* Eval(Enviromnent& env) override {
        return env.Get(name_);
    }

    Object* Clone(Heap& heap) override {
        return heap.Make<Symbol>(name_);
    }

    void Mark() override {
        if (IsMarked()) {
            return;
        }
        Object::Mark();
    }
};

class Cell : public Object {
    Object* first_;
    Object* second_;

public:
    Cell(Object* first, Object* second) : first_(first), second_(second) {
        AddDependency(first_);
        AddDependency(second_);
    }

    Object* GetFirst() const {
        return first_;
    }

    Object* GetSecond() const {
        return second_;
    }

    void SetFirst(Object* first) {
        RemoveDependency(first_);
        first_ = first;
        AddDependency(first_);
    }

    void SetSecond(Object* second) {
        RemoveDependency(second_);
        second_ = second;
        AddDependency(second_);
    }

    Object* Eval(Enviromnent& env) override {
        if (!first_) {
            throw RuntimeError("");
        }
        Object* func_obj = first_->Eval(env);
        if (Callable* func = As<Callable>(func_obj)) {
            std::vector<Object*> args;
            Object* second = second_;
            while (second) {
                Cell* pair = As<Cell>(second);
                if (!pair) {
                    throw RuntimeError("");
                }
                args.push_back(pair->GetFirst());
                second = pair->GetSecond();
            }
            return func->Apply(args, env);
        }
        throw RuntimeError("");
    }

    Object* Clone(Heap& heap) override {
        Object* f = first_ ? first_->Clone(heap) : nullptr;
        Object* s = second_ ? second_->Clone(heap) : nullptr;
        return heap.Make<Cell>(f, s);
    }

    void Mark() override {
        if (IsMarked()) {
            return;
        }
        Object::Mark();
    }
};
