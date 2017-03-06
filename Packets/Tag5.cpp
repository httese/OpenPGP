#include "Tag5.h"

Tag5::Tag5(uint8_t tag)
    : Tag6(tag),
      s2k_con(0),
      sym(0),
      s2k(),
      IV(),
      secret()
{}

Tag5::Tag5()
    : Tag5(Packet::ID::Secret_Key)
{}

Tag5::Tag5(const Tag5 & copy)
    : Tag6(copy),
      s2k_con(copy.s2k_con),
      sym(copy.sym),
      s2k(copy.s2k),
      IV(copy.IV),
      secret(copy.secret)
{}

Tag5::Tag5(const std::string & data)
    : Tag5(Packet::ID::Secret_Key)
{
    read(data);
}

Tag5::~Tag5(){}

void Tag5::read_s2k(const std::string & data, std::string::size_type & pos){
    s2k.reset();

    if (data[pos] == S2K::ID::Simple_S2K){
        s2k = std::make_shared <S2K0> ();
    }
    else if (data[pos] == S2K::ID::Salted_S2K){
        s2k = std::make_shared <S2K1> ();
    }
    else if (data[pos] == S2K::ID::Iterated_and_Salted_S2K){
        s2k = std::make_shared <S2K3> ();
    }
    else{
        throw std::runtime_error("Error: Bad S2K ID encountered: " + std::to_string(data[0]));
    }

    s2k -> read(data, pos);
}

std::string Tag5::show_private(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indents * indent_size, ' ');
    const std::string tab(indent_size, ' ');
    std::string out;
    if (s2k_con > 253){
        out += indent + tab + "String-to-Key Usage Conventions: " + std::to_string(s2k_con) + "\n" +
               indent + tab + "Symmetric Key Algorithm: " + Sym::Name.at(sym) + " (sym " + std::to_string(sym) + ")\n" +
               tab + s2k -> show(indents) + "\n";
        if (s2k -> get_type()){
            out += indent + tab + "IV: " + hexlify(IV) + "\n";
        }
    }

    out += indent + tab + "Encrypted Data (" + std::to_string(secret.size()) + " octets):\n" +
           indent + tab + tab;

    if ((pka == PKA::ID::RSA_Encrypt_or_Sign) ||
        (pka == PKA::ID::RSA_Encrypt_Only)    ||
        (pka == PKA::ID::RSA_Sign_Only)){
        out += "RSA d, p, q, u";
    }
    else if (pka == PKA::ID::ElGamal){
        out += "Elgamal x";
    }
    else if (pka == PKA::ID::DSA){
        out += "DSA x";
    }
    out += " + ";

    if (s2k_con == 254){
        out += "SHA1 hash";
    }
    else{
        out += "2 Octet Checksum";
    }

    out += ": " + hexlify(secret);
    return out;
}

void Tag5::read(const std::string & data){
    size = data.size();
    std::string::size_type pos = 0;

    // public data
    read_common(data, pos);

    // S2K usage octet
    s2k_con = data[pos++];
    if ((s2k_con != 0) && (s2k_con != 254) && (s2k_con != 255)){
        sym = s2k_con;
    }

    // one octet symmetric key encryption algorithm
    if ((s2k_con == 254) || (s2k_con == 255)){
        sym = data[pos++];
    }

    // S2K specifier
    if ((s2k_con == 254) || (s2k_con == 255)){
        read_s2k(data, pos);
    }

    // IV
    if (s2k_con){
        IV = data.substr(pos, Sym::Block_Length.at(sym) >> 3);
        pos += IV.size();
    }

    // plaintex or encrypted data
    secret = data.substr(pos, data.size() - pos);
}

std::string Tag5::show(const uint8_t indents, const uint8_t indent_size) const{
    const std::string indent(indents * indent_size, ' ');
    const std::string tab(indent_size, ' ');
    return indent + show_title() + "\n" +
           show_common(indents, indent_size) + "\n" +
           show_private(indents, indent_size);
}

std::string Tag5::raw() const{
    std::string out = raw_common() +            // public data
                      std::string(1, s2k_con);  // S2K usage octet
    if ((s2k_con == 254) || (s2k_con == 255)){
        if (!s2k){
            throw std::runtime_error("Error: S2K has not been set.");
        }
        out += std::string(1, sym);             // one octet symmetric key encryption algorithm
    }

    if ((s2k_con == 254) || (s2k_con == 255)){
        if (!s2k){
            throw std::runtime_error("Error: S2K has not been set.");
        }
        out += s2k -> write();                  // S2K specifier
    }

    if (s2k_con){
        out += IV;                              // IV
    }

    return out + secret;
}

uint8_t Tag5::get_s2k_con() const{
    return s2k_con;
}

uint8_t Tag5::get_sym() const{
    return sym;
}

S2K::Ptr Tag5::get_s2k() const{
    return s2k;
}

S2K::Ptr Tag5::get_s2k_clone() const{
    return s2k -> clone();
}

std::string Tag5::get_IV() const{
    return IV;
}

std::string Tag5::get_secret() const{
    return secret;
}

