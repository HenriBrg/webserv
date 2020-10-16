# include <string>
# include <iostream>
# include <vector>

static std::string decodeBase64(const std::string &in) {

    int val = 0;
    int valb = -8;
    unsigned char c;
    std::string out;
    std::vector<int> tab(256,-1);
    std::string::iterator it = in.begin();
    std::string::iterator itend = in.end();

    for (int i = 0; i < 64; i++)
        tab["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i; 
    for (; it != itend; it++) {
        c = *it;
        if (tab[c] == -1) break;
        val = (val << 6) + tab[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -=8;
        }
    }
    return out;
}

int main() {

    std::cout << decodeBase64("YWxhZGRpbjpvcGVuc2VzYW1l") << std::endl;

}