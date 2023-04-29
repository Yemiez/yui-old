#pragma once
namespace yui {

template<typename Owner>
struct Badge {
private:
    Badge(Owner &owner)
            : m_owner(owner) {}
    friend Owner;

public:
    Owner &owner() { return m_owner; }
    const Owner &owner() const { return m_owner; }
private:
    Owner &m_owner;
};

}