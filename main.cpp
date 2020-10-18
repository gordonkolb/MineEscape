
//  Created by gordie2 on 9/10/20.
//  Copyright © 2020 Gordon Kolb. All rights reserved.
//

#include <iostream>
#include "xcode_redirect_p2.hpp"
#include "Mine.hpp"

int main(int argc, char * argv[]) {
    std::ios_base::sync_with_stdio(false);
    xcode_redirect(argc, argv);
    cout << std::fixed << std::setprecision(2);
    Mine test;
    test.getOptions(argc, argv);
    test.read_mine();
    test.escape();
    test.print_stats();

}

