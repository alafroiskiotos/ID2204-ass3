//
//  square.cpp
//  A1
//
//  Created by Lorenzo Corneo and Antonios Kouzoupis on 13/05/15.
//  Copyright (c) 2015 Lorenzo Corneo. All rights reserved.
//

#include <math.h>

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;
using namespace Gecode::Int;

// The no-overlap propagator
class NoOverlap : public Propagator {
protected:
    // The x-coordinates
    ViewArray<IntView> x;
    // The width (array)
    int* w;
    // The y-coordinates
    ViewArray<IntView> y;
    // The heights (array)
    int* h;
public:
    // Create propagator and initialize
    NoOverlap(Home home,
              ViewArray<IntView>& x0, int w0[],
              ViewArray<IntView>& y0, int h0[])
    : Propagator(home), x(x0), w(w0), y(y0), h(h0) {
        x.subscribe(home,*this,PC_INT_BND);
        y.subscribe(home,*this,PC_INT_BND);
    }
    // Post no-overlap propagator
    static ExecStatus post(Home home,
                           ViewArray<IntView>& x, int w[],
                           ViewArray<IntView>& y, int h[]) {
        // Only if there is something to propagate
        if (x.size() > 1)
            (void) new (home) NoOverlap(home,x,w,y,h);
        return ES_OK;
    }
    
    // Copy constructor during cloning
    NoOverlap(Space& home, bool share, NoOverlap& p)
    : Propagator(home,share,p) {
        x.update(home,share,p.x);
        y.update(home,share,p.y);
        // Also copy width and height arrays
        w = home.alloc<int>(x.size());
        h = home.alloc<int>(y.size());
        for (int i=x.size(); i--; ) {
            w[i]=p.w[i]; h[i]=p.h[i];
        }
    }
    // Create copy during cloning
    virtual Propagator* copy(Space& home, bool share) {
        return new (home) NoOverlap(home,share,*this);
    }
    
    // Return cost (defined as cheap quadratic)
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO,2*x.size());
    }
    
    // Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        // X coordinate constraint
        for(int i = 0; i < x.size() - 1; i++) {
            for(int j = 0; j < x.size(); j++) {
                // Left and right constraints.
                if(x[i].lq(home, x[j].min() - w[i]) == Int::ME_INT_FAILED ||
                   x[i].gq(home, x[j].max() + w[j]) == Int::ME_INT_FAILED) {
                    return ES_FAILED;
                }
            }
        }
        
        // Y coordinate constraint
        for(int i = 0; i < y.size() - 1; i++) {
            for(int j = 0; j < y.size(); j++) {
                // Below and above constraints.
                if(y[i].lq(home, y[j].min() - h[i]) == Int::ME_INT_FAILED ||
                   y[i].gq(home, y[j].max() + h[j]) == Int::ME_INT_FAILED) {
                    return ES_FAILED;
                }
            }
        }
        
        // Checking the assignment of the (x,y) coordinates
        for(int i = 0; i < x.size(); i++) {
            if(!x[i].assigned() || !y[i].assigned()) {
                return ES_NOFIX;
            }
        }
        
        // Eventually the fixpoint has reached
        return home.ES_SUBSUMED(*this);
    }
    
    // Dispose propagator and return its size
    virtual size_t dispose(Space& home) {
        x.cancel(home,*this,PC_INT_BND);
        y.cancel(home,*this,PC_INT_BND);
        (void) Propagator::dispose(home);
        return sizeof(*this);
    }
};

/*
 * Post the constraint that the rectangles defined by the coordinates
 * x and y and width w and height h do not overlap.
 *
 * This is the function that you will call from your model. The best
 * is to paste the entire file into your model.
 */
void nooverlap(Home home,
               const IntVarArgs& x, const IntArgs& w,
               const IntVarArgs& y, const IntArgs& h) {
    // Check whether the arguments make sense
    if ((x.size() != y.size()) || (x.size() != w.size()) ||
        (y.size() != h.size()))
        throw ArgumentSizeMismatch("nooverlap");
    // Never post a propagator in a failed space
    if (home.failed()) return;
    // Set up array of views for the coordinates
    ViewArray<IntView> vx(home,x);
    ViewArray<IntView> vy(home,y);
    // Set up arrays (allocated in home) for width and height and initialize
    int* wc = static_cast<Space&>(home).alloc<int>(x.size());
    int* hc = static_cast<Space&>(home).alloc<int>(y.size());
    for (int i=x.size(); i--; ) {
        wc[i]=w[i]; hc[i]=h[i];
    }
    // If posting failed, fail space
    if (NoOverlap::post(home,vx,wc,vy,hc) != ES_OK)
        home.fail();
}

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
    x(*this, n, 0, maxSourroundingSquareSize()), y(*this, n, 0, maxSourroundingSquareSize()),
    s(*this, maxSourroundingSquareSize(), sumN()) {
        
        // Every square must fit the surrounding square.
        for(int i = 0; i < n - 1; i++) {
            rel(*this, x[i] + size(i) <= s);
            rel(*this, y[i] + size(i) <= s);
        }
        
        // symmetry removal
        rel(*this, x[0], IRT_GQ, 0);
        rel(*this, x[0], IRT_LE, floor(((s.max() - n) / 2)));
        rel(*this, y[0], IRT_LQ, x[0]);
        
        // empty strip dominance
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
        
        //            for(int i = 0; i < n - 1; i++) {
        //                // The last element (1x1) is not considered.
        //                for(int j = i + 1; j < n; j++) {
        //                    BoolVarArgs b(*this, 4, 0, 1);
        //
        //                    // Left constraint.
        //                    rel(*this, LinIntExpr(x[i] + size(i)).post(*this, ICL_VAL), IRT_LQ,
        //                        LinIntExpr(x[j]).post(*this, ICL_VAL), b[0]);
        //
        //                    // Right constraint.
        //                    rel(*this, LinIntExpr(x[i]).post(*this, ICL_VAL), IRT_GQ,
        //                        LinIntExpr(x[j] + size(j)).post(*this, ICL_VAL), b[1]);
        //
        //                    // Below constraint.
        //                    rel(*this, LinIntExpr(y[i] + size(i)).post(*this, ICL_VAL), IRT_LQ,
        //                        LinIntExpr(y[j]).post(*this, ICL_VAL), b[2]);
        //
        //                    // Above constraint.
        //                    rel(*this, LinIntExpr(y[i]).post(*this, ICL_VAL), IRT_GQ,
        //                        LinIntExpr(y[j] + size(j)).post(*this, ICL_VAL), b[3]);
        //
        //                    linear(*this, b, IRT_GQ, 1);
        //                }
        //            }
        
        IntArgs w(n);
        IntArgs h(n);
        //            int w[n];
        //            int h[n];
        IntVarArgs x1(n);
        IntVarArgs y1(n);
        
        for(int i = 0; i < n; i++) {
            w[i] = size(i);
            h[i] = size(i);
        }
        
        nooverlap(*this, x, w, y, h, ICL_VAL);
        
        branch(*this, s, INT_VAL_MIN());
        branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
        branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    
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
    Square(bool share, Square& square, int n) : Script(share, square), n(n) {
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
