#include "serializer/log/lba/disk_format.hpp"
#include "serializer/log/log_serializer.hpp"

#include "unittest/gtest.hpp"

namespace unittest {

TEST(DiskFormatTest, FlaggedOff64T) {
    off64_t offs[] = { 0, 1, 4095, 4096, 4097, 234234 * 4096, 12345678901234567LL };

    for (size_t i = 0; i < sizeof(offs) / sizeof(*offs); ++i) {
        off64_t off = offs[i];
        flagged_off64_t real = flagged_off64_t::unused();
        real.set_value(off);
        flagged_off64_t deleteblock = flagged_off64_t::unused();
        deleteblock.set_value(off);
        EXPECT_TRUE(real.has_value());
        EXPECT_TRUE(deleteblock.has_value());
        EXPECT_FALSE(real.is_padding());
        EXPECT_FALSE(deleteblock.is_padding());

        real.set_value(73);
        deleteblock.set_value(95);

        EXPECT_TRUE(real.has_value());
        EXPECT_TRUE(deleteblock.has_value());
        EXPECT_FALSE(real.is_padding());
        EXPECT_FALSE(deleteblock.is_padding());
    }


    EXPECT_FALSE(flagged_off64_t::unused().has_value());
    EXPECT_FALSE(flagged_off64_t::padding().has_value());

    EXPECT_EQ(8, sizeof(flagged_off64_t));
}

TEST(DiskFormatTest, LbaShardMetablockT) {
    EXPECT_EQ(0, offsetof(lba_shard_metablock_t, last_lba_extent_offset));
    EXPECT_EQ(8, offsetof(lba_shard_metablock_t, last_lba_extent_entries_count));
    EXPECT_EQ(16, offsetof(lba_shard_metablock_t, lba_superblock_offset));
    EXPECT_EQ(24, offsetof(lba_shard_metablock_t, lba_superblock_entries_count));
    EXPECT_EQ(32, sizeof(lba_shard_metablock_t));
}

TEST(DiskFormatTest, LbaMetablockMixinT) {
    // This test will clearly fail if it's not 16.
    EXPECT_EQ(16, LBA_SHARD_FACTOR);
    EXPECT_EQ(512, sizeof(lba_metablock_mixin_t));
}

TEST(DiskFormatTest, LbaEntryT) {
    EXPECT_EQ(0, offsetof(lba_entry_t, block_id));
    EXPECT_EQ(4, offsetof(lba_entry_t, recency));
    EXPECT_EQ(8, offsetof(lba_entry_t, offset));
    EXPECT_EQ(16, sizeof(lba_entry_t));

    lba_entry_t ent = lba_entry_t::make_padding_entry();
    ASSERT_TRUE(lba_entry_t::is_padding(&ent));
    flagged_off64_t real = flagged_off64_t::unused();
    real.set_value(1);
    ent = lba_entry_t::make(1, repli_timestamp_t::invalid, real);
    ASSERT_FALSE(lba_entry_t::is_padding(&ent));
    flagged_off64_t deleteblock = flagged_off64_t::unused();
    deleteblock.set_value(1);
    ent = lba_entry_t::make(1, repli_timestamp_t::invalid, deleteblock);
    ASSERT_FALSE(lba_entry_t::is_padding(&ent));
}

TEST(DiskFormatTest, LbaExtentT) {
    EXPECT_EQ(16, sizeof(lba_extent_t::header_t));

    // (You might want to update this test if LBA_SUPER_MAGIC_SIZE changes.)
    EXPECT_EQ(8, LBA_SUPER_MAGIC_SIZE);
    EXPECT_EQ(0, offsetof(lba_extent_t, header));
    EXPECT_EQ(8, offsetof(lba_extent_t, header.padding));
    EXPECT_EQ(16, offsetof(lba_extent_t, entries));
    EXPECT_EQ(16, sizeof(lba_extent_t));
}

TEST(DiskFormatTest, LbaSuperblockT) {
    EXPECT_EQ(0, offsetof(lba_superblock_entry_t, offset));
    EXPECT_EQ(8, offsetof(lba_superblock_entry_t, lba_entries_count));
    EXPECT_EQ(16, sizeof(lba_superblock_entry_t));

    EXPECT_EQ(0, offsetof(lba_superblock_t, magic));
    EXPECT_EQ(8, offsetof(lba_superblock_t, padding));
    EXPECT_EQ(16, offsetof(lba_superblock_t, entries));
}

TEST(DiskFormatTest, DataBlockManagerMetablockMixinT) {
    // The numbers below assume this fact about MAX_ACTIVE_DATA_EXTENTS.
    EXPECT_EQ(64, MAX_ACTIVE_DATA_EXTENTS);

    EXPECT_EQ(0, offsetof(data_block_manager_t::metablock_mixin_t, active_extents));
    EXPECT_EQ(512, offsetof(data_block_manager_t::metablock_mixin_t, blocks_in_active_extent));
    EXPECT_EQ(1024, sizeof(data_block_manager_t::metablock_mixin_t));
}

TEST(DiskFormatTest, ExtentManagerMetablockMixinT) {
    EXPECT_EQ(0, offsetof(extent_manager_t::metablock_mixin_t, debug_extents_in_use));
    EXPECT_EQ(8, sizeof(extent_manager_t::metablock_mixin_t));
}

TEST(DiskFormatTest, LogSerializerMetablockT) {
    size_t n = 0;
    EXPECT_EQ(n, offsetof(log_serializer_metablock_t, extent_manager_part));

    n += sizeof(extent_manager_t::metablock_mixin_t);
    EXPECT_EQ(n, offsetof(log_serializer_metablock_t, lba_index_part));

    n += sizeof(lba_index_t::metablock_mixin_t);
    EXPECT_EQ(n, offsetof(log_serializer_metablock_t, data_block_manager_part));

    n += sizeof(data_block_manager_t::metablock_mixin_t);
    EXPECT_EQ(n, offsetof(log_serializer_metablock_t, block_sequence_id));

    EXPECT_EQ(8, sizeof(block_sequence_id_t));
    n += sizeof(block_sequence_id_t);
    EXPECT_EQ(n, sizeof(log_serializer_metablock_t));

    EXPECT_EQ(1552, 8 + 512 + 1024 + 8);
    EXPECT_EQ(1552, sizeof(log_serializer_metablock_t));
}

TEST(DiskFormatTest, LogSerializerStaticConfigT) {
    EXPECT_EQ(0, offsetof(log_serializer_on_disk_static_config_t, block_size_));
    EXPECT_EQ(8, offsetof(log_serializer_on_disk_static_config_t, extent_size_));
    EXPECT_EQ(16, sizeof(log_serializer_on_disk_static_config_t));
}

}  // namespace unittest
