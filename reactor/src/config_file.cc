#include "config_file.h"
#include <assert.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <strings.h>
#include <vector>

config_file *config_file::config = nullptr;
config_file::~config_file() {
    for (str_map_it it = _map.begin(); it != _map.end(); it++) {
        delete it->second;
    }
}

string config_file::get_string(const string &section, const string &key,
                               const string &default_value) {
    str_map_it it = _map.find(section);
    if (it != _map.end()) {
        std::map<string, string>::const_iterator c_it = it->second->find(key);
        if (c_it != it->second->end()) {
            return c_it->second;
        }
    }
    return default_value;
}
std::vector<string> config_file::get_string_list(const string &section,
                                                 const string &key) {
    std::vector<string> v;
    string str = get_string(section, key, "");
    string sep = ",\t";
    string substr;
    string::size_type start = 0;
    string::size_type index;
    while ((index = str.find_first_of(sep, start)) != string::npos) {
        substr = str.substr(start, index - start);
        v.push_back(substr);
        start = str.find_first_not_of(sep, index);
        if (start == string::npos) {
            return v;
        }
    }
    substr = str.substr(start);
    v.push_back(substr);
    return v;
}
bool config_file::get_bool(const string &section, const string &key,
                           bool default_value) {
    str_map_it it = _map.find(section);
    if (it != _map.end()) {
        std::map<string, string>::const_iterator c_it = it->second->find(key);
        if (c_it != it->second->end()) {
            if (strcasecmp(c_it->second.c_str(), "true") == 0) {
                return true;
            }
        }
    }
    return default_value;
}
float config_file::get_float(const string &section, const string &key,
                             const float &default_value) {
    std::ostringstream convert1;
    convert1 << default_value;
    string default_value_string = convert1.str();
    string text = get_string(section, key, default_value_string);

    std::istringstream convert2(text);
    float result;
    if (!(convert2 >> result)) {
        return result;
    }
    return result;
}
unsigned config_file::get_number(const string &section, const string &key,
                                 const unsigned default_value) {
    str_map_it it = _map.find(section);
    if (it != _map.end()) {
        std::map<string, string>::const_iterator c_it = it->second->find(key);
        if (c_it != it->second->end()) {
            return parse_number(c_it->second);
        }
    }
    return default_value;
}
unsigned config_file::parse_number(const string &s) {
    std::istringstream iss(s);
    long long v = 0;
    iss >> v;
    return v;
}
string config_file::trim_left(const string &s) {
    size_t first_not_empty = 0;
    string::const_iterator beg = s.begin();
    while (beg != s.end()) {
        if (!::isspace(*beg)) {
            break;
        }
        first_not_empty++;
        beg++;
    }
    return s.substr(first_not_empty);
}
string config_file::trim_right(const string &s) {
    size_t last_not_empty = s.length();
    string::const_iterator end = s.end();
    while (end != s.begin()) {
        end--;
        if (!::isspace(*end)) {
            break;
        }
        last_not_empty--;
    }
    return s.substr(0, last_not_empty);
}
string config_file::trim(const string &s) { return trim_left(trim_right(s)); }

bool config_file::is_section(string line, string &section) {
    section = trim(line);
    if (section.empty() || section.length() <= 2) {
        return false;
    }
    if (section[0] != '[' || section[section.length() - 1] != ']') {
        return false;
    }
    section = section.substr(1, section.length() - 2);
    section = trim(section);
    return true;
}
bool config_file::load(const char *path) {
    std::ifstream ifs(path);
    // 判断文件是否打开
    if (!ifs.good()) {
        return false;
    }

    string line;
    std::map<string, string> *m = nullptr;
    while (!ifs.eof() && ifs.good()) {
        getline(ifs, line);
        string section;
        if (is_section(line, section)) {
            str_map_it it = _map.find(section);
            if (it == _map.end()) {
                m = new std::map<string, string>();
                _map.insert({section, m});
            } else {
                m = it->second;
            }
            continue;
        }
        size_t equ_pos = line.find('=');
        if (equ_pos == string::npos) {
            continue;
        }
        string key = line.substr(0, equ_pos);
        string value = line.substr(equ_pos + 1);
        key = trim(key);
        value = trim(value);

        if (key.empty()) {
            continue;
        }
        if (key[0] == '#' || key[0] == ';') {
            continue;
        }

        std::map<string, string>::iterator it = m->find(key);
        if (it != m->end()) {
            it->second = value;
        } else {
            m->insert({key, value});
        }
    }
    ifs.close();
    return true;
}
bool config_file::set_path(const char *path) {
    assert(config == nullptr);
    config = new config_file();
    return config->load(path);
}

config_file *config_file::instance() {
    assert(config != nullptr);
    return config;
}