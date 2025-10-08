
// #include "voidops.hpp"
#include "../paulicpp/pauliarray/src/sparsepaulicpp.hpp"

using namespace std;

// std::vector<uint8_t> z_vec = {1,0,1,0};
// std::vector<uint8_t> x_vec = {0,1,1,0};

// std::vector<uint8_t> z_vec2 = {1,1,0,0,1,0,0,0,1,1,1,0};
// std::vector<uint8_t> x_vec2 = {0,0,1,1,0,1,0,0,1,0,1,0};

// std::vector<uint8_t> z_vec3 = {1,1,1,1};
// std::vector<uint8_t> x_vec3 = {0,0,0,0,1,1,0,1};

// std::vector<dpoint> z1 = make_dpoint(z_vec);
// std::vector<dpoint> x1 = make_dpoint(x_vec);
// std::vector<dpoint> z2 = make_dpoint(z_vec2);
// std::vector<dpoint> x2 = make_dpoint(x_vec2);

#define RANDOM_SIZE 10000000
#define RANDOM_DENSITY 0.1
#define RANDOM_PROXIMITY 0.99



int main() {
    cout << "========== SparsePauliArray ==========" << endl;
    cout << "Generating random vectors of size " << RANDOM_SIZE << " with density " << RANDOM_DENSITY << " and proximity " << RANDOM_PROXIMITY << "...\n";
    std::vector<uint8_t> z = generate_random_vec(RANDOM_SIZE, RANDOM_DENSITY, RANDOM_PROXIMITY);
    std::vector<uint8_t> x = generate_random_vec(RANDOM_SIZE, RANDOM_DENSITY, RANDOM_PROXIMITY);
    cout << "Done.\n\n";
    // SparsePauliArray spa1(z, x, RANDOM_SIZE);
    // spa1.show();
    vector<dpoint> z_pts = make_dpoint(z);
    vector<dpoint> x_pts = make_dpoint(x);
    // show_dpoints(z_pts);
    // show_dpoints(x_pts);
    // cout << "Z popcount: " <<  << endl;
    // overlap_dpoint(z_pts, x_pts);

    vector<dpoint> xor_pts = xor_dpoint(z_pts, x_pts);
    // vector<dpoint> and_pts = and_dpoint(z_pts, x_pts);
    // vector<dpoint> or_pts = or_dpoint(z_pts, x_pts);
    // vector<dpoint> not_z_pts = not_dpoint(z_pts, RANDOM_SIZE);

    // cout << "\n\n----- RESULTS -----" << endl;
    // cout << "XOR dpoints:" << endl;
    // show_dpoints(xor_pts);
    // cout << "\nAND dpoints:" << endl;
    // show_dpoints(and_pts);
    // cout << "\nNOT dpoints (Z):" << endl;
    // show_dpoints(not_z_pts);
    // cout << "\nOR dpoints:" << endl;
    // show_dpoints(or_pts);


    // cout << "Z popcount: " << spa1.get_z_popcount() << endl;
    // cout << "X popcount: " << spa1.get_x_popcount() << endl;

    return 0;
}