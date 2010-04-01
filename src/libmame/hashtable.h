/** **************************************************************************
 * tashtable.h
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include "osdcore.h"
#include <stdint.h>
#include <string.h>


namespace Hash
{
#if 0
    /* To fix emacs indentation */
}
#endif

/**
 * HashCode stores a hash code; it wraps an unsigned int so as to allow a
 * custom hash code operator to be defined within a class as:
 *
 * void operator >>(HashCode &) const
 *
 * Or outside of a class as:
 *
 * void operator >>(const KeyT &, HashCode &)
 **/
class HashCode
{
public:

    unsigned int code;

    HashCode &operator =(unsigned int _code)
    {
        code = _code;
        return *this;
    }
};


/**
 * HashTable utility functions.
 **/
class Util
{
public:

    static unsigned int Hash(const char *bytes, int bytecount);

    /**
     * This is a specialization of Hash for 4 byte values, which is faster
     * than the generic Hash function above, and spreads hashed values out
     * evenly.
     **/
    static inline unsigned int Hash(uint32_t value)
    {
        value = (value + 0x7ed55d16) + (value << 12);
        value = (value ^ 0xc761c23c) ^ (value >> 19);
        value = (value + 0x165667b1) + (value <<  5);
        value = (value + 0xd3a2646c) ^ (value <<  9);
        value = (value + 0xfd7046c5) + (value <<  3);
        value = (value ^ 0xb55a4f09) ^ (value >> 16);
        return value;
    }

    /**
     * These are the hash table sizes that the HashTable template
     * implementation uses.
     **/
    static const int hashTableSizesG[16];
};


/**
 * This class wraps a const char * and allows it to be used as a
 * HashTable key.  Memory management of the wrapped pointer is up to the
 * user of this class.
 **/
class StringKey
{
public:

    StringKey()
    {
    }
    
    StringKey(const char *key)
        : keyM(key)
    {
    }
    
    void operator >>(HashCode &hc) const
    {
        hc = Util::Hash(keyM, strlen(keyM));
    }

    bool operator ==(const StringKey &other) const
    {
        return !strcmp(keyM, other.keyM);
    }

private:
    
    const char *keyM;
};


/* HashTable template. */
template<typename KeyT, typename ValueT>
class Table
{
private:

    class Entry
    {
    public:
        // Cached hash code for this key, so that it doesn't need to be
        // recomputed
        uint32_t hashCodeM;
                
        // The next element in the linked list of all Entries which collide at
        // the same hash table index because their hashCodeM % tableSize was
        // the same
        Entry *pNextM;

        // The key for this Entry
        KeyT keyM;
        
        // The value of this Entry
        ValueT valueM;

        void *operator new(size_t sz)
        {
            return osd_malloc(sz);
        }

        void operator delete(void *mem)
        {
            osd_free(mem);
        }
    };


public:

    /**
     * Constructor.
     **/
    Table()
        : sizeTableIndexM(0), entryCountM(0)
    {
        tableM = (Entry **) osd_malloc
            (sizeof(Entry *) * Util::hashTableSizesG[0]);

        memset(tableM, 0, sizeof(Entry *) * this->Size());
    }

    /**
     * Destructor
     **/
    ~Table()
    {
        this->Clear();
        osd_free(tableM);
    }

    /**
     * Returns the number of key/value pairs stored in the HashTable.
     *
     * @return the number of key/value pairs stored in the HashTable.
     **/
    int Count() const
    {
        return entryCountM;
    }

    /**
     * Puts a key into the hashtable and returns a pointer to the value thus
     * hashed.  Also returns whether or not the key was already hashed or if
     * this operation created a new [key, value] pair within the hashtable.
     *
     * @param key is the key to hash the value by
     * @param pValue returns a pointer to the ValueT object now hashed by
     *        the key
     * @return true if [pValue] was already hashed by [key] in this hashtable,
     *         false if [pValue] is a new object, now hashed by [key] in the
     *         hashtable
     **/
    bool Put(const KeyT &key, /* returns */ ValueT *&pValue)
    {
        HashCode hc;
        key >> hc;
        unsigned int tableIndex = hc.code % this->Size();
    
        Entry *pEntry = tableM[tableIndex];

        while (pEntry && ((pEntry->hashCodeM != hc.code) ||
                          !(pEntry->keyM == key))) {
            pEntry = pEntry->pNextM;
        }
    
        if (pEntry) {
            pValue = &(pEntry->valueM);
            return true;
        }

        if ((entryCountM == ((75 * this->Size()) / 100)) &&
            (sizeTableIndexM < 15)) {
            this->Rehash(1);
            return this->Put(key, /* returns */ pValue);
        }

        pEntry = new Entry();
        pEntry->hashCodeM = hc.code;
        pEntry->keyM = key;
        pEntry->pNextM = tableM[tableIndex];
        tableM[tableIndex] = pEntry;

        entryCountM++;
        
        pValue = &(pEntry->valueM);

        return false;
    }

