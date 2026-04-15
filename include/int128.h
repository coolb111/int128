#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>

class Int128 {
private:
    uint64_t low_;
    uint64_t high_;

    Int128(uint64_t high, uint64_t low, int);

public:
    Int128();
    Int128(int64_t value);
    explicit Int128(std::string_view value);

    explicit operator int64_t() const;
    explicit operator double() const;

    std::string str() const;

    Int128& operator+=(const Int128& other);
    Int128& operator-=(const Int128& other);
    Int128& operator*=(const Int128& other);
    Int128& operator/=(const Int128& other);

    Int128 operator-() const;

    friend Int128 operator+(Int128 lhs, const Int128& rhs);
    friend Int128 operator-(Int128 lhs, const Int128& rhs);
    friend Int128 operator*(Int128 lhs, const Int128& rhs);
    friend Int128 operator/(Int128 lhs, const Int128& rhs);

    friend bool operator==(const Int128& lhs, const Int128& rhs);
    friend bool operator!=(const Int128& lhs, const Int128& rhs);

    friend std::ostream& operator<<(std::ostream& out, const Int128& value);
};
