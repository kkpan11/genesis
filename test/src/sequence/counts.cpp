/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2016 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab, Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup test
 */

#include "common.hpp"

#include "lib/sequence/counts.hpp"
#include "lib/sequence/formats/phylip_reader.hpp"
#include "lib/sequence/functions/counts.hpp"
#include "lib/sequence/sequence_set.hpp"
#include "lib/sequence/sequence.hpp"

using namespace genesis;
using namespace genesis::sequence;

TEST( Sequence, Entropy )
{
    Sequence s_0 = { "", "", "AAAA" };
    Sequence s_1 = { "", "", "AAAC" };
    Sequence s_2 = { "", "", "AACG" };
    Sequence s_3 = { "", "", "ACGT" };

    auto counts = SequenceCounts( "ACGT", 4 );

    counts.add_sequence( s_0 );
    counts.add_sequence( s_1 );
    counts.add_sequence( s_2 );
    counts.add_sequence( s_3 );

    EXPECT_FLOAT_EQ( 0.0,       site_entropy(     counts, 0 ));
    EXPECT_FLOAT_EQ( 2.0,       site_information( counts, 0 ));
    EXPECT_FLOAT_EQ( 0.8112781, site_entropy(     counts, 1 ));
    EXPECT_FLOAT_EQ( 1.1887219, site_information( counts, 1 ));
    EXPECT_FLOAT_EQ( 1.5,       site_entropy(     counts, 2 ));
    EXPECT_FLOAT_EQ( 0.5,       site_information( counts, 2 ));
    EXPECT_FLOAT_EQ( 2.0,       site_entropy(     counts, 3 ));
    EXPECT_FLOAT_EQ( 0.0,       site_information( counts, 3 ));

    EXPECT_FLOAT_EQ( 4.3112783, absolute_entropy( counts ));
    EXPECT_FLOAT_EQ( 1.0778196, averaged_entropy( counts ));

    EXPECT_EQ( "AAAA", consensus_sequence_with_majorities( counts ));
}

TEST( Sequence, ConsensusMajority )
{
    // Skip test if no data availabe.
    NEEDS_TEST_DATA;

    // Load sequence file.
    std::string infile = environment->data_dir + "sequence/dna_5_42_s.phylip";
    SequenceSet sset;
    PhylipReader()
        .label_length( 10 )
        .from_file(infile, sset);

    // Create counts object.
    auto counts = SequenceCounts( "ACGT", 42 );
    counts.add_sequences( sset );

    // Correct sequence calculated with Seaview.
    EXPECT_EQ(
        "AAACCCTGGCCGTTCAGGGTAAACCGTGGCCGGGCAGGGTAT",
        consensus_sequence_with_majorities( counts )
    );
}

TEST( Sequence, ConsensusAmbiguity )
{
    // Skip test if no data availabe.
    NEEDS_TEST_DATA;

    // Load sequence file.
    std::string infile = environment->data_dir + "sequence/dna_5_42_s.phylip";
    SequenceSet sset;
    PhylipReader()
        .label_length( 10 )
        .from_file(infile, sset);

    // Create counts object.
    auto counts = SequenceCounts( "ACGT", 42 );
    counts.add_sequences( sset );

    // Manually calculated correct sequences.
    EXPECT_EQ(
        "AARCCYTGGCCGTTCAGGGTAAACCGTGGCCGGKCAGGGTAT",
        consensus_sequence_with_ambiguities( counts, 0.0 )
    );
    EXPECT_EQ(
        "AARCCYTGGCCGTTCAGGGTAAACCGTGGCCGGKCAGGGTAT",
        consensus_sequence_with_ambiguities( counts, 0.25 )
    );
    EXPECT_EQ(
        "AAVCCYTKGCMGTTMMGSKTRARCCNTGGCCGKDMMGSKTAW",
        consensus_sequence_with_ambiguities( counts, 0.5 )
    );
    EXPECT_EQ(
        "AMVSBYKKGCMKKKMMGSKTRMRSSNDKGCMRKDMMVSKYAW",
        consensus_sequence_with_ambiguities( counts, 1.0 )
    );

    // Some edge cases: zero sequences.
    auto counts_2 = SequenceCounts( "ACGT", 5 );
    EXPECT_EQ( "-----", consensus_sequence_with_ambiguities( counts_2, 0.0 ));
    EXPECT_EQ( "-----", consensus_sequence_with_ambiguities( counts_2, 1.0 ));

    // One sequence.
    counts_2.add_sequence( "-ACGT" );
    EXPECT_EQ( "-ACGT", consensus_sequence_with_ambiguities( counts_2, 0.0, true ));
    EXPECT_EQ( "-ACGT", consensus_sequence_with_ambiguities( counts_2, 1.0, true ));

    // More.
    counts_2.add_sequence( "-ACCT" );
    counts_2.add_sequence( "ACCT-" );
    EXPECT_EQ( "-ACBT", consensus_sequence_with_ambiguities( counts_2, 0.0, true ));
    EXPECT_EQ( "AMCBT", consensus_sequence_with_ambiguities( counts_2, 1.0, true ));
    EXPECT_EQ( "AACBT", consensus_sequence_with_ambiguities( counts_2, 0.0, false ));
    EXPECT_EQ( "AMCBT", consensus_sequence_with_ambiguities( counts_2, 1.0, false ));
}
