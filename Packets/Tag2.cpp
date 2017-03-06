#include "Tag2.h"

Tag2::Tag2()
    : Packet(Packet::ID::Signature),
      type(0),
      pka(0),
      hash(0),
      mpi(),
      left16(),
      time(0),
      keyid(),
      hashed_subpackets(),
      unhashed_subpackets()
{}

Tag2::Tag2(const Tag2 & copy)
    : Packet(copy),
      type(copy.type),
      pka(copy.pka),
      hash(copy.hash),
      mpi(copy.mpi),
      left16(copy.left16),
      time(copy.time),
      keyid(copy.keyid),
      hashed_subpackets(copy.get_hashed_subpackets_clone()),
      unhashed_subpackets(copy.get_unhashed_subpackets_clone())
{}

Tag2::Tag2(const std::string & data)
    : Tag2()
{
    read(data);
}

Tag2::~Tag2(){
    hashed_subpackets.clear();
    unhashed_subpackets.clear();
}

// Extracts Subpacket data for figuring which subpacket type to create
void Tag2::read_subpacket(const std::string & data, std::string::size_type & pos, std::string::size_type & length){
    length = 0;

    uint8_t first_octet = static_cast <unsigned char> (data[pos]);
    if (first_octet < 192){
        length = first_octet;
        pos += 1;
    }
    else if ((192 <= first_octet) && (first_octet < 255)){
        length = toint(data.substr(pos, 2), 256) - (192 << 8) + 192;
        pos += 2;
    }
    else if (first_octet == 255){
        length = toint(data.substr(pos + 1, 4), 256);
        pos += 5;
    }
}

void Tag2::read_subpackets(const std::string & data, Tag2::Subpackets_T & subpackets){
    subpackets.clear();
    std::string::size_type pos = 0;
    while (pos < data.size()){
        // read subpacket data out
        std::string::size_type length;
        read_subpacket(data, pos, length);  // pos moved past header to [length + data]

        Tag2Subpacket::Ptr subpacket = nullptr;
        switch (data[pos]){                 // first octet of data is subpacket type
            // reserved sub values will crash the program
            case 2:
                subpacket = std::make_shared <Tag2Sub2> ();
                break;
            case 3:
                subpacket = std::make_shared <Tag2Sub3> ();
                break;
            case 4:
                subpacket = std::make_shared <Tag2Sub4> ();
                break;
            case 5:
                subpacket = std::make_shared <Tag2Sub5> ();
                break;
            case 6:
                subpacket = std::make_shared <Tag2Sub6> ();
                break;
            case 9:
                subpacket = std::make_shared <Tag2Sub9> ();
                break;
            case 10:
                subpacket = std::make_shared <Tag2Sub10> ();
                break;
            case 11:
                subpacket = std::make_shared <Tag2Sub11> ();
                break;
            case 12:
                subpacket = std::make_shared <Tag2Sub12> ();
                break;
            case 16:
                subpacket = std::make_shared <Tag2Sub16> ();
                break;
            case 20:
                subpacket = std::make_shared <Tag2Sub20> ();
                break;
            case 21:
                subpacket = std::make_shared <Tag2Sub21> ();
                break;
            case 22:
                subpacket = std::make_shared <Tag2Sub22> ();
                break;
            case 23:
                subpacket = std::make_shared <Tag2Sub23> ();
                break;
            case 24:
                subpacket = std::make_shared <Tag2Sub24> ();
                break;
            case 25:
                subpacket = std::make_shared <Tag2Sub25> ();
                break;
            case 26:
                subpacket = std::make_shared <Tag2Sub26> ();
                break;
            case 27:
                subpacket = std::make_shared <Tag2Sub27> ();
                break;
            case 28:
                subpacket = std::make_shared <Tag2Sub28> ();
                break;
            case 29:
                subpacket = std::make_shared <Tag2Sub29> ();
                break;
            case 30:
                subpacket = std::make_shared <Tag2Sub30> ();
                break;
            case 31:
                subpacket = std::make_shared <Tag2Sub31> ();
                break;
            case 32:
                subpacket = std::make_shared <Tag2Sub32> ();
                break;
            default:
                throw std::runtime_error("Error: Subpacket tag not defined or reserved.");
                break;
        }

        // subpacket guaranteed to be defined
        subpacket -> read(data.substr(pos + 1, length - 1));
        subpackets.push_back(subpacket);

        // go to end of current subpacket
        pos += length;
    }
}