    /**
     * Looks up the value hashed by the given key, returning a pointer to
     * the actual value as it is stored within the hashtable.  The value
     * pointed to is owned by the hashtable.
     *
     * @param key is the key of the value to look up
     * @return A pointer to the value stored in the hashtable if the key
     *         hashed a value in the HashTable, or NULL if not
     **/
    ValueT *Get(const KeyT &key)
    {
        HashCode hc;
        key >> hc;
        
        Entry *pEntry = tableM[hc.code % this->Size()];

        while (pEntry && ((pEntry->hashCodeM != hc.code) ||
                          !(pEntry->keyM == key))) {
            pEntry = pEntry->pNextM;
        }

        if (pEntry) {
            return &(pEntry->valueM);
        }

        return 0;
    }

    /**
     * Looks up the value hashed by the given key, returning a pointer to
     * the actual value as it is stored within the hashtable.  The value
     * pointed to is owned by the hashtable.  This is the const version of
     * this type of Get().
     *
     * @param key is the key of the value to look up
     * @return A pointer to the value stored in the hashtable if the key
     *         hashed a value in the HashTable, or NULL if not
     **/
    const ValueT *Get(const KeyT &key) const
    {
        ValueT *ret = ((Table *) this)->Get(key);

        return ret;
    }

    /**
     * Removes a key, and the value it hashed, from the HashTable.
     * Additionally, if a value was removed, it is returned in [oldValue].
     *
     * @param key is the key to remove
     * @param oldValue returns the value that was removed, if one was,
     *        otherwise is unchanged
     * @return true if the key existed in the HashTable and was removed,
     *         false if not
     **/
    bool Remove(const KeyT &key, /* returns */ ValueT &oldValue)
    {
        HashCode hc;
        key >> hc;
    
        Entry **ppEntry = &(tableM[hc.code % this->Size()]);

        while (*ppEntry && (((*ppEntry)->hashCodeM != hc.code) ||
                            !((*ppEntry)->keyM != key))) {
            ppEntry = &((*ppEntry)->pNextM);
        }

        if (*ppEntry) {
            Entry *pOldEntry = *ppEntry;

            oldValue = pOldEntry->valueM;

            *ppEntry = (*ppEntry)->pNextM;

            delete pOldEntry;

            entryCountM--;

            // If this method is allowed to rehash, and if the size is now
            // smaller than 1/2 of the load at 2 sizes down, hash down
            if ((sizeTableIndexM > 1) &&
                (entryCountM < 
                 ((Util::hashTableSizesG[sizeTableIndexM - 2] * 
                   75) / 200))) {
                this->Rehash(-2);
            }

            return true;
        }

        return false;
    }

    /**
     * Clears the hashtable, removing all key/value pairs.  Note that there is
     * no way to get all of the values cleared, so if it is important that
     * each value is handled in some way as it is removed (for example, being
     * deleted or dereferenced), then use an Iterator and remove the values
     * one by one.
     **/
    void Clear()
    {
        for (int i = 0; i < this->Size(); i++) {
            Entry *pEntry = tableM[i];
            while (pEntry) {
                Entry *pNextEntry = pEntry->pNextM;
                delete pEntry;
                pEntry = pNextEntry;
            }
        }

        if (sizeTableIndexM == 0) {
            entryCountM = 0;
        }
        else {
            sizeTableIndexM = 0;
            osd_free(tableM);
            tableM = (Entry **) osd_malloc
                (sizeof(Entry *) * Util::hashTableSizesG[0]);
        }

        memset(tableM, 0, sizeof(Entry *) * this->Size());
    }

    /**
     * This is an iterator, which can iterate over the contents of a
     * Hash::Table.  It is not thread-safe with respect to the hash table, so
     * it is important that the caller have exclusive access to the has table
     * while running through the iterator.
     **/
    class Iterator
    {
    public:

