#include <iostream>
#include <vector>
#include <set>
using namespace std;

namespace hashing
{
    /**
     * Container for Buckets in Entendible Hash Table
     */
    class DataBucket
    {
        // Capacity of Bucket
        int size;
        // Local Depth of Bucket
        int localDepth;
        // Set to store the rows of the bucket (always unique)
        // remark: Data here is unique since rows in a database are unique
        set<int> data;
        friend class ExtendibleHashMap;

    public:
        /**
         * Contructor for DataBucket
         * @param _size size of the bucket
         * @param _localDepth local depth of the bucket
         */
        DataBucket(int _size, int _localDepth) : size(_size), localDepth(_localDepth) {}

        /**
        * Insert Element to the data Bucket
        * @param _data integer to be inserted
        * @retval 0 if successfully inserted
        * @retval 1 if already present
        * @retval 2 if bucket overflowed
        */
        int insert(int _data)
        {
            if (data.find(_data) != data.end())
            {
                return 1;
            }
            data.insert(_data);

            if (data.size() <= size)
            {
                return 0;
            }
            // Overflow
            return 2;
        }

        /**
         * Remove Element from the data Bucket
         * @param _key integer to be removed
         */
        void erase(int _key)
        {
            data.erase(_key);
        }

        /**
         * Destructor for the DataBucket
         * Removes all the elements from the bucket
         */
        ~DataBucket() { data.clear(); }
    };

    /**
     * Entendible Hash Table
     *
     * This class implements the Extendible Hashing Scheme.
     * Private Data Members of buckets are accessible to this class
     */
    class ExtendibleHashMap
    {
        // Global Depth for the Hash Table
        int globalDepth;
        // Size of corresponding buckets
        int bucketSize;
        // Container to store buckets
        vector<DataBucket *> buckets;

        /**
         * Get the bucket index for a given key
         * @param key key to be hashed
         * @return bucket index
         */
        int _getBucketIndex(int key)
        {
            return _getBucketIndex(key, globalDepth);
        }

        /**
         * Get the bucket index for a given key
         * @param _key key to be hashed
         * @param _globalDepth global depth of the hash table
         * @return bucket index
         */
        int _getBucketIndex(int _key, int _globalDepth)
        {
            return _key % (1 << _globalDepth);
        }

        /**
         * Extend Hash Table to double the number of buckets
         *
         * The newly created buckets point to the matching Buckets
         * from the old global depth.
         */
        void extend()
        {
            int oldDepth = globalDepth;
            globalDepth++;
            buckets.resize(1 << globalDepth);

            // Adjust the pointers from 2^oldDepth to the end
            // to the corresponding buckets from 2^oldDepth
            for (int i = 1 << oldDepth; i < buckets.size(); i++)
            {
                buckets[i] = buckets[_getBucketIndex(i, oldDepth)];
            }
        }

        /**
         * Create a split image for the bucket.
         *
         * This function also splits the data from the bucket according to the
         * new global depth. Finally the local depth of the split bucket is incremented.
         * @param splitIndex index of the bucket to be split.
         * Any of the two pointers pointing to the same bucket
         * are allowed.
         */
        void splitImage(int splitIndex)
        {
            // Half the size of buckets array
            int halfSize = 1 << (globalDepth - 1);
            // Index of position where new bucket is to be inserted
            int newBucketIndex = (splitIndex < halfSize)
                                     ? buckets.size() - (halfSize - splitIndex)
                                     : splitIndex;
            // Index of bucket from which the data is to be taken
            // As per design, only the first pointer contains data
            int dataCopy = (splitIndex < halfSize) ? splitIndex
                                                   : (splitIndex - halfSize);

            // Create new bucket with depth as global depth
            buckets[newBucketIndex] = new DataBucket(bucketSize, globalDepth);
            // Store the whole data from the bucket to be split
            set<int> bucketData = buckets[dataCopy]->data;
            // Clear contents of the bucket to be split
            buckets[dataCopy]->data.clear();
            // Increment the local depth of the split bucket
            buckets[dataCopy]->localDepth++;

            // The below loop only adds the data to the two buckets
            // 1. the split bucket
            // 2. the newly created bucket
            for (int element : bucketData)
            {
                int bucketIndex = _getBucketIndex(element);
                DataBucket *bucket = buckets[bucketIndex];
                add(element);
            }
        }

        /**
         * Reduce directory to half the size.
         *
         * Assumption: The buckets after the first half of the
         * directory are pointing to the first half of the directory.
         */
        void reduceDirectory()
        {
            globalDepth--;
            buckets.resize(1 << globalDepth);
        }

        /**
         * Find the index of the image to be split
         * @param splitIndex index of the bucket for
         * which image is to be calculated
         * @return index of the image
         */
        int getImageIndex(int splitIndex)
        {
            int oldSize = 1 << (globalDepth - 1);
            int imageIndex = (splitIndex < oldSize)
                                 ? buckets.size() - (oldSize - splitIndex)
                                 : (splitIndex - oldSize);
            return imageIndex;
        }

