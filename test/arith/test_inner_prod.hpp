
#ifndef EMP_ZK_INNER_PROD_H
#define EMP_ZK_INNER_PROD_H

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "utils.hpp"
using namespace emp;
using namespace std;

// zero-knowledge proof of [repeat] inner products between two vectors  of length [len].
double test_inner_product(BoolIO<NetIO> **ios, int party, long long repeat, long long len) {
    

    srand(time(NULL));
    uint64_t constant = 0;
    uint64_t *witness = new uint64_t[2 * len];
    memset(witness, 0, 2 * len * sizeof(uint64_t));

    setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

    IntFp *x = new IntFp[2 * len];

    if (party == ALICE) {
        uint64_t sum = 0, tmp;
        for (int i = 0; i < len; ++i) {
        witness[i] = rand() % PR;
        witness[len + i] = rand() % PR;
        }
        for (int i = 0; i < len; ++i) {
        tmp = mult_mod(witness[i], witness[len + i]);
        sum = add_mod(sum, tmp);
        }
        constant = PR - sum;
        ios[0]->send_data(&constant, sizeof(uint64_t));
    } else {
        ios[0]->recv_data(&constant, sizeof(uint64_t));
    }

    for (int i = 0; i < 2 * len; ++i)
        x[i] = IntFp(witness[i], ALICE);

    auto start = clock_start();
    for (int j = 0; j < repeat; ++j) {
        fp_zkp_inner_prdt<BoolIO<NetIO>>(x, x + len, constant, len);
    }

    finalize_zk_arith<BoolIO<NetIO>>();
    //finalize_zk_bool<BoolIO<NetIO>>();

    double tt = time_from(start);
    //cout << "prove [" << n << "," << d << "] matrix of " << n << " dots"  << endl;
    //cout << "time use for "<< m << " parties:" << (tt*  m*(m-1)) / 1000 << " ms" << endl;

    delete[] witness;
    delete[] x;
    return (tt/  CLOCKS_PER_SEC) ;
}

#endif 