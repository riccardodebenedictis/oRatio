#include "json.h"

namespace smt
{
    json::json() : ptr(new json_core()) { ptr->ref_count++; }
    json::json(json_core *const ptr) : ptr(ptr) { ptr->ref_count++; }
    json::json(const json &orig) : ptr(orig.ptr) { ptr->ref_count++; }
    json::~json()
    {
        ptr->ref_count--;
        if (ptr->ref_count == 0)
            delete ptr;
    }

    json &json::operator=(const json &t)
    {
        t.ptr->ref_count++;
        return *this;
    }
    void json::to_json(std::ostream &os) const { ptr->to_json(os); }

    json json::from_json(std::istream &is)
    {
        is >> std::ws; // we remove all the leading whitespace..
        switch (is.peek())
        {
        case '{':
        { // we have a json object..
            is.get();
            std::map<std::string, json> vals;
            do
            {
                is >> std::ws;
                if (is.get() != '\"')
                    throw std::invalid_argument("not a valid json");
                std::string id;
                while (is.peek() != '\"')
                    id += is.get();
                is.get();
                is >> std::ws;
                if (is.get() != ':')
                    throw std::invalid_argument("not a valid json");
                vals.emplace(id, from_json(is));
                is >> std::ws;
            } while (is.peek() == ',' && is.get());
            if (is.get() != '}')
                throw std::invalid_argument("not a valid json");
            return json(new json_core(vals));
        }
        case '[':
        { // we have a json array..
            is.get();
            std::vector<json> vals;
            do
            {
                vals.push_back(from_json(is));
                is >> std::ws;
            } while (is.peek() == ',' && is.get());
            if (is.get() != ']')
                throw std::invalid_argument("not a valid json");
            return json(new array_val(vals));
        }
        case '-': // we have a json number..
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            std::string num;
            num += is.get();
            while (is.peek() == '0' || is.peek() == '1' || is.peek() == '2' || is.peek() == '3' || is.peek() == '4' || is.peek() == '5' || is.peek() == '6' || is.peek() == '7' || is.peek() == '8' || is.peek() == '9')
                num += is.get();
            if (is.peek() == '.')
            {
                num += is.get();
                while (is.peek() == '0' || is.peek() == '1' || is.peek() == '2' || is.peek() == '3' || is.peek() == '4' || is.peek() == '5' || is.peek() == '6' || is.peek() == '7' || is.peek() == '8' || is.peek() == '9')
                    num += is.get();
                if (is.peek() == 'e' || is.peek() == 'E')
                {
                    num += is.get();
                    if (is.peek() == '+')
                        num += is.get();
                    if (is.peek() == '-')
                        num += is.get();
                    while (is.peek() == '0' || is.peek() == '1' || is.peek() == '2' || is.peek() == '3' || is.peek() == '4' || is.peek() == '5' || is.peek() == '6' || is.peek() == '7' || is.peek() == '8' || is.peek() == '9')
                        num += is.get();
                    return json(new double_val(std::stod(num)));
                }
                return json(new double_val(std::stod(num)));
            }
            else if (is.peek() == 'e' || is.peek() == 'E')
            {
                num += is.get();
                if (is.peek() == '+')
                    num += is.get();
                if (is.peek() == '-')
                    num += is.get();
                while (is.peek() == '0' || is.peek() == '1' || is.peek() == '2' || is.peek() == '3' || is.peek() == '4' || is.peek() == '5' || is.peek() == '6' || is.peek() == '7' || is.peek() == '8' || is.peek() == '9')
                    num += is.get();
                return json(new double_val(std::stod(num)));
            }
            else
                return json(new long_val(std::stol(num)));
        }
        case '.':
        {
            is.get();
            long dec_part;
            is >> dec_part;
            return json(new double_val(std::stod('.' + std::to_string(dec_part))));
        }
        case 'f':
        { // we have a false literal..
            is.get();
            if (is.get() == 'a' && is.get() == 'l' && is.get() == 's' && is.get() == 'e')
                return json(new bool_val(false));
            throw std::invalid_argument("not a valid json");
        }
        case 't':
        { // we have a true literal..
            is.get();
            if (is.get() == 'r' && is.get() == 'u' && is.get() == 'e')
                return json(new bool_val(true));
            throw std::invalid_argument("not a valid json");
        }
        case 'n':
        { // we have a null literal..
            is.get();
            if (is.get() == 'u' && is.get() == 'l' && is.get() == 'l')
                return json(new null_val());
            throw std::invalid_argument("not a valid json");
        }
        case '\"':
        { // we have a json string..
            is.get();
            std::string id;
            while (is.peek() != '\"')
                id += is.get();
            is.get();
            return json(new string_val(id));
        }
        default:
            throw std::invalid_argument("not a valid json");
        }
    }

    json_core::json_core() {}
    json_core::json_core(const std::map<std::string, json> &vals) : vals(vals) {}
    json_core::~json_core() {}

    void json_core::set_null(const std::string &id) { vals.emplace(id, new null_val()); }

    void json_core::to_json(std::ostream &os) const
    {
        os << '{';
        for (std::map<std::string, json>::const_iterator v_it = vals.begin(); v_it != vals.end(); ++v_it)
        {
            if (v_it != vals.begin())
                os << ", ";
            os << '\"' << v_it->first << "\": ";
            v_it->second->to_json(os);
        }
        os << '}';
    }

    null_val::null_val() {}
    null_val::~null_val() {}

    void null_val::to_json(std::ostream &os) const { os << "null"; }

    bool_val::bool_val() {}
    bool_val::bool_val(const bool &val) : val(val) {}
    bool_val::~bool_val() {}

    void bool_val::to_json(std::ostream &os) const { os << std::boolalpha << val; }

    string_val::string_val() {}
    string_val::string_val(const std::string &val) : val(val) {}
    string_val::~string_val() {}

    void string_val::to_json(std::ostream &os) const { os << '\"' << val << '\"'; }

    long_val::long_val() {}
    long_val::long_val(const long &val) : val(val) {}
    long_val::~long_val() {}

    void long_val::to_json(std::ostream &os) const { os << std::to_string(val); }

    double_val::double_val() {}
    double_val::double_val(const double &val) : val(val) {}
    double_val::~double_val() {}

    void double_val::to_json(std::ostream &os) const { os << std::to_string(val); }

    array_val::array_val() {}
    array_val::array_val(const std::vector<json> &vals) : vals(vals) {}
    array_val::~array_val() {}

    void array_val::to_json(std::ostream &os) const
    {
        os << '[';
        for (std::vector<json>::const_iterator v_it = vals.begin(); v_it != vals.end(); ++v_it)
        {
            if (v_it != vals.begin())
                os << ", ";
            v_it->to_json(os);
        }
        os << ']';
    }

    std::ostream &operator<<(std::ostream &os, const json &j)
    {
        j.to_json(os);
        return os;
    }
}; // namespace smt