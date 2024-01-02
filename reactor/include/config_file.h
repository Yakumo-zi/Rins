#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H
#include <map>
#include <string>
#include <vector>

using str_map = std::map<std::string, std::map<std::string, std::string> *>;
using str_map_it = str_map::iterator;
using string = std::string;

class config_file {

  public:
    ~config_file();
    string get_string(const string &section, const string &key,
                      const string &default_value = "");
    std::vector<string> get_string_list(const string &section,
                                        const string &key);

    unsigned get_number(const string &section, const string &key,
                        const unsigned default_value = 0);
    bool get_bool(const string &section, const string &key,
                  const bool default_value = false);
    float get_float(const string &section, const string &key,
                    const float &default_value);


    static bool set_path(const char* path);
    static config_file *instance();
    static config_file* config;
  private:
    config_file(){};
    bool is_section(string line, string &section);
    unsigned parse_number(const std::string &s);
    string trim_left(const string &s);
    string trim(const string &s);
    string trim_right(const string &s);
    bool load(const char* path);

  private:
    static config_file *file;
    str_map _map;
};
#endif