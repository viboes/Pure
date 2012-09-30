
#include "../Pure.h"

using namespace pure;
using namespace list;

#include <iostream>
#include <fstream>
#include <sstream>

using std::cout;
using std::endl;
using std::flush;

template< class X >
std::ostream& operator<< ( std::ostream& os, const std::vector<X>& v ) {
    os << "{ ";
    std::copy( begin(v), end(v), std::ostream_iterator<X>(os," ") );
    os << "}";
    return os;
}

template< class X >
std::ostream& operator<< ( std::ostream& os, 
                           const std::vector<std::vector<X>>& vv ) 
{
    os << "{\n";
    for( auto& v : vv ) 
        os << '\t' << v << '\n';
    os << "}";
    return os;
}

template< class X, size_t N >
std::ostream& operator<< ( std::ostream& os, const std::array<X,N>& v ) {
    os << "< ";
    std::copy( begin(v), end(v), std::ostream_iterator<X>(os," ") );
    os << ">";
    return os;
}

struct ToInt {
    constexpr int operator() ( char c ) {
        return c - '0';
    }

    unsigned long long operator() ( const std::string& s ) const {
        // Be lazy, use the std.
        std::istringstream is( s );
        unsigned long long x;
        is >> x;
        return x;
    }

    template< class X >
    constexpr unsigned long long from( const X& x ) {
        return x;
    }

    template< class X >
    constexpr unsigned long long operator() ( const X& x ) {
        return from(x);
    }
} toInt;

vector<int> multiples_less_than_1000( int x ) {
    return take (
        999 / x, // x goes into 1000 (999/x) times.
        iterate( pure::plus(x), x )
    );
}

void problem1() {
    cout << "The sum of every multiple of 3 or 5 between 1 and 1000: "
         << flush <<
         sum( sunion( multiples_less_than_1000(3), 
                      multiples_less_than_1000(5) ) )
         << endl;
}

constexpr auto even = fnot( rcloset( Mod(), 2 ) );

constexpr unsigned long long operator "" _K ( unsigned long long x ) {
    return x * 1000;
}
constexpr unsigned long long operator "" _M ( unsigned long long x ) {
    return x * 1000000;
}

void problem2() {
    cout << "The sum of every even Fibonacci number below 4-million: "
         << flush << 
         ( sum ^ filter(even) ^ takeWhile(lessThan(4_M)) ) (
             biIterate( Add(), 1u, 2u )
         ) << endl;
}

using PrimeType = unsigned long long int;

PrimeType next_prime( const vector<PrimeType>& past ) {
    PrimeType x = last( past );
    do x += 2;
    while( any( divisorOf(x), reverse_wrap(tail_wrap(2,past)) ) );
    return x;
}

auto primes = memorize( next_prime, 1ull, 2ull, 3ull, 5ull, 7ull, 11ull );

void problem3() {
    const long int START = 600851475143;
    long int x = START;

    cout << "The largest prime divisor of " << START << flush;

    auto p = next(begin( primes ));

    while( *p < std::sqrt(x) )
        if( x % *p++ == 0 )
            x /= *prev(p);

    cout << " is " << x << endl;
}

#include <cmath>

using Digits = std::vector<unsigned int>;

struct DigitsT {
    Digits operator() ( unsigned int x ) {
        // TODO: Perhaps this would be a good test case for implementing mapAccum.
        Digits ds;
        for( ; x > 0; x = x / 10 )
            ds.push_back( x % 10 );
        return reverse(ds);
    }

    Digits operator() ( const std::string& s ) {
        return mapExactly<vector<unsigned int>>( toInt, s );
    }
} digits;

Digits operator* ( Digits ds, int x ) {
    unsigned int carry = 0;
    ds = map (
        [&]( unsigned int d ) {
            auto r = d * x + carry;
            carry = r / 10;
            return r % 10;
        },
        reverse(move(ds))
    );

    return reverse( append( move(ds), digits(carry) ) );
}

