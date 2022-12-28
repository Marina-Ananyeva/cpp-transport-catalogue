#include "json.h"

using namespace std::literals;

namespace json {
//-----------------Node--------------------
Node::Node(std::nullptr_t null)
    : value_(null) {
}

Node::Node(Array array)
    : value_(array) {
}

Node::Node(Dict map)
    : value_(map) {
}

Node::Node(bool b)
    : value_(b) {
}

Node::Node(int value)
    : value_(value) {
}

Node::Node(double d)
    : value_(d) {
}

Node::Node(std::string value)
    : value_(value) {
}

const Node::Value& Node::GetValue() const { 
    return value_; 
}

bool Node::IsInt() const {
    return IsValue<int>();
}

bool Node::IsDouble() const {//Возвращает true, если в Node хранится int либо double.
    return IsValue<int>() || IsValue<double>();
}

bool Node::IsPureDouble() const{// Возвращает true, если в Node хранится double.
    return IsValue<double>(); 
}

bool Node::IsBool() const {
    return IsValue<bool>(); 
}

bool Node::IsString() const {
    return IsValue<std::string>(); 
}

bool Node::IsNull() const {
    return IsValue<std::nullptr_t>(); 
}

bool Node::IsArray() const {
    return IsValue<Array>(); 
}

bool Node::IsMap() const {
    return IsValue<Dict>(); 
}

//Ниже перечислены методы, которые возвращают хранящееся внутри Node значение заданного типа. 
//Если внутри содержится значение другого типа, должно выбрасываться исключение std::logic_error.
int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<bool>(value_);
}

double Node::AsDouble() const {// Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    if (!IsDouble()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<double>(value_);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<std::string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Logic error"s);
    }
    return std::get<Dict>(value_);
}

// Объекты Node можно сравнивать между собой при помощи == и !=.
// Значения равны, если внутри них значения имеют одинаковый тип и содержимое.
bool operator==(const Node& lhs, const Node &rhs) {
    if (lhs.GetValue().index() != rhs.GetValue().index()) {
        return false;
    } 
    if (lhs.IsInt() == true && lhs.IsInt() == rhs.IsInt()) { 
        return lhs.AsInt() == rhs.AsInt(); 
    } 
    if (lhs.IsPureDouble() == true && lhs.IsPureDouble() == rhs.IsPureDouble()) { 
        return lhs.AsDouble() == rhs.AsDouble(); 
    }
    if (lhs.IsBool() == true && lhs.IsBool() == rhs.IsBool()) { 
        return lhs.AsBool() == rhs.AsBool(); 
    }
    if (lhs.IsString() == true && lhs.IsString() == rhs.IsString()) { 
        return lhs.AsString() == rhs.AsString(); 
    }
    if (lhs.IsNull() == true && lhs.IsNull() == rhs.IsNull()) { 
        return true; 
    }
    if (lhs.IsArray() == true && lhs.IsArray() == rhs.IsArray()) { 
        return lhs.AsArray() == rhs.AsArray(); 
    }
    if (lhs.IsMap() == true && lhs.IsMap() == rhs.IsMap()) { 
        return lhs.AsMap() == rhs.AsMap(); 
    }
    return false;
}

bool operator!=(const Node &lhs, const Node &rhs) {
    return !(lhs == rhs);
}

namespace detail {
//-----------------Load-----------------------
Node LoadNode(std::istream& input);

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node{std::stoi(parsed_num)};
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node{std::stod(parsed_num)};
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node{s};
}

Node LoadArray(std::istream& input) {
    Array result;
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Failed to read array from stream"s);
    }

    return Node(result);
}

Node LoadDict(std::istream& input) {
    Dict result;
    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({key, LoadNode(input)});
    }

    if (!input) {
        throw ParsingError("Failed to read map from stream"s);
    }

    return Node(result);
}

Node LoadNull(std::istream& input) {
    using namespace std::literals;
    std::string parsed_null{"n"s};
    for (char c; input >> c && c != '}' && c != ']';) {
        if (c != ' ' && c != '\\' && c != '\t' && c !='\r' && c != '\n') {
            parsed_null += c;
            if (parsed_null == "null"s) {
                return Node{std::nullptr_t()};
            }
        }
        if (!input) {
            throw ParsingError("Failed to read null from stream"s);
        }
    }
    if (parsed_null != "null"s) {
        throw ParsingError("Null parsing error"s);
    } 
    return Node{};
}

Node LoadBoolTrue(std::istream& input) {
    using namespace std::literals;
    std::string parsed_bool{"t"s};
    for (char c; input >> c && c != '}' && c != ']';) {
        if (c != ' ' && c != '\\' && c != '\t' && c !='\r' && c != '\n') {
            parsed_bool += c;
            if (parsed_bool == "true"s) {
                return Node{bool(true)};
            }
            if (!input) {
                throw ParsingError("Failed to read bool_true from stream"s);
            }
        }
    }

    if (parsed_bool != "true"s) {
        throw ParsingError("Bool_true parsing error"s);
    }
    return Node{};
}

