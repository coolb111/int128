#include "int128.h"

#include <algorithm>
#include <cctype>
#include <ostream>

namespace {

struct UInt128Parts {
    uint64_t high;
    uint64_t low;
};

UInt128Parts make_parts(uint64_t high = 0, uint64_t low = 0) {
    return UInt128Parts{high, low};
}

bool is_zero(const UInt128Parts& value) {
    return value.high == 0 && value.low == 0;
}

bool get_bit(const UInt128Parts& value, int index) {
    if (index < 64) {
        return ((value.low >> index) & 1ULL) != 0;
    }
    return ((value.high >> (index - 64)) & 1ULL) != 0;
}

void set_bit(UInt128Parts& value, int index) {
    if (index < 64) {
        value.low |= (1ULL << index);
    } else {
        value.high |= (1ULL << (index - 64));
    }
}

UInt128Parts shl1(UInt128Parts value) {
    value.high = (value.high << 1) | (value.low >> 63);
    value.low <<= 1;
    return value;
}

UInt128Parts shr1(UInt128Parts value) {
    value.low = (value.low >> 1) | (value.high << 63);
    value.high >>= 1;
    return value;
}

UInt128Parts add_u128(UInt128Parts lhs, const UInt128Parts& rhs) {
    const uint64_t old_low = lhs.low;
    lhs.low += rhs.low;
    lhs.high += rhs.high + (lhs.low < old_low ? 1ULL : 0ULL);
    return lhs;
}

UInt128Parts sub_u128(UInt128Parts lhs, const UInt128Parts& rhs) {
    const uint64_t old_low = lhs.low;
    lhs.low -= rhs.low;
    lhs.high -= rhs.high + (old_low < rhs.low ? 1ULL : 0ULL);
    return lhs;
}

bool less_u128(const UInt128Parts& lhs, const UInt128Parts& rhs) {
    if (lhs.high != rhs.high) {
        return lhs.high < rhs.high;
    }
    return lhs.low < rhs.low;
}

UInt128Parts mul_u128(UInt128Parts lhs, UInt128Parts rhs) {
    UInt128Parts result = make_parts();
    while (!is_zero(rhs)) {
        if ((rhs.low & 1ULL) != 0) {
            result = add_u128(result, lhs);
        }
        lhs = shl1(lhs);
        rhs = shr1(rhs);
    }
    return result;
}

UInt128Parts div_u128(UInt128Parts dividend, const UInt128Parts& divisor) {
    if (is_zero(divisor)) {
        return make_parts();
    }

    UInt128Parts quotient = make_parts();
    UInt128Parts remainder = make_parts();

    for (int bit = 127; bit >= 0; --bit) {
        remainder = shl1(remainder);
        if (get_bit(dividend, bit)) {
            remainder.low |= 1ULL;
        }
        if (!less_u128(remainder, divisor)) {
            remainder = sub_u128(remainder, divisor);
            set_bit(quotient, bit);
        }
    }

    return quotient;
}

UInt128Parts divmod10(UInt128Parts value, uint32_t& remainder) {
    UInt128Parts quotient = make_parts();
    uint32_t rem = 0;

    for (int bit = 127; bit >= 0; --bit) {
        rem = static_cast<uint32_t>(rem * 2U + (get_bit(value, bit) ? 1U : 0U));
        if (rem >= 10U) {
            rem -= 10U;
            set_bit(quotient, bit);
        }
    }

    remainder = rem;
    return quotient;
}

} // namespace

Int128::Int128() : low_(0), high_(0) {
}

Int128::Int128(uint64_t high, uint64_t low, int) : low_(low), high_(high) {
}

Int128::Int128(int64_t value)
    : low_(static_cast<uint64_t>(value)),
      high_(value < 0 ? ~uint64_t(0) : 0) {
}

Int128::Int128(std::string_view text) : low_(0), high_(0) {
    if (text.empty()) {
        return;
    }

    std::size_t pos = 0;
    bool negative = false;

    if (text[pos] == '+' || text[pos] == '-') {
        negative = (text[pos] == '-');
        ++pos;
    }

    Int128 result;
    for (; pos < text.size(); ++pos) {
        const unsigned char ch = static_cast<unsigned char>(text[pos]);
        if (!std::isdigit(ch)) {
            break;
        }
        result *= Int128(10);
        result += Int128(static_cast<int64_t>(text[pos] - '0'));
    }

    *this = negative ? -result : result;
}

Int128::operator int64_t() const {
    return static_cast<int64_t>(low_);
}