Digits operator* ( int x, Digits ds ) {
    return move(ds) * x;
}

Digits operator+ ( Digits ds, unsigned long long x ) {
    for( size_t i = 0; i < length(ds) and x>0; i++ ) {
        auto& d = last( ds, i );
        auto r = d + x;
        d = r % 10;
        x = r / 10;
    }

    return x ? append( reverse(digits(x)), ds )
        : ds;
}

bool _palindrome( const vector<unsigned int>& v ) {
    return equal( v, reverse_wrap(v) );
}

bool palindrome( unsigned int x ) {
    return _palindrome( digits(x) );
}

template< class R, class S >
R maybeMaximum( R r, const S& s ) {
    return null(s) ? r : std::max( r, maximum(s) );
}

#include <limits>
template< class X > 
struct Largest {
    X x;

    constexpr Largest( X y ) : x(move(y)) { }
    constexpr Largest() : x(0) { }

    operator X () { return x; }

    X get() { return x; }
};

template< class X >
Largest<X> returnLargest( X x ) {
    return x;
}

namespace pure {
    template< class X > struct Monoid< Largest<X> > {
        template< class Larg >
        static constexpr Larg mempty() { return Larg(); }

        static constexpr X mappend( const X& a, const X& b ) {
            return a > b ? a : b;
        }
    };
}

void problem4() {
    cout << "The largest palindrome product of three digit numbers :"
         << flush;
    cout << foldMap (
            []( const IRange& r ) -> Largest<int> {
                auto ps = filter( palindrome, 
                                  map( times(last(r)), init(r) ) );
                return notNull(ps) ? maximum(ps) : 0;
            },
            // We remove the first three values: {} {100}, and {100,101}.
            drop( 3, inits(enumerate(100,999)) ) 
    ) << endl;
}

int gcm( int x, int y ) {
    while(true) {
        if( not x ) return y;
        y %= x;
        if( not y ) return x;
        x %= y;
    }
}

int lcm( int x, int y ) {
    return x * y / gcm(x,y);
}

void problem5() {
    cout << pure::foldl( lcm, enumerate(2,19) ) 
         << " is divisible by all numbers 1 thought 20." << endl;
}

void problem6() {
    cout << "The difference between the sum squared and squared sum "
            "of each number between 1 and 100: " << flush;

    constexpr auto N = enumerate( 1, 100 );

    unsigned int sqrOfSum = sum(N) * sum(N);

    using P = float(*)(float,float);
    cout << sqrOfSum - (unsigned int)sum( rclosure(P(pow),2) ^ N ) << endl;
}

void problem7() {
    cout << "The 1001'st prime number: " << flush;
    cout << *next( begin(primes), 10001 ) << endl;
}

int from_sym( char sym ) { return sym - '0'; }

void problem8() {
    cout << "The largest sum of five numbers in the given sequence: " << flush;

    const string nsStr = "7316717653133062491922511967442657474235534919493496983520312774506326239578318016984801869478851843858615607891129494954595017379583319528532088055111254069874715852386305071569329096329522744304355766896648950445244523161731856403098711121722383113622298934233803081353362766142828064444866452387493035890729629049156044077239071381051585930796086670172427121883998797908792274921901699720888093776657273330010533678812202354218097512545405947522435258490771167055601360483958644670632441572215539753697817977846174064955149290862569321978468622482839722413756570560574902614079729686524145351004748216637048440319989000889524345065854122758866688116427171479924442928230863465674813919123162824586178664583591245665294765456828489128831426076900422421902267105562632111110937054421750694165896040807198403850962455444362981230987879927244284909188845801561660979191338754992005240636899125607176060588611646710940507754100225698315520005593572972571636269561882670428252483600823257530420752963450";

    cout << foldMap ( 
        []( const vector<int>& v ) -> Largest<int> {
            return product( take(5,v) ); 
        },
        tails( mapTo<vector>(from_sym,nsStr) )
    ) << endl;
}

