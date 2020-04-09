#include "gtest/gtest.h"
#include "Entries.h"
#include "CommitEntriesDisk.h"

using namespace raft;

CCommitEntriesDisk commit_disk("E:/file.cache");

TEST(CommitEntriesDisk, case1) {
    Entries entries;
    entries._entries = "it is a test 1.";
    entries._index = 1;
    entries._term = 1;
    EXPECT_TRUE(commit_disk.PushEntries(entries));
}

TEST(CommitEntriesDisk, case2) {
    EXPECT_TRUE(commit_disk.PushEntries(2, 2, "it is a test 2."));
}

TEST(CommitEntriesDisk, case3) {
    EXPECT_TRUE(commit_disk.PushEntries(1, 2, "it is a test 3."));
}

TEST(CommitEntriesDisk, case3) {
    std::vector<Entries> entries_vec;
    EXPECT_TRUE(commit_disk.GetEntries(0, entries_vec));
    EXPECT_TRUE(entries_vec.size() == 2);
    EXPECT_TRUE(entries_vec[1]._entries == std::string("it is a test 3."));
}

TEST(CommitEntriesDisk, case4) {
    EXPECT_EQ(2, commit_disk.GetNewestIndex());
}