Int128::operator double() const {
    UInt128Parts magnitude{high_, low_};
    const bool negative = (high_ >> 63) != 0;
    if (negative) {
        magnitude.high = ~magnitude.high;
        magnitude.low = ~magnitude.low;
        magnitude = add_u128(magnitude, make_parts(0, 1));
    }

    const double hi = static_cast<double>(magnitude.high) * 18446744073709551616.0;
    const double lo = static_cast<double>(magnitude.low);
    const double value = hi + lo;
    return negative ? -value : value;
}

std::string Int128::str() const {
    if (high_ == 0 && low_ == 0) {
        return "0";
    }

    UInt128Parts current{high_, low_};
    const bool negative = (high_ >> 63) != 0;
    if (negative) {
        current.high = ~current.high;
        current.low = ~current.low;
        current = add_u128(current, make_parts(0, 1));
    }

    std::string digits;
    while (!is_zero(current)) {
        uint32_t remainder = 0;
        current = divmod10(current, remainder);
        digits.push_back(static_cast<char>('0' + remainder));
    }

    if (negative) {
        digits.push_back('-');
    }

    std::reverse(digits.begin(), digits.end());
    return digits;
}

Int128& Int128::operator+=(const Int128& other) {
    const uint64_t old_low = low_;
    low_ += other.low_;
    high_ += other.high_ + (low_ < old_low ? 1ULL : 0ULL);
    return *this;
}

Int128& Int128::operator-=(const Int128& other) {
    return *this += (-other);
}

Int128& Int128::operator*=(const Int128& other) {
    UInt128Parts lhs{high_, low_};
    UInt128Parts rhs{other.high_, other.low_};
    const bool negative = ((high_ >> 63) != 0) != ((other.high_ >> 63) != 0);

    if ((high_ >> 63) != 0) {
        lhs.high = ~lhs.high;
        lhs.low = ~lhs.low;
        lhs = add_u128(lhs, make_parts(0, 1));
    }
    if ((other.high_ >> 63) != 0) {
        rhs.high = ~rhs.high;
        rhs.low = ~rhs.low;
        rhs = add_u128(rhs, make_parts(0, 1));
    }

    UInt128Parts result = mul_u128(lhs, rhs);
    if (negative) {
        result.high = ~result.high;
        result.low = ~result.low;
        result = add_u128(result, make_parts(0, 1));
    }

    high_ = result.high;
    low_ = result.low;
    return *this;
}

Int128& Int128::operator/=(const Int128& other) {
    if (other.high_ == 0 && other.low_ == 0) {
        high_ = 0;
        low_ = 0;
        return *this;
    }

    UInt128Parts lhs{high_, low_};
    UInt128Parts rhs{other.high_, other.low_};
    const bool negative = ((high_ >> 63) != 0) != ((other.high_ >> 63) != 0);

    if ((high_ >> 63) != 0) {
        lhs.high = ~lhs.high;
        lhs.low = ~lhs.low;
        lhs = add_u128(lhs, make_parts(0, 1));
    }
    if ((other.high_ >> 63) != 0) {
        rhs.high = ~rhs.high;
        rhs.low = ~rhs.low;
        rhs = add_u128(rhs, make_parts(0, 1));
    }

    UInt128Parts result = div_u128(lhs, rhs);
    if (negative) {
        result.high = ~result.high;
        result.low = ~result.low;
        result = add_u128(result, make_parts(0, 1));
    }

    high_ = result.high;
    low_ = result.low;
    return *this;
}

Int128 Int128::operator-() const {
    UInt128Parts raw{high_, low_};
    raw.high = ~raw.high;
    raw.low = ~raw.low;
    raw = add_u128(raw, make_parts(0, 1));
    return Int128(raw.high, raw.low, 0);
}

Int128 operator+(Int128 lhs, const Int128& rhs) {
    lhs += rhs;
    return lhs;
}

Int128 operator-(Int128 lhs, const Int128& rhs) {
    lhs -= rhs;
    return lhs;
}

Int128 operator*(Int128 lhs, const Int128& rhs) {
    lhs *= rhs;
    return lhs;
}

Int128 operator/(Int128 lhs, const Int128& rhs) {
    lhs /= rhs;
    return lhs;
}

bool operator==(const Int128& lhs, const Int128& rhs) {
    return lhs.high_ == rhs.high_ && lhs.low_ == rhs.low_;
}

bool operator!=(const Int128& lhs, const Int128& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Int128& value) {
    out << value.str();
    return out;
}