void Tag2::read(const std::string & data){
    size = data.size();
    tag = 2;
    version = data[0];
    if (version < 4){
        if (data[1] != 5){
            throw std::runtime_error("Error: Length of hashed material must be 5.");
        }
        type   = data[2];
        time   = toint(data.substr(3, 4), 256);
        keyid  = data.substr(7, 8);

        pka    = data[15];
        hash   = data[16];
        left16 = data.substr(17, 2);
        std::string::size_type pos = 19;
        if (pka < 4){
            mpi.push_back(read_MPI(data, pos)); // RSA m**d mod n
        }
        if (pka == 17){
            mpi.push_back(read_MPI(data, pos)); // DSA r
            mpi.push_back(read_MPI(data, pos)); // DSA s
        }
    }
    else if (version == 4){
        type = data[1];
        pka  = data[2];
        hash = data[3];

        // hashed subpackets
        const uint16_t hashed_size = toint(data.substr(4, 2), 256);
        read_subpackets(data.substr(6, hashed_size), hashed_subpackets);

        // unhashed subpacketss
        const uint16_t unhashed_size = toint(data.substr(hashed_size + 6, 2), 256);
        read_subpackets(data.substr(hashed_size + 6 + 2, unhashed_size), unhashed_subpackets);

        // get left 16 bits
        left16 = data.substr(hashed_size + 6 + 2 + unhashed_size, 2);

//        if (pka < 4)
        std::string::size_type pos = hashed_size + 6 + 2 + unhashed_size + 2;
        mpi.push_back(read_MPI(data, pos));         // RSA m**d mod n
        if (pka == 17){
//            mpi.push_back(read_MPI(data, pos));   // DSA r
            mpi.push_back(read_MPI(data, pos));     // DSA s
        }
    }
    else{
        throw std::runtime_error("Error: Tag2 Unknown version: " + std::to_string(static_cast <unsigned int> (version)));
    }
}

std::string Tag2::show(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indents * indent_size, ' ');
    const std::string tab(indent_size, ' ');

    std::string out = indent + show_title() + "\n" +
                      indent + tab + "Version: " + std::to_string(version) + "\n";
    if (version < 4){
        out += indent + tab + "Hashed Material:\n" +
               indent + tab + tab + "Signature Type: " + Signature_Type::Name.at(type) + " (type 0x" + makehex(type, 2) + ")\n" +
               indent + tab + tab + "Creation Time: " + show_time(time) + "\n" +
               indent + tab + "Signer's Key ID: " + hexlify(keyid) + "\n" +
               indent + tab + "Public Key Algorithm: " + PKA::Name.at(pka) + " (pka " + std::to_string(pka) + ")\n" +
               indent + tab + "Hash Algorithm: " + Hash::Name.at(hash) + " (hash " + std::to_string(hash) + ")\n";
    }
    else if (version == 4){
        out += indent + tab + "Signature Type: " + Signature_Type::Name.at(type) + " (type 0x" + makehex(type, 2) + ")\n" +
               indent + tab + "Public Key Algorithm: " + PKA::Name.at(pka) + " (pka " + std::to_string(pka) + ")\n" +
               indent + tab + "Hash Algorithm: " + Hash::Name.at(hash) + " (hash " + std::to_string(hash) + ")";

        if (hashed_subpackets.size()){
            time_t create_time = 0;

            out += "\n" + indent + tab + "Hashed Sub:";
            for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
                // capture signature creation time to combine with expiration time
                if (s -> get_type() == Tag2Subpacket::ID::Signature_Creation_Time){
                    create_time = std::static_pointer_cast <Tag2Sub2> (s) -> get_time();
                }

                if (s -> get_type() == Tag2Subpacket::ID::Key_Expiration_Time){
                    out += "\n" + std::static_pointer_cast <Tag2Sub9> (s) -> show(create_time, indents + 2, indent_size);
                }
                else{
                    out += "\n" + s -> show(indents + 2, indent_size);
                }
            }
        }

        if (unhashed_subpackets.size()){
            time_t create_time = 0;

            out += "\n" + indent + tab + "Unhashed Sub:";
            for(Tag2Subpacket::Ptr const & s : unhashed_subpackets){
                // capture signature creation time to combine with expiration time
                if (s -> get_type() == Tag2Subpacket::ID::Signature_Creation_Time){
                    create_time = std::static_pointer_cast <Tag2Sub2> (s) -> get_time();
                }

                if (s -> get_type() == Tag2Subpacket::ID::Key_Expiration_Time){
                    out += "\n" + std::static_pointer_cast <Tag2Sub9> (s) -> show(create_time, indents + 2, indent_size);
                }
                else{
                    out += "\n" + s -> show(indents + 2, indent_size);
                }
            }
        }
    }

    out += "\n" + indent + tab + "Hash Left 16 Bits: " + hexlify(left16);

    if (pka < 4){
        out += "\n" + indent + tab + "RSA m**d mod n (" + std::to_string(bitsize(mpi[0])) + " bits): " + mpitohex(mpi[0]);
    }
    else if (pka == 17){
        out += "\n" + indent + tab + "DSA r (" + std::to_string(bitsize(mpi[0])) + " bits): " + mpitohex(mpi[0])
            += "\n" + indent + tab + "DSA s (" + std::to_string(bitsize(mpi[1])) + " bits): " + mpitohex(mpi[1]);
    }

    return out;
}

