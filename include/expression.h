#pragma once

#include "int128.h"

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

class Expression {
public:
    using Context = std::map<std::string, Int128>;

    virtual ~Expression() = default;

    virtual Int128 eval(const Context& context) const = 0;
    virtual Expression* clone() const = 0;
    virtual std::string str() const = 0;
};

std::ostream& operator<<(std::ostream& out, const Expression& expr);

class Const final : public Expression {
private:
    Int128 value_;

public:
    explicit Const(const Int128& value);
    explicit Const(int64_t value);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Variable final : public Expression {
private:
    std::string name_;

public:
    explicit Variable(std::string name);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Negate final : public Expression {
private:
    std::unique_ptr<Expression> expr_;

public:
    explicit Negate(const Expression& expr);
    Negate(const Negate& other);
    Negate& operator=(const Negate& other);
    Negate(Negate&&) noexcept = default;
    Negate& operator=(Negate&&) noexcept = default;
    ~Negate() override = default;

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class BinaryExpression : public Expression {
private:
    std::unique_ptr<Expression> lhs_;
    std::unique_ptr<Expression> rhs_;

protected:
    const Expression& lhs() const;
    const Expression& rhs() const;

public:
    BinaryExpression(const Expression& lhs, const Expression& rhs);
    BinaryExpression(const BinaryExpression& other);
    BinaryExpression& operator=(const BinaryExpression& other);
    BinaryExpression(BinaryExpression&&) noexcept = default;
    BinaryExpression& operator=(BinaryExpression&&) noexcept = default;
    ~BinaryExpression() override = default;
};

class Add final : public BinaryExpression {
public:
    Add(const Expression& lhs, const Expression& rhs);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Subtract final : public BinaryExpression {
public:
    Subtract(const Expression& lhs, const Expression& rhs);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Multiply final : public BinaryExpression {
public:
    Multiply(const Expression& lhs, const Expression& rhs);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

class Divide final : public BinaryExpression {
public:
    Divide(const Expression& lhs, const Expression& rhs);

    Int128 eval(const Context& context) const override;
    Expression* clone() const override;
    std::string str() const override;
};

Negate operator-(const Expression& expr);
Add operator+(const Expression& lhs, const Expression& rhs);
Subtract operator-(const Expression& lhs, const Expression& rhs);
Multiply operator*(const Expression& lhs, const Expression& rhs);
Divide operator/(const Expression& lhs, const Expression& rhs);
