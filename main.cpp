#include <iostream>
#include <functional>
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <istream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <cstdio>
#include <stdio.h>
#include <ctime>
#include <chrono>
#include <unordered_set>

class Base {
    public: 
        Base(std::string name) : _name(name) {
            _time = current_time();
            _date = current_date();
        }

        std::string current_date(){
            std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            char buf[100] = {0};
            std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&now));
            return buf;
        }
        std::string current_time(){
            time_t now = time(NULL);
            struct tm tstruct;
            char buf[40];
            tstruct = *localtime(&now);
            //format: HH:mm:ss
            strftime(buf, sizeof(buf), "%X", &tstruct);
            return buf;
        }

        std::string _time;
        std::string _date;
        std::string _name;

        bool operator==(const Base& p) const { 
            return _name == p._name && _time == p._time && _date == p._date; 
        } 
};

class File : public Base {
    public:
        File(std::string name) : Base(name) { }
};

struct dir_hash {
    std::size_t operator()(const Dir& dir) const {
        return std::hash<std::string>()(dir._name);
    }
};

struct file_hash {
    std::size_t operator()(const File& file) const {
        return std::hash<std::string>()(file._name);
    }
};

class Dir : public Base {
    public: 
        Dir(std::string name) : Base(name) { }
        std::unordered_set<Dir, dir_hash> _dirs;
        std::unordered_set<File, file_hash> _files;
};

inline void print_date_time(const Base* current_dir_ptr) {
    std::cout << current_dir_ptr->_date << ", " << current_dir_ptr->_time << " ";
}

std::ostream& operator<<(std::ostream& os, const Dir& dir)
{
    print_date_time(&dir);
    os << dir._name;
    return os;
}

std::ostream& operator<<(std::ostream& os, const File& file)
{
    print_date_time(&file);
    os << file._name;
    return os;
}

void list_files_and_folders(Dir* current_dir_ptr) {
    print_date_time(current_dir_ptr);
    std::cout << current_dir_ptr->_name << std::endl;
    for(const Dir& el:current_dir_ptr->_dirs) {
        std::cout << el << std::endl;
    } 
    for(const File& el:current_dir_ptr->_files) {
        std::cout << el << std::endl;
    } 
}

void call_cd(Dir* current_dir_ptr, std::string param) {
    if(param != "." || (current_dir_ptr->_name == "." && param == "..")) {
        *current_dir_ptr = Dir(param);
    }
}

void call_dir(Dir* current_dir_ptr, std::string param) {
    list_files_and_folders(current_dir_ptr);
}

void call_mkfile(Dir* current_dir_ptr, std::string param) {
    File file(param);
    current_dir_ptr->_files.insert(file);
}

void call_mkdir(Dir* current_dir_ptr, std::string param) {
    Dir dir(param);
    current_dir_ptr->_dirs.insert(dir);
}

void call_rmdir(Dir* current_dir_ptr, std::string param) {
    for(const Dir& el : current_dir_ptr->_dirs) {
        if(el._name == param) {
            current_dir_ptr->_dirs.erase(el);
            return;
        }
    }
}

void call_rmfile(Dir* current_dir_ptr, std::string param) {
    for(const File& el : current_dir_ptr->_files) {
        if(el._name == param) {
            current_dir_ptr->_files.erase(el);
            return;
        }
    }
}

class File_System {
    public:
        File_System() {
            // initialize root
            root = new Dir(".");
            current_dir_ptr = root;
            function_map.emplace("cd", &call_cd);
            function_map.emplace("mkdir", &call_mkdir);
            function_map.emplace("dir", &call_dir);
            function_map.emplace("mkfile", &call_mkfile);
            function_map.emplace("rmfile", &call_rmfile);
            function_map.emplace("rmdir", &call_rmdir);
        }

        ~File_System() {
            delete root;
        }
        
        void execute_command (std::string command_name, std::string param) {
            function_map[command_name](current_dir_ptr, param);
        }

    private:
        std::map<std::string, std::function<void(Dir* current_dir_ptr, std::string param)>> function_map;
        Dir* root;
        Dir* current_dir_ptr;
};

std::vector<std::string> split(const std::string& str, const char& delimiter) {
    std::vector<std::string>  v;
    std::stringstream ss(str);
    std::string tmp;
    
    while(getline(ss, tmp, delimiter)) {
        v.push_back(tmp);
    }

    return v;
}

const char space = ' ';

void parse_and_execute(const std::string& line, File_System& fs) {
        std::istringstream iss(line);
        std::vector<std::string> el;
        el = split(line, space);
        if(el.size() == 1) el.push_back("");
        fs.execute_command(el[0], el[1]);
}

void process_from_file(File_System& fs) {
    std::string line;
    std::ifstream file("./test_input.txt");
    while (std::getline(file, line)) {
        parse_and_execute(line, fs);
    }
}

void process_from_cmd(File_System& fs) {
    std::string str;
    while (std::getline(std::cin, str)) {
        if(str == "") continue;
        parse_and_execute(str, fs);
    }
}

int main() {
    File_System fs;
    std::cout << "Do you want to read commands from file? (Y/N)?";
    char answer;
    std::cin >> answer;
    answer == 'Y' ? process_from_file(fs) : process_from_cmd(fs);
}

/*
expected output(with different time and date)
2020-02-12, 23:58:50 .
2020-02-12, 23:58:50 folder1
2020-02-12, 23:58:50 folder1
2020-02-12, 23:58:50 file1.txt

*/