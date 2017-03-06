#include "Tag9.h"

Tag9::Tag9()
    : Packet(Packet::ID::Symmetrically_Encrypted_Data),
      encrypted_data()
{}

Tag9::Tag9(const Tag9 & copy)
    : Packet(copy),
      encrypted_data(copy.encrypted_data)
{}

Tag9::Tag9(const std::string & data)
    : Tag9()
{
    read(data);
}

void Tag9::read(const std::string & data){
    size = data.size();
    encrypted_data = data;
}

std::string Tag9::show(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indents * indent_size, ' ');
    const std::string tab(indent_size, ' ');
    return indent + show_title() + "\n" +
           indent + tab + "Encrypted Data (" + std::to_string(encrypted_data.size()) + " octets): " + hexlify(encrypted_data);
}

std::string Tag9::raw() const{
    return encrypted_data;
}

std::string Tag9::get_encrypted_data() const{
    return encrypted_data;
}

void Tag9::set_encrypted_data(const std::string & e){
    encrypted_data = e;
    size = raw().size();
}

Packet::Ptr Tag9::clone() const{
    return std::make_shared <Tag9> (*this);
}
