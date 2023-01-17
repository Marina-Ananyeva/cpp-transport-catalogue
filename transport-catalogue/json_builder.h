#pragma once
#include "json.h"

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <typeinfo>

namespace json {
class KeyItemContext;
class DictItemContext;
class StartArrayItemContext;
class ValueAfterKeyItemContext;
class ValueAfterStartArrayItemContext;

class Builder {

class LogicError : public std::logic_error {
public:
    using logic_error::logic_error;
};

public:
    template <class T>
    bool IsValue(Node::Value value) const;

    bool IsLastObjectIsArray() const;
    bool IsLastObjectIsDict() const;
    void CheckIfObjectNotBuilt() const;
    void CheckIfObjectNotFinished() const;
    void CheckIfObjectFinished() const;

    Node GetValue(Node::Value value) const;
    Node GetNode() const;

    Builder& Value(Node::Value);
    KeyItemContext Key(std::string);
    DictItemContext StartDict();
    StartArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;                      //сам конструируемый объект
    Node key_;                      //ключ текущего объекта Dict
    std::vector<Node*> nodes_stack_ = {}; //стек указателей на те вершины JSON, которые ещё не построены: 
};

template <class T>
bool Builder::IsValue(Node::Value value) const {
    return std::holds_alternative<T>(value);
}

class KeyItemContext : public Builder {
public:
    KeyItemContext(Builder& builder) : builder_(builder) {
    }
    ValueAfterKeyItemContext Value(Node::Value value);
    KeyItemContext Key(std::string) = delete;
    Builder& EndDict() = delete;
    DictItemContext StartDict();
    StartArrayItemContext StartArray();
    StartArrayItemContext EndArray() = delete;
    Node Build() = delete;
private:
    Builder &builder_;
};

class ValueAfterKeyItemContext : public Builder {
public:
    ValueAfterKeyItemContext(Builder& builder) : builder_(builder) {
    }
    Builder &Value(Node::Value) = delete;
    KeyItemContext Key(std::string);
    DictItemContext StartDict() = delete;
    StartArrayItemContext StartArray() = delete;
    Builder &EndDict();
    Builder& EndArray() = delete;
    Node Build() = delete;
private:
    Builder &builder_;
};

class DictItemContext : public Builder {
public:
    DictItemContext(Builder& builder) : builder_(builder) {
    }
    Builder& Value(Node::Value) = delete;
    KeyItemContext Key(std::string str);
    DictItemContext StartDict() = delete;
    StartArrayItemContext StartArray() = delete;
    Builder &EndDict();
    Builder& EndArray() = delete;
    Node Build() = delete;
private:
    Builder &builder_;
};

class StartArrayItemContext : public Builder {
public:
    StartArrayItemContext(Builder& builder) : builder_(builder) {
    }
    ValueAfterStartArrayItemContext Value(Node::Value value);
    KeyItemContext Key(std::string) = delete;
    DictItemContext StartDict();
    StartArrayItemContext StartArray();
    Builder& EndDict() = delete;
    Builder &EndArray();
    Node Build() = delete;
private:
    Builder &builder_;
};

class ValueAfterStartArrayItemContext : public Builder {
public:
    ValueAfterStartArrayItemContext(Builder& builder) : builder_(builder) {
    }
    ValueAfterStartArrayItemContext Value(Node::Value);
    KeyItemContext Key(std::string) = delete;
    DictItemContext StartDict();
    StartArrayItemContext StartArray();
    Builder& EndDict() = delete;
    Builder &EndArray();
    Node Build() = delete;
private:
    Builder &builder_;
};

} //namespace json
/*
class Builder;
    class BaseContext;
    class DictItemContext;
    class KeyItemContext;
    class ArrayItemContext;
    class ValueAfterKeyContext;
 
    class BaseContext 
    {
    public:
        BaseContext(Builder& builder);
        DictItemContext StartDict();
        KeyItemContext Key(std::string key);
        ArrayItemContext StartArray();      
        Builder& EndDict();
        Builder& EndArray();
        Builder& Value(Node value);
 
    protected:
        Builder& builder_;
    };
 
    class DictItemContext : public BaseContext
    {
    public:
        DictItemContext(Builder& builder);
        DictItemContext StartDict() = delete;
        Builder& StartArray() = delete;
        Builder& EndArray() = delete;
        //Builder& Value(Node value) = delete;
 
        ValueAfterKeyContext Value(Node value) = delete;
    };
 
    class ValueAfterKeyContext : public KeyItemContext
    {
    public:
        ValueAfterKeyContext(Builder& builder);
        Builder& Value(Node value) = delete;
    };
 
    class KeyItemContext : public BaseContext
    {
    public:
        KeyItemContext(Builder& builder);
        KeyItemContext Key(std::string key) = delete;
        Builder& EndArray() = delete;
        Builder& EndDict() = delete;
    };
    
    class ArrayItemContext : public BaseContext
    {
    public:
        ArrayItemContext(Builder& builder);
        Builder& EndDict() = delete;
        KeyItemContext Key(std::string key) = delete;
    };
 
    class Builder
    {
    public:
        Builder();
 
        KeyItemContext Key(std::string key);
 
        Builder& Value(Node value);
 
        DictItemContext StartDict();
 
        Builder& EndDict();
 
        ArrayItemContext StartArray();
 
        Builder& EndArray();
 
        Node Build();
 
    private:
        Node root_ = nullptr; // сам конструируемый объект
        std::vector<Node*> nodes_stack_; // стек указателей на те вершины JSON, которые ещё не построены:
                                         // то есть текущее описываемое значение и цепочка его родителей. 
    };
*/