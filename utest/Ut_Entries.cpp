#include "gtest/gtest.h"
#include "Entries.h"
using namespace raft;

Entries entries;

TEST(Entries, case1) {
    entries._entries = "it is a test.";
    entries._index = 21451;
    entries._term = 20;
    EntriesRef ref(entries);
    EXPECT_EQ(21451, ref.GetIndex());
}

TEST(Entries, case2) {
    EntriesRef ref(entries);
    EXPECT_EQ(20, ref.GetTerm());
}

TEST(Entries, case3) {
    EntriesRef ref(entries);
    std::string str = ref.GetEntriesContent();
    EXPECT_STREQ("it is a test.", str.c_str());
}

TEST(Entries, case4) {
    EntriesRef ref(entries);
    std::string data = ref.GetData();
    EntriesRef ref_new((char*)data.c_str(), data.length());
    EXPECT_EQ(21451, ref_new.GetIndex());
}

TEST(Entries, case5) {
    EntriesRef ref(entries);
    std::string data = ref.GetData();
    EntriesRef ref_new((char*)data.c_str(), data.length());
    EXPECT_EQ(20, ref_new.GetTerm());
}

TEST(Entries, case6) {
    EntriesRef ref(entries);

    std::string data = ref.GetData();
    EntriesRef ref_new((char*)data.c_str(), data.length());

    std::string str = ref_new.GetEntriesContent();
    EXPECT_STREQ("it is a test.", str.c_str());
}

TEST(Entries, case7) {
    char str[] = "it is a test.";
    EntriesRef ref((uint32_t)25, (uint32_t)1000, str, sizeof(str));

    Entries entries = ref.GetEntries(); 
    EXPECT_EQ(25, entries._term);
}

TEST(Entries, case8) {
    char str[] = "it is a test.";
    EntriesRef ref((uint32_t)25, (uint32_t)1000, str, sizeof(str));

    Entries entries = ref.GetEntries();
    EXPECT_EQ(1000, entries._index);
}

TEST(Entries, case9) {
    char str[] = "it is a test.";
    EntriesRef ref((uint32_t)25, (uint32_t)1000, str, sizeof(str));

    Entries entries = ref.GetEntries();
    EXPECT_STREQ("it is a test.", entries._entries.c_str());
}