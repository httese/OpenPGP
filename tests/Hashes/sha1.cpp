#include <gtest/gtest.h>

#include "Hashes/Hashes.h"

#include "testvectors/sha/sha1shortmsg.h"

TEST(SHA1, short_msg) {

    ASSERT_EQ(SHA1_SHORT_MSG.size(), SHA1_SHORT_MSG_HEXDIGEST.size());

    for ( unsigned int i = 0; i < SHA1_SHORT_MSG.size(); ++i ) {
        auto sha1 = OpenPGP::Hash::use(OpenPGP::Hash::ID::SHA1, unhexlify(SHA1_SHORT_MSG[i]));
        EXPECT_EQ(hexlify(sha1), SHA1_SHORT_MSG_HEXDIGEST[i]);
    }
}
