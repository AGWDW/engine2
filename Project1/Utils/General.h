#pragma once
#include <vector>
#include <array>
#include <string>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <list>
#include <iterator>
namespace Utils {
    inline std::vector<std::string> split(const std::string& str, const std::string& delim)
    {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do
        {
            pos = str.find(delim, prev);
            if (pos == std::string::npos) pos = str.length();
            std::string token = str.substr(prev, pos - prev);
            if (!token.empty()) tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    };

    inline glm::mat4 rotate(glm::mat4 matrix, const glm::vec3& vec) {
        if (glm::all(glm::equal(vec, { 0, 0, 0 }))) return matrix;
        return glm::rotate(matrix, glm::radians(glm::length(vec)), glm::normalize(vec));
        /*for (short i = 0; i < 3; i++) {
            glm::vec3 d(0);
            d[i] = 1;
            matrix = glm::rotate(matrix, glm::radians(vec[0]), d);
        }
        return matrix;*/
    };
    inline glm::mat4 rotate(glm::mat4 matrix, const glm::quat& vec) {
        return matrix * glm::mat4_cast(vec);
    }
    template<class T>
    inline bool contains(const std::vector<T>& a, const T& b) {
        auto found = std::find(a.begin(), a.end(), b);
        return found != a.end();
    }
    inline bool contains(const std::string& a, const std::string& b) {
        const short s = b.size();
        for (short i = 0; i < a.size() - b.size(); i++) {
            const std::string& sub = a.substr(i, s);
            if (sub == b) {
                return true;
            }
        }
        return false;
    }

    inline bool contained(std::array<glm::vec2, 2> bound, glm::vec2 pos) {
        return glm::all(glm::greaterThanEqual(pos, bound[0]) && glm::lessThanEqual(pos, bound[1]));
    }
    inline bool contained(std::array<glm::vec3, 2> bound, glm::vec3 pos) {
        return glm::all(glm::greaterThanEqual(pos, bound[0]) && glm::lessThanEqual(pos, bound[1]));
    }

    inline std::string toUTF8(unsigned int codepoint)
    {
        std::string out;

        if (codepoint <= 0x7f)
            out.append(1, static_cast<char>(codepoint));
        else if (codepoint <= 0x7ff)
        {
            out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
            out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
        else if (codepoint <= 0xffff)
        {
            out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
            out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
        else
        {
            out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
            out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
        return out;
    }
    inline glm::vec3 xAxis(float mag = 1) {
        return glm::vec3(1, 0, 0) * mag;
    }
    inline glm::vec3 yAxis(float mag = 1) {
        return glm::vec3(0, 1, 0) * mag;
    }
    inline glm::vec3 zAxis(float mag = 1) {
        return glm::vec3(0, 0, 1) * mag;
    }
    inline glm::vec3 zero() {
        return glm::vec3(0);
    }
    inline glm::vec3 fill(const float& num) {
        return glm::vec3(num);
    }
    inline const std::string getFileName(const std::string& filePath, const bool& includeExtension = false) {
        std::string dilimeter = "/";
        std::array<std::string, 2> possible = {
            "/", "\\"
        };
        for (const std::string& pos : possible) {
            if (contains(filePath, pos)) {
                dilimeter = pos;
                break;
            }
        }
        auto s = split(filePath, dilimeter);
        std::string res = s.back();
        if (!includeExtension) {
            s = split(res, ".");
            res = s[0];
        }
        return res;
    }
    template<class T>
    inline const unsigned& findIndex(std::vector<T>& a, T& b) {
        auto c = std::find(a.begin(), a.end(), b);
        return std::distance(a.begin(), c);
    }
    template<class T>
    inline T& elementAt(std::list<T>& list, const int& index) {
        auto s = list.begin();
        std::advance(s, index);
        return *s;
    }
    inline glm::vec3 map(const glm::vec3& x, const glm::vec2& from, const glm::vec2& to) {
        float a = from.x;
        float b = from.y;
        float c = to.x;
        float d = to.y;
        return ((c + d) + (d - c) * ((2.0f * x - (a + b)) / (b - a))) / 2.0f;
    }
    template<class T>
    bool find(const std::list<T>& a, T& b) {
        auto it = std::find(a.begin(), a.end(), b);
        bool res = it != a.end();
        if (res) 
            b = *it;
        return res;
    }

    inline bool nan_inf(const glm::mat3& a) {
        for (char i = 0; i < 3; i++) {
            for (char j = 0; j < 3; j++) {
                if (glm::isnan(a[i][j]) || glm::isinf(a[i][j]) || glm::isinf(-a[i][j]))
                    return true;
            }
        }
        return false;
    }
    inline glm::vec3 tripProduct(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        return b * glm::dot(c, a) - a * glm::dot(c, b);
    }
    namespace BigMaths {
        using Vector6 = std::array<float, 6>;
        using Vector12 = std::array<float, 12>;
        using MassMatrix6 = std::array<std::array<float, 6>, 6>;
        using Matrix12 = std::array<std::array<float, 12>, 12>;

        inline const Vector12 combine(const Vector6& a, const Vector6& b)
        {
            Vector12 res{};
            res[0] = a[0];
            res[1] = a[1];
            res[2] = a[2];
            res[3] = a[3];
            res[4] = a[4];
            res[5] = a[5];

            res[6] =  b[0];
            res[7] =  b[1];
            res[8] =  b[2];
            res[9] =  b[3];
            res[10] = b[4];
            res[11] = b[5];
            return res;
        }

        inline const Matrix12 combine(const MassMatrix6& a, const MassMatrix6& b)
        {
            Matrix12 res{};
            for (char i = 0; i < 6; i++) {
                for (char j = 0; j < 6; j++) {
                    res[i][j] = a[i][j];
                    res[i + 6][j + 6] = b[i][j];
                }
            }
            return res;
        }
        // 1/n element wise
        inline const Matrix12 inverse(const Matrix12& m)
        {
            Matrix12 res{};
            for (char i = 0; i < 12; i++) {
                for (char j = 0; j < 12; j++) {
                    if (m[i][j] == 0) {
                        res[i][j] = 0;
                    }
                    else {
                        res[i][j] = 1.0f / m[i][j];
                    }
                }
            }
            return res;
        }
    };
}

// dot product [1 x 1]
inline float operator * (const Utils::BigMaths::Vector12& a, const Utils::BigMaths::Vector12& b)
{
    float res = 0;
    for (char i = 0; i < 12; i++) {
        res += a[i] * b[i];
    }
    return res;
}
// element wise multiplication
inline Utils::BigMaths::Vector12 operator * (const Utils::BigMaths::Vector12& a, const float& b)
{
    Utils::BigMaths::Vector12 res = a;
    for (char i = 0; i < 12; i++) {
        res[i] *= b;
    }
    return res;
}
// dot product [12 x 1]
inline Utils::BigMaths::Vector12 operator * (const Utils::BigMaths::Matrix12& a, const Utils::BigMaths::Vector12& b)
{
    Utils::BigMaths::Vector12 res{};
    for (char i = 0; i < 12; i++) {
        for (char j = 0; j < 12; j++) {
            res[i] += a[i][j] * b[i];
        }
    }
    return res;
}
// dot product [1 x 12]
inline Utils::BigMaths::Vector12 operator * (const Utils::BigMaths::Vector12& a, const Utils::BigMaths::Matrix12& b)
{
    Utils::BigMaths::Vector12 res{};
    for (char i = 0; i < 12; i++) {
        for (char j = 0; j < 12; j++) {
            res[i] += a[i] * b[i][j];
        }
    }
    return res;
}
// dot product [3 x 1]
inline glm::vec3 operator * (const glm::mat3& a, const glm::vec3& b)
{
    glm::vec3 res(0);
    for (char i = 0; i < 3; i++) {
        for (char j = 0; j < 3; j++) {
            res[i] += a[i][j] * b[i];
        }
    }
    return res;
}
// element wise addition
inline const Utils::BigMaths::Vector12 operator +(const Utils::BigMaths::Vector12& a, const Utils::BigMaths::Vector12& b)
{
    Utils::BigMaths::Vector12 res{};
    for (char i = 0; i < 12; i++)
        res[i] = a[i] + b[i];
    return res;
}