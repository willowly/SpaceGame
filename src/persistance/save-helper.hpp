#include "cista.h"
#include <string>
#include <optional>

using std::string;

namespace SaveHelper {
    
    template<typename T>
    void save(T obj,string path) {


        std::vector<unsigned char> buf;
        {  // Serialize.
            buf = cista::serialize(obj);
        }

        FileHelper::writeBinary(path,buf);
    }

    template<typename T>
    std::optional<T> load(string path) {
        namespace data = cista::raw;
        

        auto saved_buf = FileHelper::readBinary(path);

        if(saved_buf.size() == 0) {
            std::cout << "No saved data" << std::endl;
            return std::nullopt;
        } else {
            try {
                auto deserialized = cista::deserialize<T>(saved_buf);
                return *deserialized;
            } catch (std::runtime_error e) {
                std::cout << "data is corrupt" << std::endl;
                std::cout << e.what() << std::endl;
                return std::nullopt;
            }
            
        }
    }

};