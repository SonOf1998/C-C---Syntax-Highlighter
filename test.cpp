#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Kollekcio.hpp"
#include "Kor.hpp"
#include "Nszog.hpp"
#include "Sikidom.hpp"

#include "gtest_lite.h"
//#include "memtrace.h"

#define TESZT

std::vector<int> v;
std::vector<std::unique_ptr<int>> v2;
std::vector<DateTime> v3;

namespace
{
    // I don't want to #include <algorithm> for a single function template :)
    template <typename T>
    void my_swap(T& x, T& y)
    {
        T z(x);
        x = y;
        y = z;
    }

    typedef unsigned char byte;

    const byte CANARY[] = {0x5A, 0xFE, 0x6A, 0x8D,
                           0x5A, 0xFE, 0x6A, 0x8D,
                           0x5A, 0xFE, 0x6A, 0x8D,
                           0x5A, 0xFE, 0x6A, 0x8D};

    bool canary_dead(const byte* cage)
    {
        bool dead = memcmp(cage, CANARY, sizeof CANARY);
        if (dead)
        {
            for (size_t i = 0; i < sizeof CANARY; ++i)
            {
                byte b = cage[i];
                printf(b == CANARY[i] ? "__ " : "%2X ", b);
            }
            putchar('\n');
        }
        return dead;
    }

    enum kind_of_memory {AVAILABLE, TOMBSTONE, NON_ARRAY_MEMORY, ARRAY_MEMORY};

    const char* kind_string[] = {0, 0, "non-array memory", "    array memory"};

    struct metadata
    {
        byte* address;
        size_t size;
        kind_of_memory kind;

        bool in_use() const
        {
            return kind & 2;
        }

        void print() const
        {
            printf("%s at %p (%d bytes)\n", kind_string[kind], address, size);
        }

        bool must_keep_searching_for(void* address)
        {
            return kind == TOMBSTONE || (in_use() && address != this->address);
        }

        bool canaries_alive() const
        {
            bool alive = true;
            if (canary_dead(address - sizeof CANARY))
            {
                printf("ERROR:    buffer underflow at %p\n", address);
                alive = false;
            }
            if (canary_dead(address + size))
            {
                printf("ERROR:     buffer overflow at %p\n", address);
                alive = false;
            }
            return alive;
        }
    };

    const size_t MINIMUM_CAPACITY = 11;

    class hashtable
    {
        metadata* data;
        size_t used;
        size_t capacity;
        size_t tombstones;

    public:

        size_t size() const
        {
            return used - tombstones;
        }

        void print() const
        {
            for (size_t i = 0; i < capacity; ++i)
            {
                if (data[i].in_use())
                {
                    printf(":( leaked ");
                    data[i].print();
                }
            }
        }

        hashtable()
        {
            used = 0;
            capacity = MINIMUM_CAPACITY;
            data = static_cast<metadata*>(calloc(capacity, sizeof(metadata)));
            tombstones = 0;
        }

        ~hashtable()
        {
            free(data);
        }

        hashtable(const hashtable& that)
        {
            used = 0;
            capacity = 3 * that.size() | 1;
            if (capacity < MINIMUM_CAPACITY) capacity = MINIMUM_CAPACITY;
            data = static_cast<metadata*>(calloc(capacity, sizeof(metadata)));
            tombstones = 0;

            for (size_t i = 0; i < that.capacity; ++i)
            {
                if (that.data[i].in_use())
                {
                    insert_unsafe(that.data[i]);
                }
            }
        }

        hashtable& operator=(hashtable copy)
        {
            swap(copy);
            return *this;
        }

        void swap(hashtable& that)
        {
            my_swap(data, that.data);
            my_swap(used, that.used);
            my_swap(capacity, that.capacity);
            my_swap(tombstones, that.tombstones);
        }

        void insert_unsafe(const metadata& x)
        {
            *find(x.address) = x;
            ++used;
        }

        void insert(const metadata& x)
        {
            if (2 * used >= capacity)
            {
                hashtable copy(*this);
                swap(copy);
            }
            insert_unsafe(x);
        }

        metadata* find(void* address)
        {
            size_t index = reinterpret_cast<size_t>(address) % capacity;
            while (data[index].must_keep_searching_for(address))
            {
                ++index;
                if (index == capacity) index = 0;
            }
            return &data[index];
        }

        void erase(metadata* it)
        {
            it->kind = TOMBSTONE;
            ++tombstones;
        }
    } the_hashset;

    struct heap_debugger
    {
        heap_debugger()
        {
            puts("heap debugger started");
        }

        ~heap_debugger()
        {
            the_hashset.print();
            puts("heap debugger shutting down");
        }
    } the_heap_debugger;

    void* allocate(size_t size, kind_of_memory kind) throw (std::bad_alloc)
    {
        byte* raw = static_cast<byte*>(malloc(size + 2 * sizeof CANARY));
        if (raw == 0) throw std::bad_alloc();

        memcpy(raw, CANARY, sizeof CANARY);
        byte* payload = raw + sizeof CANARY;
        memcpy(payload + size, CANARY, sizeof CANARY);

        metadata md = {payload, size, kind};
        the_hashset.insert(md);
        printf("allocated ");
        md.print();
        return payload;
    }

    void release(void* payload, kind_of_memory kind) throw ()
    {
        if (payload == 0) return;

        metadata* p = the_hashset.find(payload);

        if (!p->in_use())
        {
            printf("ERROR:   no dynamic memory at %p\n", payload);
        }
        else if (p->kind != kind)
        {
            printf("ERROR:wrong form of delete at %p\n", payload);
        }
        else if (p->canaries_alive())
        {
            printf("releasing ");
            p->print();
            free(static_cast<byte*>(payload) - sizeof CANARY);
            the_hashset.erase(p);
        }
    }
}