constexpr bool triplet( int a, int b, int c ) {
    return a < b and b < c and a*a + b*b == c*c;
}

void problem9() {
    cout << "The triplet, (" << flush;

    unsigned int a = 4, b = 5;
    auto c = [&]() { return std::hypot(a,b); };

    while( a+b+c() != 1000 ) {
        a++;
        for( b = a+1; a + b + c() < 1000; b++ ) { 
        }
    }

    cout << a << ',' << b << ',' << c()
         << "), equals " << a+b+c() << " when added and " 
         << int(a*b*c()) << " when multiplied. " << endl;
}

bool _notDivisibleByOdd( PrimeType odd, PrimeType x ) {
    for( ; odd <= std::sqrt(x); odd += 2 )
        if( x % odd == 0 )
            return false;
    return true;
}

bool notDivisibleByOdd( PrimeType x ) {
    return _notDivisibleByOdd( 3, x );
}

void problem10() {
    cout << "The sum of all primes below 2 million is: " << flush;
    PrimeType ds = 2;
    for( PrimeType x=3; x < 2000000; x+=2 ) {
        // Using the memorized "primes" is too slow for large primes. 
        if( notDivisibleByOdd(x) )
            ds = ds + x;
    }
    cout << ds << endl;
}

using Row = vector<unsigned int>;
using Mat = vector<Row>;

using Vec = std::array<int,2>;
const int& get_x( const Vec& v ) { return get<0>(v); }
const int& get_y( const Vec& v ) { return get<1>(v); }
int& get_x( Vec& v ) { return get<0>(v); }
int& get_y( Vec& v ) { return get<1>(v); }

Vec operator+ ( const Vec& a, const Vec& b ) {
    return zipWith( Add(), a, b );
}

Vec operator- ( const Vec& a, const Vec& b ) {
    return zipWith( Subtract(), a, b );
}

Vec operator* ( Vec a, int x ) {
    return map( pure::plus(x), move(a) );
}

#include <cstdio>
Vec operator "" _v ( const char* const str, 
                                              size_t ) 
{
    Vec v;
    sscanf( str, "%dx%d", &get_x(v), &get_y(v) );
    return v;
}

unsigned int take_dir_prod( Vec dir, Vec pos, size_t n, const Mat& m ) {
    unsigned int prod = 1;

    Vec end = pos + dir * n;
    if(    get_y(end) < 0 or get_y(end) >= (int)length(m) 
        or get_x(end) < 0 or get_x(end) >= (int)length(m[0]) )
        return 1;

    for( ; n--; pos = pos+dir ) 
        prod *= m[get_y(pos)][get_x(pos)];

    return prod;
}

struct Line {
    std::string ln;
    operator const std::string& () { return ln; }
};

std::istream& operator>> ( std::istream& is, Line& l ) {
    return std::getline( is, l.ln );
}

void problem11() {
    std::ifstream fin( "e11" );

    cout << "The largest product is: " << flush;

    Mat mat = mapExactly<Mat> ( 
        []( const Line& l ) { return mapExactly<Row>( toInt, l.ln ); },
        io::fileContents<Line>( fin ) 
    );


    cout << foldMap ( 
        [&](int i, int j, const Vec& dir) -> Largest<unsigned int> { 
            return take_dir_prod( dir, {{i,j}}, 4, mat ); 
        }, 
        enumerate(mat), enumerate(mat[0]),  
        std::initializer_list<Vec>{ "-1x0"_v, " 1x1"_v,
                                    " 0x1"_v, "-1x1"_v }
    ) << endl;
}

using Factor = PrimeType;
using Factors = std::vector<PrimeType>;

bool isPrime( Factor x ) {
    return elem( x, takeWhile( lessThan(x), primes ) );
}

