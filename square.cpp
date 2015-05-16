//
//  square.cpp
//
//  Created by Lorenzo Corneo and Antonios Kouzoupis on 13/05/15.
//

#include <math.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Square : public Script {
protected:
    // Number of squares to be packed.
    const int n;
    
    // Minimum value for thre surrounding square.
    IntVar s;
    
    // These two arrays represent the coordinates of the squares, accordingly to the relative axis.
    IntVarArray x, y;
public:
    Square(const SizeOptions& opt) : Script(opt), n(opt.size()),
    x(*this, opt.size(), 0, maxSourroundingSquareSize()), y(*this, opt.size(), 0, maxSourroundingSquareSize()),
    s(*this, maxSourroundingSquareSize(), sumN()) {
        
        // Every square must fit the surrounding square.
        for(int i = 0; i < n - 1; i++) {
            rel(*this, x[i] + size(i) <= s);
            rel(*this, y[i] + size(i) <= s);
        }
        
        // Symmetry removal.
        rel(*this, x[0], IRT_GQ, 0);
        rel(*this, x[0], IRT_LE, floor(((s.max() - n) / 2)));
        rel(*this, y[0], IRT_LQ, x[0]);
        
        // Empty strip dominance.
        if(n == 2) {
            rel(*this, x[0], IRT_LE, 2);
            rel(*this, y[0], IRT_LE, 2);
        } else if (n == 3) {
            rel(*this, x[0], IRT_LE, 3);
            rel(*this, y[0], IRT_LE, 3);
        } else if (n == 4) {
            rel(*this, x[0], IRT_LE, 2);
            rel(*this, y[0], IRT_LE, 2);
        } else if(n >= 5 && n <= 8) {
            rel(*this, x[0], IRT_LE, 3);
            rel(*this, y[0], IRT_LE, 3);
        } else if (n >= 9 && n <= 11) {
            rel(*this, x[0], IRT_LE, 4);
            rel(*this, y[0], IRT_LE, 4);
        } else if(n >= 12 && n <= 17) {
            rel(*this, x[0], IRT_LE, 5);
            rel(*this, y[0], IRT_LE, 5);
        } else if (n >= 18 && n <= 21) {
            rel(*this, x[0], IRT_LE, 6);
            rel(*this, y[0], IRT_LE, 6);
        } else if (n >= 22 && n <= 29) {
            rel(*this, x[0], IRT_LE, 7);
            rel(*this, y[0], IRT_LE, 7);
        } else if (n >= 30 && n <= 34) {
            rel(*this, x[0], IRT_LE, 8);
            rel(*this, y[0], IRT_LE, 8);
        } else if (n >= 35 && n <= 44) {
            rel(*this, x[0], IRT_LE, 9);
            rel(*this, y[0], IRT_LE, 9);
        } else if (n == 45) {
            rel(*this, x[0], IRT_LE, 10);
            rel(*this, y[0], IRT_LE, 10);
        }

    	// The last element (1x1) is not considered.
        for(int i = 0; i < n - 1; i++) {
        	for(int j = i + 1; j < n; j++) {
        		BoolVarArgs b(*this, 4, 0, 1);

        		// Left constraint.
        		rel(*this, LinIntExpr(x[i] + size(i)).post(*this, ICL_VAL), IRT_LQ,
        				LinIntExpr(x[j]).post(*this, ICL_VAL), b[0]);

        		// Right constraint.
        		rel(*this, LinIntExpr(x[i]).post(*this, ICL_VAL), IRT_GQ,
        				LinIntExpr(x[j] + size(j)).post(*this, ICL_VAL), b[1]);

        		// Below constraint.
        		rel(*this, LinIntExpr(y[i] + size(i)).post(*this, ICL_VAL), IRT_LQ,
        				LinIntExpr(y[j]).post(*this, ICL_VAL), b[2]);

        		// Above constraint.
        		rel(*this, LinIntExpr(y[i]).post(*this, ICL_VAL), IRT_GQ,
        				LinIntExpr(y[j] + size(j)).post(*this, ICL_VAL), b[3]);

                // At least one of the constraints expressed above must hold.
        		linear(*this, b, IRT_GQ, 1);
        	}
        }

        // The commented snippet below is the code used to test the implemented propagator no-overlap.

        // IntArgs w(n);
        // IntArgs h(n);
        // IntVarArgs x1(x);
        // IntVarArgs y1(y);
        
        // for(int i = 0; i < n; i++) {
        //     w[i] = size(i);
        //     h[i] = size(i);
        // }
        
        // nooverlap(*this, x1, w, y1, h);

        // Branching first on s.
        branch(*this, s, INT_VAL_MIN());
        branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
        branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    
    // Returns the sum of the length of all the N squares.
    int sumN() {
        int sum = 0;
        for (int i = 1; i < n + 1 ; i++) {
            sum += i;
        }
        return sum;
    }
    
    // Returns the dimension of the square given the index. Accordingly to the assignment text, the biggest
    // square is placed to the first index. (Biggest first)
    int size(int index) {
        return n - index;
    }
    
    // Returns the max size of the side of the surrounding square.
    int maxSourroundingSquareSize() {
        return ceil(sqrt(n * (n + 1) * (2 * n + 1) / 6));
    }
    
    /// Constructor for cloning \a s
    Square(bool share, Square& square, int size) : Script(share, square), n(size) {
        x.update(*this, share, square.x);
        y.update(*this, share, square.y);
        s.update(*this, share, square.s);
    }
    
    /// Perform copying during cloning
    virtual Space*
    copy(bool share) {
        return new Square(share,*this, n);
    }
    
    virtual void print(std::ostream& os) const {
        std::cout << "s -> " << s << std::endl;
        for(int i = 0; i < n; i++) {
            std::cout << "[" << x[i] << ", " << y[i] << "]" << std::endl;
        }
        
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[]) {
    SizeOptions opt("Square");
    opt.size(5);
    opt.parse(argc, argv);
    Script::run<Square,BAB,SizeOptions>(opt);
    return 0;
}