Node LoadBoolFalse(std::istream& input) {
    using namespace std::literals;
    std::string parsed_bool{"f"s};
    for (char c; input >> c && c != '}' && c != ']';) {
        if (c != ' ' && c != '\\' && c != '\t' && c !='\r' && c != '\n') {
            parsed_bool += c;
            if (parsed_bool == "false"s) {
                return Node{bool(false)};
            }
            if (!input) {
                throw ParsingError("Failed to read bool_false from stream"s);
            }
        }
    }

    if (parsed_bool != "false"s) {
        throw ParsingError("Bool_false parsing error"s);
    }
    return Node{};
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;
    if (c) {
        if (c != ' ' && c != '\\' && c != '\t' && c !='\r' && c != '\n') {
            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                return LoadNull(input);
            } else if (c == 't') {
                return LoadBoolTrue(input);
            } else if (c == 'f') {
                return LoadBoolFalse(input);
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }
    }
    if (!input) {
        throw ParsingError("Failed to read Node from stream"s);
    }
    return Node{};
}

//------------------Print---------------------------------------
void PrintNode(const json::Node &node, std::ostream &out);
void PrintNode(const json::Node &node, const PrintContext &ctx);

// Шаблон, подходящий для вывода double и int
template <typename T>
void PrintValue(const T& value, std::ostream& out) {
    out << value;
}

template <typename T>
void PrintValue(const T& value, const PrintContext& ctx) {
    PrintValue(value, ctx.out);
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(std::nullptr_t null, const PrintContext& ctx) {
    PrintValue(null, ctx.out);
}

// Перегрузка функции PrintValue для вывода значений string
void PrintValue(const std::string& s, std::ostream& out) {
    using namespace std::literals;
    auto it = s.begin();
    auto end = s.end();

    out << "\""s;//так как string
    while (it != end) {
        const char ch = *it;

        if (ch == '\\') {
            ++it;
            out << "\\"s << "\\"s;
            continue;
        }
        if (ch == '\"') {
            ++it;
            out << "\\"s << "\""s;
            continue;
        }
        if (ch == '\n') {
            ++it;
            out << "\\"s << "n"s;
            continue;
        }
        if (ch == '\r') {
            ++it;
            out << "\\"s << "r"s;
            continue;
        }
        out << *it;
        ++it;
    }
    out << "\""s;//закрывающие кавычки, т.к. string
}

void PrintValue(const std::string& s, const PrintContext& ctx) {
    PrintValue(s, ctx.out);
}

// Перегрузка функции PrintValue для вывода значений bool
void PrintValue(const bool& b, std::ostream& out) {
    if (b == false) {
        out << "false"s;
    } else {
    out << "true"s;
    }
}

void PrintValue(const bool& b, const PrintContext& ctx) {
    PrintValue(b, ctx.out);
}

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(const json::Array& array, std::ostream& out) {
    out << "["s << std::endl;
    for (size_t i = 0; i < array.size(); ++i) {
        PrintNode(array[i], out);
        if (i + 1 != array.size()) {
            out << ","s << std::endl;
        }
    }
    out << std::endl << "]"s;
}

void PrintValue(const json::Array& array, const PrintContext& ctx) {
    ctx.out << "["s << std::endl;
    for (size_t i = 0; i < array.size(); ++i) {
        ctx.Indented().PrintIndent();
        PrintNode(array[i], ctx.Indented());
        if (i + 1 != array.size()) {
            ctx.out << ","s << std::endl;
        }
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "]"s;
}

// Перегрузка функции PrintValue для вывода значений Dict
void PrintValue(const json::Dict& map, std::ostream& out) {
    out << "{"s << std::endl;
    size_t i = 0;
    for (const auto& [key, val] : map) {
        ++i;
        out << "\"" << key << "\": "s;
        PrintNode(val, out);
        if (i + 1 <= map.size()) {
            out << ","s << std::endl;;
        }
    }
    out << std::endl << "}"s;
}

void PrintValue(const json::Dict& map, const PrintContext& ctx) {
    ctx.out << "{"s << std::endl;
    size_t i = 0;
    for (const auto& [key, val] : map) {
        ++i;
        ctx.Indented().PrintIndent();
        ctx.out << "\"" << key << "\": "s;
        PrintNode(val, ctx.Indented());
        if (i + 1 <= map.size()) {
            ctx.out << ","s << std::endl;
        }
    }
    ctx.out << std::endl;
    ctx.PrintIndent();
    ctx.out << "}"s;
}

void PrintNode(const json::Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
}

void PrintNode(const json::Node& node, const PrintContext& ctx) {
    std::visit([&ctx](const auto& value){ 
            PrintValue(value, ctx); },
            node.GetValue());
}
}// namespace detail

//-------------------Document-------------------

Document::Document(Node root)
    : root_(root) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document &rhs) const {
    return this->GetRoot() == rhs.GetRoot();
}

bool Document::operator!=(const Document &rhs) const {
    return this->GetRoot() != rhs.GetRoot();
}

Document Load(std::istream& input) {
    return Document{detail::LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    detail::PrintContext ctx(output);                           //задаем параметры отступов
    detail::PrintNode(doc.GetRoot(), ctx);                      //вывод с настроенными отступами
    //detail::PrintNode(doc.GetRoot(), output);                 //вариативность для вывода без настроек отступов
    //output << std::endl;
}
}// namespace json