// Computes the low factors of x, given an accumulation of low factors. 
// (Low as in less than or equal to sqrt(x).)
Factors _lowFactors( Factors lfs, Factor x ) {
    // If len(lfs) == 1, it is prime.
    // If len(lfs) == 0, panic.
    if( length(lfs) < 2 )
        return lfs;

    auto next = filter (
        [&]( Factor y ) { return y <= std::sqrt(x) and x % y == 0; },
        nub(map(Mult(), lfs, lfs))
    );

    return lfs == next ? lfs : _lowFactors( move(next), x );
}

Factors primeFactors( Factor x ) {
    return filter (
        divisorOf(x), 
        takeWhile( lessThan(std::sqrt(x)), primes ) 
    );
}

Factors lowFactors( Factor x ) {
    return _lowFactors( primeFactors(x), x );
}

Factor nFactors( Factor x ) {
    auto fs = lowFactors(x);
    unsigned long long n = length( fs ) * 2;
    return n - (last(fs) == std::sqrt(x));
}

Factor triangleNumber( Factor x ) {
    return sum( enumerate(1,x) );
}

void problem12() {
    cout << "The first triangle number with over 500 divisors : " << flush;
    
    Factor n = 8;
    auto tri = [&]() { return triangleNumber(n); };
    while( nFactors(tri()) <= 500 )  
        n++;

    cout << "traiangle(" << n << ") = " << tri() << endl;
}

void problem13() {
    std::ifstream fin( "e13" );

    cout << "The first 10 digits of the sum is : " << flush;

    using DS = std::vector<Digits>;

    auto nums = mapExactly<DS> (
        digits,
        io::fileContents<std::string>(fin) 
    );

    unsigned int carry = 0;
    auto sum = reverse (
        map (
            [&]( unsigned int i ) {
                unsigned long int sum = carry;
                for( auto j : enumerate(nums) )
                    sum += nums[j][i];
                carry = sum / 10;
                return sum % 10;
            },
            // Reverse the order of the columns: LSD first.
            reverse( dupTo<std::vector>(enumerate(nums[0])) )
        )
    );

    cout << append( digits(carry), take(8,sum) ) << endl;
}

unsigned int e14Iterate( unsigned int x ) {
    return even(x) ? x/2 : 3*x + 1;
}

struct E14 {
    unsigned int starting;
    unsigned int count;

    E14( unsigned int start ) : starting(start), count(0) {
        auto n = starting;
        count = 1;
        
        while( n > 1 ) {
            count++;
            n = e14Iterate( n );
        }
    }
};

constexpr bool operator< ( const E14& a, const E14& b ) {
    return a.count < b.count;
}

constexpr bool operator> ( const E14& a, const E14& b ) {
    return a.count > b.count;
}

void problem14() {
    cout << "The maximum path of the serries starts on " << flush;

    unsigned int x = 13;
    E14 max( x );
    while( x++ < 1_M ) {
        max = std::max( max, E14(x) );
    }

    cout << max.starting << " (" << max.count << " runs)" << endl;
}

struct Cache { Vec dims; unsigned long long x; };

bool operator== ( const Vec& dims, const Cache& c ) {
    return c.dims == dims;
}
bool operator== ( const Cache& c, const Vec& dims ) {
    return c.dims == dims;
}

unsigned long long int& countWaysCached( int x, int y ) {
    if( x < y )
        return countWaysCached(y,x);

    static std::vector<Cache> cache = { {{{0,0}},0}, {{{1,1}},2} };

    auto it = cfind( Vec{{x,y}}, cache );
    if( it != end(cache) )
        return it->x;
    else {
        // When y = 1, we have a grid like this:
        //     2  1 1 ...
        //      +-+-+-...-+
        //      | | | ... |
        //      +-+-+-...-+
        // with 2 + (x-1) paths.
        //
        // When x or y equals 0, we are looking at a one-path grid.
        //    1 +-+-...-+
        //
        // Return one of the above, signifying the value, or zero signifying it
        // must be calculated.
        unsigned int ans = y==1 ? x+1 
            : x==0 or y==0;
        cache.push_back( { {{x,y}}, ans } );
        return cache.back().x;
    }
}

