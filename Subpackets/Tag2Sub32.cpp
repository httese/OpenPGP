#include "Tag2Sub32.h"

Tag2Sub32::Tag2Sub32()
    : Tag2Subpacket(Tag2Subpacket::ID::Embedded_Signature),
      embedded()
{}

Tag2Sub32::Tag2Sub32(const Tag2Sub32 & copy)
    : Tag2Subpacket(copy),
    embedded(std::dynamic_pointer_cast<Tag2> (copy.embedded -> clone()))
{}

Tag2Sub32::Tag2Sub32(const std::string & data)
    : Tag2Sub32()
{
    read(data);
}

Tag2Sub32::~Tag2Sub32(){}

void Tag2Sub32::read(const std::string & data){
    embedded = std::make_shared <Tag2> (data);
    size = data.size();
}

std::string Tag2Sub32::show(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indent_size, ' ');
    const std::string tab(indents * indent_size, ' ');
    return tab + show_title() + "\n" + 
           tab + embedded -> show(indents, indent_size);
}

std::string Tag2Sub32::raw() const{
    return embedded -> raw();
}

Tag2::Ptr Tag2Sub32::get_embedded() const{
    return embedded;
}

void Tag2Sub32::set_embedded(const Tag2::Ptr & e){
    embedded = std::dynamic_pointer_cast<Tag2> (e -> clone());
}

Tag2Subpacket::Ptr Tag2Sub32::clone() const{
    return std::make_shared <Tag2Sub32> (*this);
}

Tag2Sub32 & Tag2Sub32::operator=(const Tag2Sub32 & copy){
    Tag2Subpacket::operator=(copy);
    embedded = std::dynamic_pointer_cast<Tag2> (copy.embedded -> clone());
    return *this;
}
