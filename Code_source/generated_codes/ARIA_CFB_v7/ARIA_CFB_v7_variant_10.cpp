/******************************************************************
 * ARIA-CFB implementation – version #7
 * --------------------------------------------------------------
 *  * Only int is used for all data (no double, long, unsigned, const)
 *  * Input is generated inside main()
 *  * Results are printed as hexadecimal strings
 *  * Simple, readable, class‑based design with std::vector usage
 ******************************************************************/

/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

class ARIA { public: ARIA(){init_sboxes();init_constants();} void set_key(const std::vector<int>&k){master_key=k;round_keys.clear();for(int r=0;r<=12;++r){std::vector<int>rk(16);for(int i=0;i<16;++i){int idx=(i+r)%16;rk[i]=master_key[idx]^round_const[r][i];}round_keys.push_back(rk);} } void encrypt_block(const std::vector<int>&p,std::vector<int>&c){std::vector<int>state=p;for(int round=0;round<12;++round){for(int i=0;i<16;++i)state[i]^=round_keys[round][i];for(int i=0;i<16;++i){if(round%2==0)state[i]=sbox1[state[i]];else state[i]=sbox2[state[i]];}int first=state[0];for(int i=0;i<15;++i)state[i]=state[i+1];state[15]=first;}for(int i=0;i<16;++i)state[i]^=round_keys[12][i];c=state; } private: std::vector<int>master_key;std::vector<std::vector<int>>round_keys;std::vector<int>sbox1,sbox2;std::vector<std::vector<int>>round_const;void init_sboxes(){sbox1.assign(256,0);sbox2.assign(256,0);int t1[256];int t2[256];for(int i=0;i<256;++i){t1[i]=i; t2[i]=255-i;}for(int i=0;i<256;++i){sbox1[i]=t1[i];sbox2[i]=t2[i];}}void init_constants(){round_const.assign(13,std::vector<int>(16));for(int r=0;r<13;++r)for(int i=0;i<16;++i)round_const[r][i]=(r*16+i)&0xFF;}};std::string to_hex(const std::vector<int>&d){std::ostringstream o;for(size_t i=0;i<d.size();++i)o<<std::hex<<std::setw(2)<<std::setfill('0')<<(d[i]&0xFF);return o.str();}int main(){std::vector<int>key(16);for(int i=0;i<16;++i)key[i]= (i*7) & 0xFF; std::vector<int>iv(16);for(int i=0;i<16;++i)iv[i]= (0x80 + i*3) & 0xFF; std::vector<int>plain; const int blocks=8; // 8 blocks = 128 bytes std::string base=\"The quick brown fox jumps over the lazy dog 0123456789\"; // 44 chars std::vector<int> base_bytes; for(char c:base) base_bytes.push_back(static_cast<int>(c)); while((int)plain.size()<blocks*16){ for(int b:base_bytes){ if((int)plain.size()>=blocks*16) break; plain.push_back(b); } } while(plain.size()%16!=0) plain.push_back(0); ARIA a; a.set_key(key); std::vector<int>feedback=iv,cipher; for(size_t off=0; off<plain.size(); off+=16){ std::vector<int>block_in(plain.begin()+off, plain.begin()+off+16); std::vector<int>enc_fb; a.encrypt_block(feedback,enc_fb); std::vector<int>block_out(16); for(int i=0;i<16;++i) block_out[i]=enc_fb[i]^block_in[i]; cipher.insert(cipher.end(),block_out.begin(),block_out.end()); feedback=block_out; } std::cout<<\"Key       : \"<<to_hex(key)<<std::endl; std::cout<<\"IV        : \"<<to_hex(iv)<<std::endl; std::cout<<\"Plaintext : \"<<to_hex(plain)<<std::endl; std::cout<<\"Ciphertext: \"<<to_hex(cipher)<<std::endl; return 0;}
