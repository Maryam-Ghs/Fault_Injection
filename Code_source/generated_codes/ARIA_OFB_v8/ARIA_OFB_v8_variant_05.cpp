#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */
int main()
{
    std::srand( static_cast<int>( std::time( 0 ) ) );

    int *arrKey = new int[16];
    int iIdx = 0;
    while ( iIdx < 16 )
    {
        arrKey[iIdx] = (iIdx % 4 < 2) ? 42 : 99; // 42,42,99,99,...
        iIdx = iIdx + 1;
    }

    int *arrIV = new int[16];
    iIdx = 0;
    while ( iIdx < 16 )
    {
        arrIV[iIdx] = 77; // all identical
        iIdx = iIdx + 1;
    }

    const int BLOCKS = 3;
    const int BYTES  = BLOCKS * 16;
    int *arrPlain = new int[BYTES];
    iIdx = 0;
    while ( iIdx < BYTES )
    {
        arrPlain[iIdx] = ((iIdx / 3) % 2 == 0) ? 5 : 10; // 5,5,5,10,10,10,...
        iIdx = iIdx + 1;
    }

    int sBox1[256], sBox2[256], sBox3[256], sBox4[256];
    iIdx = 0;
    while ( iIdx < 256 )
    {
        sBox1[iIdx] = ( iIdx * 7 ) % 256;
        sBox2[iIdx] = ( iIdx * 13 ) % 256;
        sBox3[iIdx] = ( iIdx * 17 ) % 256;
        sBox4[iIdx] = ( iIdx * 23 ) % 256;
        iIdx = iIdx + 1;
    }

    const int ROUNDS = 12;
    int **arrRound = new int*[ROUNDS];
    iIdx = 0;
    while ( iIdx < ROUNDS )
    {
        arrRound[iIdx] = new int[16];
        iIdx = iIdx + 1;
    }

    int rIdx = 0;
    while ( rIdx < ROUNDS )
    {
        int shift = rIdx;
        int j = 0;
        while ( j < 16 )
        {
            int src = ( j + shift ) % 16;
            arrRound[rIdx][j] = arrKey[src];
            j = j + 1;
        }
        rIdx = rIdx + 1;
    }

    auto encryptBlock = [&](int *inBlk, int *outBlk)
    {
        int *tmp = new int[16];
        int k = 0;
        while ( k < 16 )
        {
            tmp[k] = inBlk[k];
            k = k + 1;
        }

        int rnd = 0;
        while ( rnd < ROUNDS )
        {
            int p = 0;
            while ( p < 16 )
            {
                int sel = p % 4;
                if ( sel == 0 ) tmp[p] = sBox1[ tmp[p] ];
                else if ( sel == 1 ) tmp[p] = sBox2[ tmp[p] ];
                else if ( sel == 2 ) tmp[p] = sBox3[ tmp[p] ];
                else tmp[p] = sBox4[ tmp[p] ];
                p = p + 1;
            }

            int *mixed = new int[16];
            int a = 0;
            while ( a < 16 )
            {
                int left = tmp[(a+1)%16];
                int right = tmp[(a+15)%16];
                mixed[a] = (tmp[a] + left + right) % 256;
                a = a + 1;
            }

            int b = 0;
            while ( b < 16 )
            {
                tmp[b] = mixed[b];
                b = b + 1;
            }
            delete [] mixed;

            int c = 0;
            while ( c < 16 )
            {
                tmp[c] = tmp[c] ^ arrRound[rnd][c];
                c = c + 1;
            }

            rnd = rnd + 1;
        }

        int d = 0;
        while ( d < 16 )
        {
            outBlk[d] = tmp[d];
            d = d + 1;
        }
        delete [] tmp;
    };

    int *arrCipher = new int[BYTES];
    int *arrStream = new int[16];
    encryptBlock( arrIV, arrStream );

    int blockIdx = 0;
    while ( blockIdx < BLOCKS )
    {
        int bytePos = 0;
        while ( bytePos < 16 )
        {
            int srcIdx = blockIdx * 16 + bytePos;
            arrCipher[srcIdx] = arrPlain[srcIdx] ^ arrStream[bytePos];
            bytePos = bytePos + 1;
        }

        int *nextStream = new int[16];
        encryptBlock( arrStream, nextStream );
        int e = 0;
        while ( e < 16 )
        {
            arrStream[e] = nextStream[e];
            e = e + 1;
        }
        delete [] nextStream;

        blockIdx = blockIdx + 1;
    }

    std::cout << "Key (hex): ";
    iIdx = 0;
    while ( iIdx < 16 )
    {
        std::cout << std::hex << ( arrKey[iIdx] & 0xFF );
        if ( iIdx != 15 ) std::cout << " ";
        iIdx = iIdx + 1;
    }
    std::cout << std::dec << "\n";

    std::cout << "IV  (hex): ";
    iIdx = 0;
    while ( iIdx < 16 )
    {
        std::cout << std::hex << ( arrIV[iIdx] & 0xFF );
        if ( iIdx != 15 ) std::cout << " ";
        iIdx = iIdx + 1;
    }
    std::cout << std::dec << "\n\n";

    std::cout << "Plaintext (hex):\n";
    blockIdx = 0;
    while ( blockIdx < BLOCKS )
    {
        int pos = blockIdx * 16;
        iIdx = 0;
        while ( iIdx < 16 )
        {
            std::cout << std::hex << ( arrPlain[pos + iIdx] & 0xFF );
            if ( iIdx != 15 ) std::cout << " ";
            iIdx = iIdx + 1;
        }
        std::cout << std::dec << "\n";
        blockIdx = blockIdx + 1;
    }

    std::cout << "\nCiphertext (hex):\n";
    blockIdx = 0;
    while ( blockIdx < BLOCKS )
    {
        int pos = blockIdx * 16;
        iIdx = 0;
        while ( iIdx < 16 )
        {
            std::cout << std::hex << ( arrCipher[pos + iIdx] & 0xFF );
            if ( iIdx != 15 ) std::cout << " ";
            iIdx = iIdx + 1;
        }
        std::cout << std::dec << "\n";
        blockIdx = blockIdx + 1;
    }

    delete [] arrKey;
    delete [] arrIV;
    delete [] arrPlain;
    delete [] arrCipher;
    delete [] arrStream;
    int cleanIdx = 0;
    while ( cleanIdx < ROUNDS )
    {
        delete [] arrRound[cleanIdx];
        cleanIdx = cleanIdx + 1;
    }
    delete [] arrRound;
    return 0;
}
