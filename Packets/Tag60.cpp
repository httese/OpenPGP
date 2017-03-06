#include "Tag60.h"

Tag60::Tag60()
    : Tag60(std::string())
{}

Tag60::Tag60(const Tag60 & copy)
    : Packet(copy),
      stream(copy.stream)
{}

Tag60::Tag60(const std::string & data)
    : Packet(60),
      stream(data)
{}

void Tag60::read(const std::string & data){
    stream = data;
}

std::string Tag60::show(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indents * indent_size, ' ');
    const std::string tab(indent_size, ' ');
    return indent + show_title() + "\n" + 
           indent + tab + hexlify(stream);
}

std::string Tag60::raw() const{
    return stream;
}

std::string Tag60::get_stream() const{
    return stream;
}

void Tag60::set_stream(const std::string & data){
    stream = data;
}

Packet::Ptr Tag60::clone() const{
    return std::make_shared <Tag60> (*this);
}