void* operator new(size_t size) throw (std::bad_alloc)
{
    return allocate(size, NON_ARRAY_MEMORY);
}

void* operator new[](size_t size) throw (std::bad_alloc)
{
    return allocate(size, ARRAY_MEMORY);
}

void operator delete(void* payload) throw ()
{
    release(payload, NON_ARRAY_MEMORY);
}

void operator delete[](void* payload) throw ()
{
    release(payload, ARRAY_MEMORY);
}

int main()
{
    #ifndef TESZT
    std::ios_base::sync_with_stdio(false);

    Kollekcio<Sikidom*> kollekcio;

    std::ifstream txtfajl("alakzatok.txt");
    std::string sor;

    if(txtfajl.is_open())
    {
        while(std::getline(txtfajl, sor))
        {
            std::istringstream iss(sor);

            Pont2D kp;
            Pont2D csp;

            if(sor.find("k") != std::string::npos)
            {
                char c;
                iss >> c >> kp >> csp;
                kollekcio.push_back(new Kor(kp, csp));

            } else {
                int n;
                iss >> n >> kp >> csp;
                kollekcio.push_back(new Nszog(kp, csp, n));
            }
        }
    }

    txtfajl.close();

    Pont2D sajat_pont;

    std::cout << "Adjon meg egy pontot: " << std::endl;
    std::cin >> sajat_pont;

    for(unsigned int i = 0; i < kollekcio.size(); i++)
    {
        if(kollekcio[i]->benne_van(sajat_pont))
        {
            kollekcio[i]->kiir();
        }
    }

    #endif


    /// JPORTÁS TESZTEK
    #ifdef TESZT
    Kollekcio kollekcio;


    TEST(Test1, kollekcio_hozzaadogatos_teszt)
    {
        EXPECT_EQ((size_t)0, kollekcio.size())          << "default konstruktor nem 0-at ad meretnek" << std::endl;
        EXPECT_EQ((size_t)10, kollekcio.capacity())     << "hibas kezdeti kapacitas" << std::endl;

        kollekcio.push_back(new Nszog({0,0}, {0,1}, 15));
        EXPECT_EQ((size_t)1, kollekcio.size())          << "nem no a meret uj elem hozzavetelenel" << std::endl;

        /// mi van ha elérjük a kollekció kapacitásának maximumát?
        for(int i = 0; i < 10; i++)
        {
            kollekcio.push_back(new Kor({0,0}, {0,1}));
        }

        EXPECT_EQ((size_t)20, kollekcio.capacity())     << "nem duplazta meg a kollekcionak lefoglalt helyet I." << std::endl;

        /// és ha megint?
        for(int i = 0; i < 10; i++)
        {
            kollekcio.push_back(new Kor({0,0}, {0,1}));
        }

        EXPECT_EQ((size_t)40, kollekcio.capacity())     << "nem duplazta meg a kollekcionak lefoglalt helyet II." << std::endl;
    } ENDM

    TEST(Test2, tulindexeleses_teszt)
    {
        /// a kollekciónk mérete az elõzõ teszt után 21, azaz a 20.indexig nem várunk kivételt
        EXPECT_NO_THROW(kollekcio[20]);
        /// a 20.-nál már várunk
        EXPECT_THROW(kollekcio[21], std::out_of_range);
    } ENDM

    TEST(Test3, belso_pont_teszt)
    {
        /// felhasználjuk kollekcio[0]-át, ami egy origó középpontó egység sugarú kör

        /// (1/2, 1/2) pont benne van
        Pont2D p = {0.5, 0.5};
        EXPECT_TRUE(kollekcio[0]->benne_van(p))         << "az origo pedig belso pont" << std::endl;

        /// a (0, 1) pont a körvonalon van, ezt nem fogadjuk el belsõ pontnak
        p = {0.0, 1.0};
        EXPECT_FALSE(kollekcio[0]->benne_van(p))        << "a korvonal nem belso pont!" << std::endl;

        /// a (100, 100) pont nyilván kivül esik
        p = {100, 100};
        EXPECT_FALSE(kollekcio[0]->benne_van(p));


        /// hasonló próbák egy (1, 1) középpontú négyzettel, aminek egyik csúcsa a (0, 0);
        Nszog negyzet({1, 1}, {0, 0}, 4);

        /// belsõ pont
        p = {0.2, 1.8};
        EXPECT_TRUE(negyzet.benne_van(p));

        /// egy csúcs
        p = {2, 2};
        EXPECT_FALSE(negyzet.benne_van(p));

        /// külsõ pont
        p = {100, 100};
        EXPECT_FALSE(negyzet.benne_van(p));
    } ENDM

    for(Kollekcio::iterator iter = kollekcio.begin(); iter != kollekcio.end(); ++iter)
    {
        delete *iter;
        *iter = NULL;
    }


    /*int i = 0;
    for(Sikidom* s : kollekcio)
    {
        std::cout << s << std::endl;
        std::cout << i++ << std::endl;
        if(s != NULL)
        {
            delete s;
        }
    }*/

    /*int i = 0;
    for(Kollekcio::iterator iter = kollekcio.begin(); iter != kollekcio.end(); ++iter)
    {
        std::cout << i++ << std::endl;

        delete *iter;
    }*/

    #endif

    return 0;
}
