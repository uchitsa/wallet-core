// Copyright © 2017-2020 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Ethereum/Address.h"
#include "Ethereum/RLP.h"
#include "Ethereum/Signer.h"
#include "HexCoding.h"

#include <gtest/gtest.h>

namespace TW::Ethereum {

using boost::multiprecision::uint256_t;

class SignerExposed : public Signer {
public:
    SignerExposed(boost::multiprecision::uint256_t chainID) : Signer(chainID) {}
};

TEST(EthereumTransaction, encodeTransactionLegacy) {
    auto transaction = TransactionLegacy::buildERC20Transfer(
        /* nonce: */ 0,
        /* gasPrice: */ 42000000000, // 0x09c7652400
        /* gasLimit: */ 78009, // 130B9
        /* tokenContract: */ parse_hex("0x6b175474e89094c44da98b954eedeac495271d0f"), // DAI
        /* toAddress: */ parse_hex("0x5322b34c88ed0691971bf52a7047448f0f4efc84"),
        /* amount: */ 2000000000000000000
    );
    uint256_t dummyChain = 0x34;
    auto dummySignature = SignatureRSV{0, 0, 0};

    auto hash = transaction.hash(dummyChain);
    EXPECT_EQ(hex(hash), "b3525019dc367d3ecac48905f9a95ff3550c25a24823db765f92cae2dec7ebfd");
    
    auto encoded = transaction.encoded(dummySignature);
    EXPECT_EQ(hex(encoded), "f86a808509c7652400830130b9946b175474e89094c44da98b954eedeac495271d0f80b844a9059cbb0000000000000000000000005322b34c88ed0691971bf52a7047448f0f4efc840000000000000000000000000000000000000000000000001bc16d674ec80000808080");
}

TEST(EthereumTransaction, encodeTransactionAccessList) {
    auto transaction = TransactionAccessList(
        /* nonce: */ 0,
        /* gasPrice: */ 42000000000, // 0x09c7652400
        /* gasLimit: */ 78009, // 130B9
        /* toAddress: */ parse_hex("0x5322b34c88ed0691971bf52a7047448f0f4efc84"),
        /* value: */ 2000000000000000000,
        /* data */ parse_hex("01"),
        /* accessList */ parse_hex("02")
    );
    uint256_t dummyChain = 0x34;
    auto dummySignature = SignatureRSV{0, 0, 0};

    auto hash = transaction.hash(dummyChain);
    EXPECT_EQ(hex(hash), "2d4a5679255a62b61c7d1725e1fe1aaa6df6489c732548cc56f9953263edf75c");

    auto encoded = transaction.encoded(dummySignature);
    EXPECT_EQ(hex(encoded), "ef01808509c7652400830130b9945322b34c88ed0691971bf52a7047448f0f4efc84881bc16d674ec800000102808080");
}

TEST(EthereumSigner, Hash) {
    auto address = parse_hex("0x3535353535353535353535353535353535353535");
    auto transaction = TransactionLegacy::buildNativeTransfer(
        /* nonce: */ 9,
        /* gasPrice: */ 20000000000,
        /* gasLimit: */ 21000,
        /* to: */ address,
        /* amount: */ 1000000000000000000
    );
    auto signer = SignerExposed(1);
    //auto hash = signer.hash(transaction, signer.chainID);
    auto hash = transaction.hash(signer.chainID);
    
    ASSERT_EQ(hex(hash), "daf5a779ae972f972197303d7b574746c7ef83eadac0f2791ad23db92e4c8e53");
}

TEST(EthereumSigner, Sign) {
    auto address = parse_hex("0x3535353535353535353535353535353535353535");
    auto transaction = TransactionLegacy::buildNativeTransfer(
        /* nonce: */ 9,
        /* gasPrice: */ 20000000000,
        /* gasLimit: */ 21000,
        /* to: */ address,
        /* amount: */ 1000000000000000000
    );

    auto key = PrivateKey(parse_hex("0x4646464646464646464646464646464646464646464646464646464646464646"));
    auto signer = SignerExposed(1);
    auto signature = signer.sign(key, transaction);

    ASSERT_EQ(signature.v, 37);
    ASSERT_EQ(signature.r, uint256_t("18515461264373351373200002665853028612451056578545711640558177340181847433846"));
    ASSERT_EQ(signature.s, uint256_t("46948507304638947509940763649030358759909902576025900602547168820602576006531"));
}

TEST(EthereumSigner, SignERC20Transfer) {
    auto transaction = TransactionLegacy::buildERC20Transfer(
        /* nonce: */ 0,
        /* gasPrice: */ 42000000000, // 0x09c7652400
        /* gasLimit: */ 78009, // 130B9
        /* tokenContract: */ parse_hex("0x6b175474e89094c44da98b954eedeac495271d0f"), // DAI
        /* toAddress: */ parse_hex("0x5322b34c88ed0691971bf52a7047448f0f4efc84"),
        /* amount: */ 2000000000000000000
    );

    auto key = PrivateKey(parse_hex("0x608dcb1742bb3fb7aec002074e3420e4fab7d00cced79ccdac53ed5b27138151"));
    auto signer = SignerExposed(1);
    auto signature = signer.sign(key, transaction);

    ASSERT_EQ(signature.v, 37);
    ASSERT_EQ(hex(TW::store(signature.r)), "724c62ad4fbf47346b02de06e603e013f26f26b56fdc0be7ba3d6273401d98ce");
    ASSERT_EQ(hex(TW::store(signature.s)), "032131cae15da7ddcda66963e8bef51ca0d9962bfef0547d3f02597a4a58c931");
}

} // namespace TW::Ethereum