Tag6 Tag5::get_public_obj() const{
    Tag6 out(raw());
    out.set_tag(Packet::ID::Public_Key);
    return out;
}

Tag6::Ptr Tag5::get_public_ptr() const{
    return std::make_shared <Tag6> (raw());
}

void Tag5::set_s2k_con(const uint8_t c){
    s2k_con = c;
    size = raw_common().size() + 1;
    if (s2k){
        size += s2k -> write().size();
    }
    if (s2k_con){
        size += IV.size();
    }
    size += secret.size();
}

void Tag5::set_sym(const uint8_t s){
    sym = s;
    size = raw_common().size() + 1;
    if (s2k){
        size += s2k -> write().size();
    }
    if (s2k_con){
        size += IV.size();
    }
    size += secret.size();
}

void Tag5::set_s2k(const S2K::Ptr & s){
    if (s -> get_type() == S2K::ID::Simple_S2K){
        s2k = std::make_shared <S2K0> ();
    }
    else if (s -> get_type() == S2K::ID::Salted_S2K){
        s2k = std::make_shared <S2K1> ();
    }
    else if (s -> get_type() == S2K::ID::Iterated_and_Salted_S2K){
        s2k = std::make_shared <S2K3> ();
    }
    s2k = s -> clone();
    size = raw_common().size() + 1;
    if (s2k){
        size += s2k -> write().size();
    }
    if (s2k_con){
        size += IV.size();
    }
    size += secret.size();
}

void Tag5::set_IV(const std::string & iv){
    IV = iv;
    size = raw_common().size() + 1;
    if (s2k){
        size += s2k -> write().size();
    }
    if (s2k_con){
        size += IV.size();
    }
    size += secret.size();
}

void Tag5::set_secret(const std::string & s){
    secret = s;
    size = raw_common().size() + 1;
    if (s2k){
        size += s2k -> write().size();
    }
    if (s2k_con){
        size += IV.size();
    }
    size += secret.size();
}

std::string Tag5::calculate_key(const std::string & passphrase) const {
    std::string key;

    if (s2k_con == 0){                                  // No S2K
        // not encrypted
    }
    else if ((s2k_con == 254) || (s2k_con == 255)){     // S2K exists
        if (!s2k){
            throw std::runtime_error("Error: S2K has not been set.");
        }

        key = s2k -> run(passphrase, Sym::Key_Length.at(sym) >> 3);
    }
    else{
        key = MD5(passphrase).digest();                 // simple MD5 for all other values
    }

    return key;
}

const std::string & Tag5::encrypt_secret_keys(const std::string & passphrase, const PKA::Values & keys){
    secret = "";

    // convert keys into string
    for(PGPMPI const & mpi : keys){
        secret += write_MPI(mpi);
    }

    // calculate checksum
    if(s2k_con == 254){
        secret += use_hash(s2k -> get_hash(), secret); // SHA1
    }
    else{
        uint16_t sum = 0;
        for(char const & c : secret){
            sum += static_cast <unsigned char> (c);
        }

        secret += unhexlify(makehex(sum, 4));
    }

    if (s2k_con){   // secret needs to be encrypted
        // calculate key to encrypt
        const std::string key = calculate_key(passphrase);

        // encrypt
        secret = use_normal_CFB_encrypt(sym, secret, key, IV);
    }

    return secret;
}

PKA::Values Tag5::decrypt_secret_keys(const std::string & passphrase) const {
    std::string keys;

    // S2k != 0 -> secret keys are encrypted
    if (s2k_con){
        // calculate key to decrypt
        const std::string key = calculate_key(passphrase);

        // decrypt
        keys = use_normal_CFB_decrypt(sym, secret, key, IV);
    }
    else{
        keys = secret;
    }

    // remove checksum from cleartext key string
    const unsigned int hash_size = (s2k_con == 254)?20:2;
    const std::string given_checksum = keys.substr(keys.size() - hash_size, hash_size);
    keys = keys.substr(0, keys.size() - hash_size);
    std::string calculated_checksum;

    // calculate and check checksum
    if(s2k_con == 254){
        calculated_checksum = use_hash(s2k -> get_hash(), keys); // SHA1
    }
    else{
        uint16_t sum = 0;
        for(char const & c : keys){
            sum += static_cast <unsigned char> (c);
        }

        calculated_checksum = unhexlify(makehex(sum, 4));
    }

    if (calculated_checksum != given_checksum){
        throw std::runtime_error("Error: Secret key checksum and calculated checksum do not match. ");
    }

    // extract MPI values
    PKA::Values out;
    std::string::size_type pos = 0;
    while (pos < keys.size()){
        out.push_back(read_MPI(keys, pos));
    }

    return out;
}

Packet::Ptr Tag5::clone() const{
    Ptr out = std::make_shared <Tag5> (*this);
    out -> s2k = s2k?s2k -> clone():nullptr;
    return out;
}

Tag5 & Tag5::operator=(const Tag5 & copy){
    Key::operator=(copy);
    s2k_con = copy.s2k_con;
    sym = copy.sym;
    s2k = copy.s2k?copy.s2k -> clone():nullptr;
    IV = copy.IV;
    secret = copy.secret;
    return *this;
}