unsigned long long int countWays( int max, int x, int y ) {
    // Get a cached reference.
    auto& count = countWaysCached( max - x, max - y );
    return count ?  count 
        // Cache the value before returning.
        : (count = countWays(max,x+1,y) + countWays(max,x,y+1));
}

unsigned long long int countWays( int max ) {
    return countWays(max,0,0);
}

void problem15() {
    cout << "A 20x20 grid can be traversed " << flush;
    cout << countWays(20) << " ways." << endl;
}

void problem16() {
    cout << "The sum of " << flush;
    Digits ds = { 2 };
    for( unsigned int power = 1; power < 1000; power++ ) {
        ds = move(ds) * 2;
    }
    cout << "2^1000 is " << flush;
    cout << sum(ds) << endl;
}

std::string ones( unsigned int x ) {
    static const std::vector<std::string> strings = {
        "", "one", "two", "three", "four", "five", 
        "six", "seven", "eight", "nine", 
    };
    return strings[ x % 10 ];
}

std::string tens( unsigned int x ) {
    if( x < 10 )
        return ones(x);

    static const std::vector<std::string> teens = {
        "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", 
        "sixteen", "seventeen", "eighteen", "nineteen"
    };

    x = x % 100;

    if( x >= 10 and x < 20 )
        return teens[ x % 10 ];

    static const std::vector<std::string> values = {
        "", "ten", "twenty", "thirty", "forty", "fifty", 
        "sixty", "seventy", "eighty", "ninety" 
    };
    unsigned int i = x / 10;
    const std::string HUNDRED = "";
    auto o = ones( x );
    return (x >= 100 and i == 0 ? HUNDRED : values[ i ]) 
        + (length(o) ? "-" + o : std::string(""));
}

std::string hundreds( unsigned int x ) {
    auto t = tens( x % 100 );
    auto o = ones( x / 100 );
    return x < 100 ? t 
        : length(t) ? o + " hundred and " + t
        : o + " hundred";
}

void problem17() {
    cout << "The length of every number, 1-1000, written out : " << flush;
    unsigned long long sum = 0;
    for( auto x : enumerate(1,999) ) {
        auto s = hundreds(x);
        auto c = std::count_if (
            begin(s), end(s),
            []( char c ){
                return not std::isspace(c) and c != '-';
            } 
        );
        sum += c;
    }
    cout << sum + length("onethousand") << endl;
}

void problem18() {
    std::ifstream fin( "e18" );

    auto rows = mapTo<std::vector> (
        []( const Line& l ) { return mapTo<std::vector>( toInt, 
                                                         words(l.ln ) ); },
        io::fileContents<Line>(fin) 
    );
    using Rows = decltype( rows );
    using Row  = list::SeqVal<Rows>;

    cout << "The maximum path computes to : " << flush;

    for( unsigned int y = 1; y < length(rows); y++ ) {
        for( auto x : enumerate(rows[y]) ) {
            using U = unsigned long long;
            U fromAbove = 0;
            if( x > 0 )
               fromAbove = rows[y-1][x-1];
            if( x < length(rows[y]) - 1 ) 
                fromAbove = std::max( fromAbove, (U)rows[y-1][x] );
            rows[y][x] += fromAbove;
        }
    }

    cout << list::maximum( last(rows) ) << endl;
}

void problem20() {
    cout << "The sum of !100 = " << flush;
    Digits ds = { 1 };
    for( unsigned int x = 100; x > 1; x-- )
        ds = move(ds) * x;
    cout << sum(ds) << endl;
}

int main() {
    problem1();
    problem2();
    problem3();
    problem4();
    problem5();
    problem6();
    problem7();
    problem8();
    problem9();
    problem10(); 
    problem11(); 
    problem12();
    problem13();
    problem14();
    problem15();
    problem16();
    problem17();
    problem18();
    // 19!
    problem20();
}