std::string Tag2::raw() const{
    std::string out(1, version);
    if (version < 4){// to recreate older keys
        out += "\x05" + std::string(1, type) + unhexlify(makehex(time, 8)) + keyid + std::string(1, pka) + std::string(1, hash) + left16;
    }
    if (version == 4){
        std::string hashed_str = "";
        for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
            hashed_str += s -> write();
        }
        std::string unhashed_str = "";
        for(Tag2Subpacket::Ptr const & s : unhashed_subpackets){
            unhashed_str += s -> write();
        }
        out += std::string(1, type) + std::string(1, pka) + std::string(1, hash) + unhexlify(makehex(hashed_str.size(), 4)) + hashed_str + unhexlify(makehex(unhashed_str.size(), 4)) + unhashed_str + left16;
    }
    for(PGPMPI const & i : mpi){
        out += write_MPI(i);
    }
    return out;
}

uint8_t Tag2::get_type() const{
    return type;
}

uint8_t Tag2::get_pka() const{
    return pka;
}

uint8_t Tag2::get_hash() const{
    return hash;
}

std::string Tag2::get_left16() const{
    return left16;
}

PKA::Values Tag2::get_mpi() const{
    return mpi;
}

uint32_t Tag2::get_time() const{
    if (version == 3){
        return time;
    }
    else if (version == 4){
        for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
            if (s -> get_type() == 2){
                return Tag2Sub2(s -> raw()).get_time();
            }
        }
    }
    return 0;
}

std::string Tag2::get_keyid() const{
    if (version == 3){
        return keyid;
    }
    else if (version == 4){
        // usually found in unhashed subpackets
        for(Tag2Subpacket::Ptr const & s : unhashed_subpackets){
            if (s -> get_type() == 16){
                return Tag2Sub16(s -> raw()).get_keyid();
            }
        }
        // search hashed subpackets if necessary
        for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
            if (s -> get_type() == 16){
                return Tag2Sub16(s -> raw()).get_keyid();
            }
        }
    }
    else{
        throw std::runtime_error("Error: Signature Packet version " + std::to_string(version) + " not defined.");
    }
    return ""; // should never reach here; mainly just to remove compiler warnings
}

Tag2::Subpackets_T Tag2::get_hashed_subpackets() const{
    return hashed_subpackets;
}

Tag2::Subpackets_T Tag2::get_hashed_subpackets_clone() const{
    std::vector <Tag2Subpacket::Ptr> out;
    for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
        out.push_back(s -> clone());
    }
    return out;
}

Tag2::Subpackets_T Tag2::get_unhashed_subpackets() const{
    return unhashed_subpackets;
}

Tag2::Subpackets_T Tag2::get_unhashed_subpackets_clone() const{
    std::vector <Tag2Subpacket::Ptr> out;
    for(Tag2Subpacket::Ptr const & s : unhashed_subpackets){
        out.push_back(s -> clone());
    }
    return out;
}

std::string Tag2::get_up_to_hashed() const{
    if (version == 3){
        return "\x03" + std::string(1, type) + unhexlify(makehex(time, 8));
    }
    else if (version == 4){
        std::string hashed = "";
        for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
            hashed += s -> write();
        }
        return "\x04" + std::string(1, type) + std::string(1, pka) + std::string(1, hash) + unhexlify(makehex(hashed.size(), 4)) + hashed;
    }
    else{
        throw std::runtime_error("Error: Signature packet version " + std::to_string(version) + " not defined.");
    }
    return ""; // should never reach here; mainly just to remove compiler warnings
}

