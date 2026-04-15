#include "expression.h"

#include <utility>

std::ostream& operator<<(std::ostream& out, const Expression& expr) {
    out << expr.str();
    return out;
}

Const::Const(const Int128& value) : value_(value) {
}

Const::Const(int64_t value) : value_(value) {
}

Int128 Const::eval(const Context&) const {
    return value_;
}

Expression* Const::clone() const {
    return new Const(*this);
}

std::string Const::str() const {
    return value_.str();
}

Variable::Variable(std::string name) : name_(std::move(name)) {
}

Int128 Variable::eval(const Context& context) const {
    const auto it = context.find(name_);
    if (it == context.end()) {
        return Int128();
    }
    return it->second;
}

Expression* Variable::clone() const {
    return new Variable(*this);
}

std::string Variable::str() const {
    return name_;
}

Negate::Negate(const Expression& expr) : expr_(expr.clone()) {
}

Negate::Negate(const Negate& other) : expr_(other.expr_->clone()) {
}

Negate& Negate::operator=(const Negate& other) {
    if (this != &other) {
        expr_.reset(other.expr_->clone());
    }
    return *this;
}

Int128 Negate::eval(const Context& context) const {
    return -expr_->eval(context);
}

Expression* Negate::clone() const {
    return new Negate(*this);
}

std::string Negate::str() const {
    return "(-" + expr_->str() + ")";
}

const Expression& BinaryExpression::lhs() const {
    return *lhs_;
}

const Expression& BinaryExpression::rhs() const {
    return *rhs_;
}

BinaryExpression::BinaryExpression(const Expression& lhs, const Expression& rhs)
    : lhs_(lhs.clone()), rhs_(rhs.clone()) {
}

BinaryExpression::BinaryExpression(const BinaryExpression& other)
    : lhs_(other.lhs_->clone()), rhs_(other.rhs_->clone()) {
}

BinaryExpression& BinaryExpression::operator=(const BinaryExpression& other) {
    if (this != &other) {
        lhs_.reset(other.lhs_->clone());
        rhs_.reset(other.rhs_->clone());
    }
    return *this;
}

Add::Add(const Expression& lhs, const Expression& rhs) : BinaryExpression(lhs, rhs) {
}

Int128 Add::eval(const Context& context) const {
    return lhs().eval(context) + rhs().eval(context);
}

Expression* Add::clone() const {
    return new Add(*this);
}

std::string Add::str() const {
    return "(" + lhs().str() + " + " + rhs().str() + ")";
}

Subtract::Subtract(const Expression& lhs, const Expression& rhs) : BinaryExpression(lhs, rhs) {
}

Int128 Subtract::eval(const Context& context) const {
    return lhs().eval(context) - rhs().eval(context);
}

Expression* Subtract::clone() const {
    return new Subtract(*this);
}

std::string Subtract::str() const {
    return "(" + lhs().str() + " - " + rhs().str() + ")";
}

Multiply::Multiply(const Expression& lhs, const Expression& rhs) : BinaryExpression(lhs, rhs) {
}

Int128 Multiply::eval(const Context& context) const {
    return lhs().eval(context) * rhs().eval(context);
}

Expression* Multiply::clone() const {
    return new Multiply(*this);
}

std::string Multiply::str() const {
    return "(" + lhs().str() + " * " + rhs().str() + ")";
}

Divide::Divide(const Expression& lhs, const Expression& rhs) : BinaryExpression(lhs, rhs) {
}

Int128 Divide::eval(const Context& context) const {
    return lhs().eval(context) / rhs().eval(context);
}

Expression* Divide::clone() const {
    return new Divide(*this);
}

std::string Divide::str() const {
    return "(" + lhs().str() + " / " + rhs().str() + ")";
}

Negate operator-(const Expression& expr) {
    return Negate(expr);
}

Add operator+(const Expression& lhs, const Expression& rhs) {
    return Add(lhs, rhs);
}

Subtract operator-(const Expression& lhs, const Expression& rhs) {
    return Subtract(lhs, rhs);
}

Multiply operator*(const Expression& lhs, const Expression& rhs) {
    return Multiply(lhs, rhs);
}

Divide operator/(const Expression& lhs, const Expression& rhs) {
    return Divide(lhs, rhs);
}