    public:
        /**
         * Contructor for the hash table.
         * Initial Global Depth is assumed to be 0.
         * @param size capacity of buckets
         */
        ExtendibleHashMap(int _size) : globalDepth(0), bucketSize(_size)
        {
            buckets.resize(1 << globalDepth);
            for (int i = 0; i < buckets.size(); i++)
            {
                buckets[i] = new DataBucket(_size, 0);
            }
        }

        /**
         * Contructor for the hash table with given global depth
         * @param _size capacity of buckets
         * @param _globalDepth initial global depth for the hash table
         */
        ExtendibleHashMap(int _globalDepth, int _size)
            : globalDepth(_globalDepth), bucketSize(_size)
        {
            buckets.resize(1 << globalDepth);
            for (int i = 0; i < buckets.size(); i++)
            {
                buckets[i] = new DataBucket(_size, _globalDepth);
            }
        }

        /**
         * Destructor for the hash table.
         * Frees all the memory allocated for the buckets.
         */
        ~ExtendibleHashMap()
        {
            for (DataBucket *bucket : buckets)
            {
                delete bucket;
            }
            buckets.clear();
        }

        /**
         * Add element to the hash table.
         *
         * @par If bucket overflows,
         * 1. If localDepth of the bucket equals to the globalDepth,
         * we double the directory and split the image.
         * 2. Else only split image is returned.
         *
         * @param key data to be inserted into the hash table
         * @return index of the bucket where data is inserted
         */
        int add(int key)
        {
            int bucketIndex = _getBucketIndex(key);
            DataBucket *bucket = buckets[bucketIndex];
            int ret = bucket->insert(key);
            if (ret == 2)
            {
                if (bucket->localDepth == globalDepth)
                {
                    extend();
                }

                splitImage(bucketIndex);
            }
            return bucketIndex;
        }

        /**
         * Remove the key from the hash table.
         * @param key key to be removed
         *
         * If case key does not exist, no effect on the
         * hash table is performed.
         *
         * @par If a pair of matching buckets has size less
         * than half the capacity, we merge them into single bucket.
         * The pointer in the second bucket now points to the first bucket.
         *
         * @par If local depths of all the buckets in the hash table are same
         * and one less than the global depth, we reduce the directory to half
         * the size.
         */
        void remove(int key)
        {
            int bucketIndex = _getBucketIndex(key);
            int image = getImageIndex(bucketIndex);
            bool areLocalDepthsEqual = true;

            buckets[bucketIndex]->erase(key);
            // If the number of buckets is greater than 1 and
            // the two matching buckets have less than half the capacity
            // of the bucket in both the buckets, merge them.
            if (image != bucketIndex && buckets[bucketIndex]->data.size() <= bucketSize / 2 && buckets[image]->data.size() <= bucketSize / 2)
            {
                // bucketIndex = smaller index
                // image = larger index
                if (bucketIndex > image)
                {
                    swap(bucketIndex, image);
                }

                // Merge the two buckets
                // Insert the data from the bucket in second half of the directory
                // to the bucket in first half of the directory
                for (int element : buckets[image]->data)
                {
                    buckets[bucketIndex]->insert(element);
                }
                // Delete the second bucket
                delete buckets[image];
                // Update the pointer in the second bucket
                buckets[image] = buckets[bucketIndex];
                // Reduce the local depth of the split bucket
                // since both are merged
                buckets[bucketIndex]->localDepth--;
            }

            // For Reducing the Directory Size by double
            // Check if all local depths are equal
            for (int i = 1; i < buckets.size(); i++)
            {
                if (buckets[i]->localDepth != buckets[0]->localDepth)
                {
                    areLocalDepthsEqual = false;
                    break;
                }
            }
            // If all local depths are equal and one less than global depth,
            // Reduce the directory size to half
            if (areLocalDepthsEqual && globalDepth - 1 == buckets[0]->localDepth)
            {
                // reduce directory to half
                reduceDirectory();
            }
        }

        /**
         * Print the contents of the hash table.
         */
        void print()
        {
            cout << endl;
            for (int id = 0; id < buckets.size(); id++)
            {
                cout << "Bucket " << id + 1 << " / " << buckets.size() << endl;
                cout << "Data:\t";
                for (int el : buckets[id]->data)
                {
                    cout << el << " ";
                }
                cout << endl
                     << endl;
            }
        }
    };
}

int main()
{
    int capacity;
    cout << "Enter capacity of each bucket:\t";
    cin >> capacity;

    hashing::ExtendibleHashMap *mp = new hashing::ExtendibleHashMap(0, 2);
    int inp = 1, x;

    while (inp != 0)
    {
        cout << "\nInput Format:\n";
        cout << "0  : Exit the program\n";
        cout << "1 x: Insert an element x (x is an integer)\n";
        cout << "2 x: Remove an element x (x is an integer)\n";
        cout << "3  : Print the hash table\n\n";

        cin >> inp;
        if (inp == 0)
        {
            break;
        }
        switch (inp)
        {
        case 1:
            cin >> x;
            mp->add(x);
            break;
        case 2:
            cin >> x;
            mp->remove(x);
            break;
        case 3:
            mp->print();
            break;
        default:
            cout << "Invalid Input\n";
            break;
        }
    }
    delete mp;
}