std::string Tag2::get_without_unhashed() const{
    std::string out(1, version);
    if (version < 4){// to recreate older keys
        out += "\x05" + std::string(1, type) + unhexlify(makehex(time, 8)) + keyid + std::string(1, pka) + std::string(1, hash) + left16;
    }
    if (version == 4){
        std::string hashed_str = "";
        for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
            hashed_str += s -> write();
        }
        out += std::string(1, type) + std::string(1, pka) + std::string(1, hash) + unhexlify(makehex(hashed_str.size(), 4)) + hashed_str + zero + zero + left16;
    }
    for(PGPMPI const & i : mpi){
        out += write_MPI(i);
    }
    return out;
}

void Tag2::set_pka(const uint8_t p){
    pka = p;
    size = raw().size();
}

void Tag2::set_type(const uint8_t t){
    type = t;
    size = raw().size();
}

void Tag2::set_hash(const uint8_t h){
    hash = h;
    size = raw().size();
}

void Tag2::set_left16(const std::string & l){
    left16 = l;
    size = raw().size();
}

void Tag2::set_mpi(const PKA::Values & m){
    mpi = m;
    size = raw().size();
}

void Tag2::set_time(const uint32_t t){
    if (version == 3){
        time = t;
    }
    else if (version == 4){
        unsigned int i;
        for(i = 0; i < hashed_subpackets.size(); i++){
            if (hashed_subpackets[i] -> get_type() == 2){
                break;
            }
        }
        Tag2Sub2::Ptr sub2 = std::make_shared <Tag2Sub2> ();
        sub2 -> set_time(t);
        if (i == hashed_subpackets.size()){ // not found
            hashed_subpackets.push_back(sub2);
        }
        else{                               // found
            hashed_subpackets[i] = sub2;
        }
    }
    size = raw().size();
}

void Tag2::set_keyid(const std::string & k){
    if (k.size() != 8){
        throw std::runtime_error("Error: Key ID must be 8 octets.");
    }

    if (version == 3){
        keyid = k;
    }
    else if (version == 4){
        unsigned int i;
        for(i = 0; i < unhashed_subpackets.size(); i++){
            if (unhashed_subpackets[i] -> get_type() == 16){
                break;
            }
        }
        Tag2Sub16::Ptr sub16 = std::make_shared <Tag2Sub16> ();
        sub16 -> set_keyid(k);
        if (i == unhashed_subpackets.size()){   // not found
            unhashed_subpackets.push_back(sub16);
        }
        else{                                   // found
            unhashed_subpackets[i] = sub16;
        }
    }
    size = raw().size();
}

void Tag2::set_hashed_subpackets(const Tag2::Subpackets_T & h){
    hashed_subpackets.clear();
    for(Tag2Subpacket::Ptr const & s : h){
        hashed_subpackets.push_back(s -> clone());
    }
    size = raw().size();
}

void Tag2::set_unhashed_subpackets(const Tag2::Subpackets_T & u){
    unhashed_subpackets.clear();
    for(Tag2Subpacket::Ptr const & s : u){
        unhashed_subpackets.push_back(s -> clone());
    }
    size = raw().size();
}

std::string Tag2::find_subpacket(const uint8_t sub) const{
    // 5.2.4.1. Subpacket Hints
    //
    //   It is certainly possible for a signature to contain conflicting
    //   information in subpackets. For example, a signature may contain
    //   multiple copies of a preference or multiple expiration times. In
    //   most cases, an implementation SHOULD use the last subpacket in the
    //   signature, but MAY use any conflict resolution scheme that makes
    //   more sense.

    std::string out;
    for(Tag2Subpacket::Ptr const & s : hashed_subpackets){
        if (s -> get_type() == sub){
            out = s -> raw();
            break;
        }
    }
    for(Tag2Subpacket::Ptr const & s : unhashed_subpackets){
        if (s -> get_type() == sub){
            out = s -> raw();
            break;
        }
    }
    return out;
}

Packet::Ptr Tag2::clone() const{
    Ptr out = std::make_shared <Tag2> (*this);
    out -> hashed_subpackets = get_hashed_subpackets_clone();
    out -> unhashed_subpackets = get_unhashed_subpackets_clone();
    return out;
}

Tag2 & Tag2::operator=(const Tag2 & copy){
    Packet::operator=(copy);
    type = copy.type;
    pka = copy.pka;
    hash = copy.hash;
    mpi = copy.mpi;
    left16 = copy.left16;
    time = copy.time;
    keyid = copy.keyid;
    hashed_subpackets = copy.get_hashed_subpackets_clone();
    unhashed_subpackets = copy.get_unhashed_subpackets_clone();
    return *this;
}