        /**
         * Create an iterator; it will start off at the first available
         * element.
         **/
        Iterator(const Table &ht)
            : htM(ht), indexM(0), sizeM(ht.Size()), pEntryM(0)
        {
            this->Advance();
        }

        /**
         * Returns true if the iterator has a current element, false if not.
         * 
         * @return true if the iterator has a current element, false if not.
         **/
        bool HasCurrent() const
        {
            return pEntryM;
        }

        /**
         * Returns the key of the current hash table element.  This should
         * only be called when HasCurrent() returns true.
         *
         * @return the key of the current hash table element
         **/
        const KeyT &GetCurrentKey() const
        {
            return pEntryM->keyM;
        }

        /**
         * Returns the value of the current hash table element.  This should
         * only be called when HasCurrent() returns true.
         *
         * @return the value of the current hash table element
         **/
        const ValueT &GetCurrentValue() const
        {
            return pEntryM->valueM;
        }

        /**
         * Advanced to the next element of the hash table (or past it if it is
         * the last element, in which case HasCurrent() will return false).
         **/
        void Advance()
        {
            while (indexM < sizeM) {
                if (pEntryM) {
                    if ((pEntryM = pEntryM->pNextM)) {
                        break;
                    }
                }
                else {
                    if ((pEntryM = htM.tableM[indexM])) {
                        break;
                    }
                }
                indexM++;
            }
        }

    private:

        const Table &htM;
        int indexM, sizeM;
        const Entry *pEntryM;
    };

private:

    int Size() const
    {
        return Util::hashTableSizesG[sizeTableIndexM];
    }

    void Rehash(int increment)
    {
        int oldTableSize = this->Size();
        sizeTableIndexM += increment;
        int newTableSize = this->Size();
        Entry **newTable = 
            (Entry **) osd_malloc(sizeof(Entry *) * newTableSize);
        memset(newTable, 0, sizeof(Entry *) * newTableSize);

        for (int i = 0; i < oldTableSize; i++) {
            Entry *pEntry = tableM[i];
            while (pEntry) {
                int newIndex = pEntry->hashCodeM % newTableSize;
                Entry *pNextEntry = pEntry->pNextM;
                pEntry->pNextM = newTable[newIndex];
                newTable[newIndex] = pEntry;
                pEntry = pNextEntry;
            }
        }

        osd_free(tableM);

        tableM = newTable;
    }

    // The index into the size array of the current size
    int sizeTableIndexM;

    // The number of elements currently in the hash table
    int entryCountM;

    // This is the hashtable
    Entry **tableM;
};


/**
 * Hash operators for standard types
 **/

inline void operator >>(bool val, HashCode &hc)
{
    hc = val;
}

inline void operator >>(char val, HashCode &hc)
{
    hc = val;
}

inline void operator >>(signed char val, HashCode &hc)
{
    hc = val;
}

inline void operator >>(unsigned char val, HashCode &hc)
{
    hc = val;
}

inline void operator >>(signed short val, HashCode &hc)
{
    hc = Util::Hash((uint32_t) val);
}

inline void operator >>(unsigned short val, HashCode &hc)
{
    hc = Util::Hash((uint32_t) val);
}

inline void operator >>(signed int val, HashCode &hc)
{
    if (sizeof(int) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(unsigned int val, HashCode &hc)
{
    if (sizeof(int) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(signed long val, HashCode &hc)
{
    if (sizeof(long) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(unsigned long val, HashCode &hc)
{
    if (sizeof(long) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(signed long long val, HashCode &hc)
{
    if (sizeof(long long) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(unsigned long long val, HashCode &hc)
{
    if (sizeof(long long) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(float val, HashCode &hc)
{
    hc = Util::Hash((char *) &val, sizeof(val));
}

inline void operator >>(double val, HashCode &hc)
{
    hc = Util::Hash((char *) &val, sizeof(val));
}

inline void operator >>(long double val, HashCode &hc)
{
    hc = Util::Hash((char *) &val, sizeof(val));
}

inline void operator >>(wchar_t val, HashCode &hc)
{
    if (sizeof(wchar_t) == 4) {
        hc = Util::Hash((uint32_t) val);
    }
    else {
        hc = Util::Hash((char *) &val, sizeof(val));
    }
}

inline void operator >>(const char *val, HashCode &hc)
{
    hc = Util::Hash(val, strlen(val));
}

} /* namespace HashTable */

#endif /* __HASH_TABLE_H__ */
