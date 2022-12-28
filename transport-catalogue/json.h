#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(std::nullptr_t null);
    Node(Array array);
    Node(Dict map);
    Node(bool b);
    Node(int value);
    Node(double d);
    Node(std::string value);

    const Value &GetValue() const;

    template <class T>
    bool IsValue() const;

    bool IsInt() const;
    bool IsDouble() const; //Возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const; //Возвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    //Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа. 
    //Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;// Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
private:
    Value value_;
};

// Объекты Node можно сравнивать между собой при помощи == и !=.
// Значения равны, если внутри них значения имеют одинаковый тип и содержимое.
bool operator==(const Node &lhs, const Node &rhs);
bool operator!=(const Node &lhs, const Node &rhs);

template <class T>
bool Node::IsValue() const {
    return std::holds_alternative<T>(value_);
}

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document &rhs) const;
    bool operator!=(const Document &rhs) const;
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

//------------------------------------------------------------------
namespace detail {
// Контекст вывода, хранит ссылку на поток вывода и текущий отступ
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
    PrintContext(std::ostream& out)
        : out(out) {
    }

    PrintContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }
    
    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};
}// namespace detail
}// namespace json