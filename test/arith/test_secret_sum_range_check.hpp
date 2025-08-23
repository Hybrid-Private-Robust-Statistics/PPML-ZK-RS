#ifndef EMP_ZK_RANGE_CHECK_SUM_H
#define EMP_ZK_RANGE_CHECK_SUM_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "utils.hpp"
#include "test_input_range.hpp"
#include "random_linear_combination.hpp"

using namespace emp;
using namespace std;




void secret_range_check(int64_t inp, IntFp zk_inp, int64_t B_low, int64_t B_high, vector<IntFp> &zk_zero_checking, IntFp &zk_l) {
    //assert(inp >= B_low);
    //assert(inp <= B_high);

	// changed in utils to handle negative B_low values
    int needed_bits_range_check = get_num_range_bits(B_low, B_high) + 1; // For the sign bit;

    IntFp tmp0, tmp1;

    tmp0 = zk_inp.negate();
    tmp0 = tmp0 + B_low;

    tmp1 = IntFp(B_high, PUBLIC);
    tmp1 = tmp1.negate();
    tmp1 = tmp1 + zk_inp;

    bool* bits = new bool[needed_bits_range_check * 2];

    // Mask for the target bit width (64 bits in this example)
    const int target_bit_width = needed_bits_range_check; // Adjust as necessary
    if (target_bit_width >= 64) {
        throw std::invalid_argument("target_bit_width must be less than 64 for int64_t");
    }
    const int64_t mask = (1LL << target_bit_width) - 1;


    // x - a -> changed to signed int
    int64_t delta_low = (inp - B_low) & mask;
    if (delta_low & (1LL << (target_bit_width - 1))) {
        delta_low -= (1LL << target_bit_width); // Handle sign extension
    }

    // b - x -> changed to signed int
    int64_t delta_high = (B_high - inp) & mask;
    if (delta_high & (1LL << (target_bit_width - 1))) {
        delta_high -= (1LL << target_bit_width); // Handle sign extension
    }

    // Extract bits of x-a, b-x in the clear
    for (int j = 0; j < needed_bits_range_check; j++) {
        bits[j] = delta_low & 1;
        delta_low >>= 1;

        bits[needed_bits_range_check + j] = delta_high & 1;
        delta_high >>= 1;
    }

    IntFp* zk_bits = new IntFp[needed_bits_range_check * 2];
    for (int i = 0; i < needed_bits_range_check * 2; i++) {
        zk_bits[i] = IntFp(bits[i], ALICE);
    }

    // Check if they are bits
    for (int i = 0; i < needed_bits_range_check * 2; i++) {
        IntFp tmp;
        tmp = zk_bits[i] * zk_bits[i];
        tmp = tmp + zk_bits[i].negate();

        zk_zero_checking.emplace_back(tmp);
    }

    // Check the bit decomposition for two's complement :done
    uint64_t cur = 1;
    for (int j = 0; j < needed_bits_range_check - 1; ++j) {
        tmp0 = tmp0 + zk_bits[j] * cur;
        tmp1 = tmp1 + zk_bits[needed_bits_range_check + j] * cur;
        cur <<= 1;
    }

    // handle MSB for two's complement : done
    tmp0 = tmp0 + (zk_bits[needed_bits_range_check - 1] * cur).negate();
    tmp1 = tmp1 + (zk_bits[2 * needed_bits_range_check - 1] * cur).negate();

    // update sum of sign bits indicating if this value passes the range check
    zk_l = zk_l + zk_bits[needed_bits_range_check - 1] + zk_bits[2 * needed_bits_range_check - 1];
    zk_zero_checking.emplace_back(tmp0);
    zk_zero_checking.emplace_back(tmp1);

    

    delete[] bits;
    delete[] zk_bits;
}


double test_secret_sum_range_check(BoolIO<NetIO> **ios, int party, uint64_t B_low, uint64_t B_high, long long num_records, int parties) {

    setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	vector<IntFp> zk_zero_checking;
	vector<int64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	//if(party == ALICE) {
	//	cout << "start to create fake data and input them, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	//}

	auto dataset = create_random_dataset(precomputed_dataset, zk_precomputed_dataset, num_records);
	
	int64_t l = num_records / 2;
	IntFp zk_l = IntFp(l, ALICE);
	zk_l = zk_l.negate();
    auto total_time_start = clock_start();
	for(int i = 0; i < dataset.size(); i++) {
		secret_range_check(dataset[i].inp, dataset[i].zk_inp, B_low, B_high, zk_zero_checking, zk_l);
	}
	zk_zero_checking.emplace_back(zk_l);
	//if(party == ALICE) {
		//cout << "after all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	//}

	// compute the random linear combination
	// temporarily, we set the challenges to be 7 and 8

	int64_t randlc_res_1;
	IntFp zk_randlc_res_1 = IntFp(0, PUBLIC);

	int64_t randlc_res_2;
	IntFp zk_randlc_res_2 = IntFp(0, PUBLIC);


	random_linear_combination_signed(precomputed_dataset, zk_precomputed_dataset, 7, randlc_res_1, zk_randlc_res_1);
	random_linear_combination_signed(precomputed_dataset, zk_precomputed_dataset, 8, randlc_res_2, zk_randlc_res_2);

	batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
    finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
    auto total_time = time_from(total_time_start);
    //cout << "prove [" << num_records << "] secret range checks" << endl;
    //cout << "time use for "<< parties << " parties:" << (total_time* parties*(parties-1)) /  CLOCKS_PER_SEC << " sec" << endl;
    double tt = total_time /  CLOCKS_PER_SEC;
    return (tt/  CLOCKS_PER_SEC) ;
	
}
#endif //EMP_ZK_RANGE_CHECK_SUM_H
