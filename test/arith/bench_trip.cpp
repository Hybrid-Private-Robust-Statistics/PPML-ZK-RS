// party0 in AWS must be party 1 in execution!!

#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
#include "test_input_speed.hpp"
#include "test_input_range.hpp"
#include "test_secret_input_range.hpp"
#include "test_secret_sum_range_check.hpp"

#include "test_inner_prod.hpp"
#include "utils.hpp"

using namespace emp;
using namespace std;

int port, party;
//const int threads = 48;
uint64_t b_low = 1; 
uint64_t b_high = 10000;

double initial_input_commitment(BoolIO<NetIO> *ios[threads], int party, long long n, long long d, int m){
    double tt, tt1, tt2, tt3, tt4, tt5;
    long long input_size, input_sizeX, input_sizey, repeat, len;
    // 1: X : n x d 
    input_sizeX = n * d;
    input_sizey = n;
    tt1 = test_input_speed(ios, party, input_sizeX);
    tt2 = test_input_speed(ios, party, input_sizey);
    // 2: X_i : 1 x d , d dots
    repeat = n;
    len  = d;
    input_size = repeat * len * 2;
    tt3 = test_inner_product(ios, party, repeat, len);
    tt4 = test_input_speed(ios, party, input_size);
    // 3: |X_i| = 1, n elements
    repeat = n;
    tt5 = test_input_range(ios, party, b_low, b_high, repeat, m);
    
    // Total time for pairwise proofs between parties
    tt = (m-1)*(tt1 + tt2 + tt3 + tt4 + tt5);
    return tt;
}

double model_input_update(BoolIO<NetIO> *ios[threads], int party, long long n, long long d, int m){
    double tt, tt1, tt2, tt3, tt4, tt5, tt6, tt7, tt8, tt9;
    long long input_size, input_sizeX, input_sizey, repeat, len;
    
    // Case 1: matrix = d *  n , n dots
    repeat = n*d;
    len  = n;
    input_size = repeat * len * 2;
    tt1 = test_inner_product(ios, party, repeat, len);
    tt2 = test_input_speed(ios, party, input_size);

    // Case 2: size = d * d,  d dots

    repeat = d*d ;
    len  = d;
    input_size = repeat * len *2;
    tt3 = test_inner_product(ios, party, repeat, len);
    tt4 = test_input_speed(ios, party, input_size);
    int64_t b_low = 1; 
    int64_t b_high = 10000;

    // Case 3: size = d * 1, n dots
    repeat = d ;
    len  = n;
    input_size = repeat * len *2;
    tt5 = test_inner_product(ios, party, repeat, len );
    tt6  = test_input_speed(ios, party, input_size);

    // Case 4: size = d * d, n dots
    repeat = d*d ;
    len  = n;
    input_size = repeat * len *2;
    tt7 = test_inner_product(ios, party, repeat, len );
    tt8  = test_input_speed(ios, party, input_size);
    
    tt9 = test_input_range(ios, party, b_low, b_high, d*d, m);
    // Total time for pairwise proofs between parties
    tt = (m-1)*(tt1 + tt2 + tt3 + tt4 + tt5 + tt6 + tt7 + tt8 + tt9);
    return tt;
}

double participation_set_update(BoolIO<NetIO> *ios[threads], int party, long long n, long long d, int m){
    double tt, tt1, tt2, tt3;
    long long input_size, input_sizeX, input_sizey, repeat, len;
    tt1 = test_secret_input_range(ios, party, b_low, b_high, n, m);
    tt2 = test_input_speed(ios, party, n);
    tt3 = test_secret_sum_range_check(ios, party, b_low, b_high, n, m);
    tt = (m-1)*(tt1 + 2*tt2 + 2*tt3 );
    return tt;
}

int main(int argc, char **argv) {
    double input_commitment_time = 0;
    double input_update_time = 0;
    double participation_update_time = 0;
    long long repeat, len;
    long long n, d;
    long long input_size;
    int m;
    parse_party_and_port(argv, &party, &port);
    BoolIO<NetIO> *ios[threads];
    for (int i = 0; i < threads; ++i)
        ios[i] = new BoolIO<NetIO>(
            new NetIO(party == ALICE ? nullptr : "172.31.48.245", port + i),
            party == ALICE);

    std::cout << std::endl << "------------ ";
    std::cout << "ZKP inner product test";
    std::cout << " ------------" << std::endl << std::endl;
    ;

    if (argc < 3) {
        std::cout << "usage: [binary] PARTY PORT N D M"
                << std::endl;
        return -1;
    } else if (argc < 5) {
        n = 100;
        d = 10;
        m = 2;
    } else {
        n = atoll(argv[3]);
        d = atoll(argv[4]);
        m = atoi(argv[5]);
        n = n / m;
    }
    //setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);
    //setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);

    input_commitment_time = initial_input_commitment(ios, party, n, d, m);
    cout << "Input Commitment Time (s): " << std::fixed << std::setprecision(6) << input_commitment_time << endl;

    input_update_time = model_input_update(ios, party, n, d, m);
    cout << "Input Update Time (s): " << std::fixed << std::setprecision(6) << input_update_time << endl;
    
    participation_update_time = participation_set_update(ios, party, n, d, m);
    cout << "Participation Set Update Time (s): " << std::fixed << std::setprecision(6) << participation_update_time << endl;
    
    return 0;
}
