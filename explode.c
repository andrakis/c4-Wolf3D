// explode.c -- PKWARE's Data Compression Library "implode" format,
// decompressed. Apogee's installers used it: the shareware Wolfenstein
// 3D's W3DSW14.SHR is a run of files compressed this way (unshr.c).
//
// The format, as zlib's contrib/blast documents it (Mark Adler):
// - a byte: 0, literals are raw bytes, or 1, they are Huffman coded;
// - a byte: 4, 5 or 6, the window as that many bits of distance (1, 2,
//   4 KB);
// - then bits, least significant first: 0 and a literal, or 1, a length
//   (a code and extra bits; 519 is the end), and a distance (a code,
//   then 2 bits for a length of 2 and window bits otherwise).
// The three codes are fixed, canonical, sent with each bit inverted, and
// given here as their code lengths, compactly: each byte is a length
// (low 4 bits) and a count less one (high 4).
//
// Plain C in the subset c4lc and gcc share: no tables of structs, loops
// as whiles.

enum { EX_MAXBITS = 13 };

// a code: how many of each length, then the symbols in canonical order
static int ex_lit_count[EX_MAXBITS + 1], ex_lit_sym[256];
static int ex_len_count[EX_MAXBITS + 1], ex_len_sym[16];
static int ex_dist_count[EX_MAXBITS + 1], ex_dist_sym[64];
static int ex_ready;

static unsigned char ex_litlen[] = {
    11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
    9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
    7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
    8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
    44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
    44, 173 };
static unsigned char ex_lenlen[] = { 2, 35, 36, 53, 38, 23 };
static unsigned char ex_distlen[] = { 2, 20, 53, 230, 247, 151, 248 };
static int ex_base[] = { 3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264 };
static int ex_extra[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8 };

// Build a code from its compact lengths; the number of symbols, or -1
static int ex_build(unsigned char *rep, int nrep, int *count, int *sym, int nsym) {
    int length[256], offs[EX_MAXBITS + 2], i, n, len, num, s;
    n = 0; i = 0;
    while (i < nrep) {
        len = rep[i] & 15;
        num = (rep[i] >> 4) + 1;
        while (num > 0) { if (n >= 256) return 0 - 1; length[n] = len; ++n; --num; }
        ++i;
    }
    if (n != nsym) return 0 - 1;
    i = 0; while (i <= EX_MAXBITS) { count[i] = 0; ++i; }
    s = 0; while (s < n) { ++count[length[s]]; ++s; }
    offs[1] = 0;
    i = 1; while (i < EX_MAXBITS) { offs[i + 1] = offs[i] + count[i]; ++i; }
    s = 0; while (s < n) { if (length[s]) { sym[offs[length[s]]] = s; ++offs[length[s]]; } ++s; }
    return n;
}

int ex_init() {
    if (ex_ready) return 1;
    if (ex_build(ex_litlen, sizeof(ex_litlen), ex_lit_count, ex_lit_sym, 256) < 0) return 0;
    if (ex_build(ex_lenlen, sizeof(ex_lenlen), ex_len_count, ex_len_sym, 16) < 0) return 0;
    if (ex_build(ex_distlen, sizeof(ex_distlen), ex_dist_count, ex_dist_sym, 64) < 0) return 0;
    ex_ready = 1;
    return 1;
}

// the input, a bit at a time
static unsigned char *ex_in;
static int ex_inlen, ex_inpos, ex_bitbuf, ex_bitcnt, ex_err;

static int ex_bits(int need) {
    int v;
    while (ex_bitcnt < need) {
        if (ex_inpos >= ex_inlen) { ex_err = 1; return 0; }
        ex_bitbuf = ex_bitbuf | ((ex_in[ex_inpos] & 255) << ex_bitcnt);
        ++ex_inpos;
        ex_bitcnt = ex_bitcnt + 8;
    }
    v = ex_bitbuf & ((1 << need) - 1);
    ex_bitbuf = ex_bitbuf >> need;
    ex_bitcnt = ex_bitcnt - need;
    return v;
}

// a symbol of a code: its bits arrive first-bit-first, each inverted
static int ex_decode(int *count, int *sym) {
    int len, code, first, index, n;
    code = 0; first = 0; index = 0;
    len = 1;
    while (len <= EX_MAXBITS) {
        code = code | (ex_bits(1) ^ 1);
        if (ex_err) return 0 - 1;
        n = count[len];
        if (code < first + n) return sym[index + (code - first)];
        index = index + n;
        first = (first + n) << 1;
        code = code << 1;
        ++len;
    }
    ex_err = 2;
    return 0 - 1;
}

// Decompress IN (INLEN bytes) into OUT, which holds OUTCAP. The bytes
// written, or -1 if the data is not this format or OUT is too small.
int explode(unsigned char *in, int inlen, unsigned char *out, int outcap) {
    int lit, dict, sym, len, dist, pos, k;
    if (!ex_init()) return 0 - 1;
    ex_in = in; ex_inlen = inlen; ex_inpos = 0; ex_bitbuf = 0; ex_bitcnt = 0; ex_err = 0;
    lit = ex_bits(8);
    dict = ex_bits(8);
    if (ex_err || lit > 1 || dict < 4 || dict > 6) return 0 - 1;
    pos = 0;
    while (1) {
        if (ex_bits(1)) {
            sym = ex_decode(ex_len_count, ex_len_sym);
            if (sym < 0) return 0 - 1;
            len = ex_base[sym] + ex_bits(ex_extra[sym]);
            if (len == 519) break;                     // the end
            k = len == 2 ? 2 : dict;
            sym = ex_decode(ex_dist_count, ex_dist_sym);
            if (sym < 0) return 0 - 1;
            dist = (sym << k) + ex_bits(k) + 1;
            if (ex_err || dist > pos || pos + len > outcap) return 0 - 1;
            while (len > 0) { out[pos] = out[pos - dist]; ++pos; --len; }
        } else {
            sym = lit ? ex_decode(ex_lit_count, ex_lit_sym) : ex_bits(8);
            if (ex_err || sym < 0 || pos >= outcap) return 0 - 1;
            out[pos] = sym;
            ++pos;
        }
        if (ex_err) return 0 - 1;
    }
    return pos;
}
