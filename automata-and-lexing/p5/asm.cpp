#include <cstdio>
#include <cctype>
#include <cstring>

// ----- tiny helpers (ctype with unsigned char) -----
static inline bool is_alpha(char c) { return std::isalpha((unsigned char)c); }
static inline bool is_digit(char c) { return std::isdigit((unsigned char)c); }
static inline bool is_alnum(char c) { return std::isalnum((unsigned char)c); }
static inline bool is_space(char c) { return std::isspace((unsigned char)c); }

// ----- classifiers operating on raw buffers -----
static bool isDirective(const char* s, int n) {
    if (n < 2) return false;
    if (s[0] != '.') return false;
    if (!is_alpha(s[1])) return false;
    for (int i = 2; i < n; ++i) if (!is_alnum(s[i])) return false;
    return true;
}
static bool isLabel(const char* s, int n) {
    if (n < 2) return false;                  // at least "a:"
    if (s[n-1] != ':') return false;
    if (!is_alpha(s[0])) return false;
    for (int i = 1; i < n-1; ++i) if (!is_alnum(s[i])) return false;
    return true;
}
static bool isID(const char* s, int n) {
    if (n < 1) return false;
    if (!is_alpha(s[0])) return false;
    for (int i = 1; i < n; ++i) if (!is_alnum(s[i])) return false;
    return true;
}
static bool isReg(const char* s, int n) {
    // "$" followed by 1 or 2 decimal digits (00..99 allowed per spec)
    if (n < 2 || n > 3) return false;
    if (s[0] != '$') return false;
    for (int i = 1; i < n; ++i) if (!is_digit(s[i])) return false;
    return true;
}
static bool isHexInt(const char* s, int n) {
    if (n < 3) return false;
    if (!(s[0] == '0' && s[1] == 'x')) return false;
    for (int i = 2; i < n; ++i) {
        char c = s[i];
        if (!is_digit(c) && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) return false;
    }
    return true;
}
static bool isDec(const char* s, int n) {
    // "0" XOR (optional "-" then [1-9][0-9]*)
    if (n == 1 && s[0] == '0') return true;
    int i = 0;
    if (n >= 1 && s[0] == '-') i = 1;
    if (i >= n) return false;                 // "-" alone invalid
    if (!(s[i] >= '1' && s[i] <= '9')) return false; // no leading zero for nonzero ints
    for (i = i + 1; i < n; ++i) if (!is_digit(s[i])) return false;
    return true;
}

static void printToken(const char* kind, const char* s, int n) {
    if (std::strcmp(kind, "COMMA") == 0)   { std::puts("COMMA ,"); return; }
    if (std::strcmp(kind, "LPAREN") == 0)  { std::puts("LPAREN ("); return; }
    if (std::strcmp(kind, "RPAREN") == 0)  { std::puts("RPAREN )"); return; }
    std::fputs(kind, stdout); std::fputc(' ', stdout);
    std::fwrite(s, 1, (size_t)n, stdout);
    std::fputc('\n', stdout);
}

static bool classifyAndEmit(const char* s, int n) {
    if (n == 0) return true;
    if (n == 1 && s[0] == ',') { printToken("COMMA",  nullptr, 0); return true; }
    if (n == 1 && s[0] == '(') { printToken("LPAREN", nullptr, 0); return true; }
    if (n == 1 && s[0] == ')') { printToken("RPAREN", nullptr, 0); return true; }

    if (isDirective(s, n)) { printToken("DIRECTIVE", s, n); return true; }
    if (isLabel(s, n))     { printToken("LABEL",     s, n); return true; }
    if (isReg(s, n))       { printToken("REG",       s, n); return true; }
    if (isHexInt(s, n))    { printToken("HEXINT",    s, n); return true; }
    if (isDec(s, n))       { printToken("DEC",       s, n); return true; }
    if (isID(s, n))        { printToken("ID",        s, n); return true; }

    // error
    std::fputs("ERROR: Unknown token: ", stderr);
    std::fwrite(s, 1, (size_t)n, stderr);
    std::fputc('\n', stderr);
    return false;
}

int main() {
    // States: whitespace, token, comment
    enum { WS, TOK, COMMENT } state = WS;

    // Fixed token buffer (line lengths in tests are small; adjust if needed)
    char tok[4096];
    int  tn = 0;

    int ch;
    bool hadError = false;

    auto flushToken = [&](bool &ok){
        if (tn > 0) {
            ok = classifyAndEmit(tok, tn);
            tn = 0;
        } else {
            ok = true;
        }
    };

    bool ok = true;
    while ((ch = std::getchar()) != EOF) {
        char c = (char)ch;

        if (state == COMMENT) {
            if (c == '\n') state = WS; // reset at end of line
            continue;
        }

        // comment start?
        if (c == '#' || c == ';') {
            // flush pending token, then ignore rest of line
            flushToken(ok);
            if (!ok) { hadError = true; break; }
            state = COMMENT;
            continue;
        }

        // separators that are tokens themselves: , ( )
        if (c == ',' || c == '(' || c == ')') {
            // flush the running token first
            flushToken(ok);
            if (!ok) { hadError = true; break; }
            // emit single-char punctuation token
            char p = c;
            if (!classifyAndEmit(&p, 1)) { hadError = true; break; }
            state = WS;
            continue;
        }

        // whitespace/newline separate tokens
        if (is_space(c)) {
            flushToken(ok);
            if (!ok) { hadError = true; break; }
            state = WS;
            continue;
        }

        // otherwise, append to current token
        if (tn < (int)sizeof(tok)) {
            tok[tn++] = c;
        } else {
            // token too long -> error
            std::fputs("ERROR: Unknown token: <too long>\n", stderr);
            hadError = true;
            break;
        }
        state = TOK;
    }

    if (!hadError) {
        flushToken(ok);
        if (!ok) hadError = true;
    }

    return hadError ? 1 : 0;
}
