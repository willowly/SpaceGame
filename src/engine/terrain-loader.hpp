#include "actor/terrain.hpp"

#include "glm/glm.hpp"

using glm::vec3;

enum class TerrainJobState {
    FINISHED,
    WAITING,
    IN_PROGRESS
};

class TerrainJob {

    std::shared_mutex mutex;
    std::shared_ptr<Terrain> terrain = nullptr;
    ChunkAddress address;
    TerrainJobState state = TerrainJobState::FINISHED;
    public:
        std::atomic<std::thread::id> worker;

        TerrainJobState getJobState() const {
            return state;
        }
        
        bool trySetJob(std::shared_ptr<Terrain> terrain,ChunkAddress address) {

            
            std::unique_lock lock(mutex,std::defer_lock);
            
            if(lock.try_lock()) {
                if(state != TerrainJobState::FINISHED) return false;
                this->address = address;
                this->terrain = terrain;
                state = TerrainJobState::WAITING;
                return true;
            }
            return false;
        }

        //shared_ptr is null if it failed
        std::pair<std::shared_ptr<Terrain>,ChunkAddress> tryGetJob() {

            
            
            std::unique_lock lock(mutex,std::defer_lock);

            
            if(lock.try_lock()) {
                if(state != TerrainJobState::WAITING) return std::pair<std::shared_ptr<Terrain>,ChunkAddress>(nullptr,ChunkAddress());;
                state = TerrainJobState::IN_PROGRESS;
                worker = std::this_thread::get_id();
                return std::pair<std::shared_ptr<Terrain>,ChunkAddress>(terrain,address);
            }
            return std::pair<std::shared_ptr<Terrain>,ChunkAddress>(nullptr,ChunkAddress());
        }

        void finishJob() {
            std::unique_lock lock(mutex);
            
            state = TerrainJobState::FINISHED;
        }
};

class TerrainLoader {

    public:
        static const int terrainJobCount = 64;

        TerrainJobState getJobState(int index) {
            return TerrainJobState::WAITING;//terrainJobs.at(index).getJobState();
        } 
        std::thread::id getJobWorker(int index) {
            return terrainJobs.at(index).worker;
        } 

    private:

    std::thread mainThread;
    std::vector<std::shared_ptr<Terrain>> terrains;
    std::mutex terrainMutex;
    std::atomic<bool> stopSignal;

    std::vector<std::thread> workerThreads;

    
    std::array<TerrainJob,terrainJobCount> terrainJobs;
    
    std::mutex cameraPositionMutex;
    vec3 cameraPosition = {};


    void mainTask() {
        int jobIndex = 0;
        while(!stopSignal) {
            // if(getTerrainVectorSize() == 0) {
            //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // }
            for (int i = 0; i < getTerrainVectorSize(); i++)
            {
                auto terrain = getTerrain(i);
                auto pos = getCameraPosition();
                if(terrain != nullptr) {
                    auto addressOpt = terrain->getNextChunkToload(pos);
                    if(addressOpt == std::nullopt) {
                        continue;
                    }
                    auto address = addressOpt.value();
                    for (size_t i = 0; i < terrainJobCount; i++)
                    {
                        static_assert(terrainJobCount > 0);
                        //std::cout << "MAIN: checking job" << jobIndex << std::endl;
                        jobIndex = getNextJob(jobIndex);
                        if(terrainJobs.at(jobIndex).trySetJob(terrain,address)) {
                            terrain->addPlaceholder(address);
                            //std::cout << "MAIN: setting job" << jobIndex << std::endl;
                            break;
                        };
                    }
                }
            }
        }
    }

    // safely loops around the circular buffer 
    int getNextJob(int currentIndex) {
        currentIndex++;
        if(currentIndex >= terrainJobCount) {
            currentIndex = 0;
        }
        return currentIndex;
    }

    void workerTask() {
        ZoneScoped
        int jobIndex = 0;
        while(!stopSignal) {
            for (size_t i = 0; i < terrainJobCount; i++)
            {
                
                static_assert(terrainJobCount > 0);
                jobIndex = getNextJob(jobIndex);
                auto pair = (terrainJobs.at(jobIndex).tryGetJob());
                if(pair.first != nullptr) {
                    pair.first->addChunk(pair.second);
                    terrainJobs.at(jobIndex).finishJob();
                    //std::cout << std::this_thread::get_id() << "WORKER: done job" << jobIndex << std::endl;
                }
            }
        }
    }

    vec3 getCameraPosition() {
        std::scoped_lock lock(cameraPositionMutex);
        return cameraPosition;
    }

    std::shared_ptr<Terrain> getTerrain(int index) {
        ZoneScoped
        std::scoped_lock lock(terrainMutex);
        assert(index >= 0);
        if(static_cast<int>(terrains.size()) <= index) {
            return nullptr;
        }
        return terrains.at(index);
    }

    int getTerrainVectorSize() {
        ZoneScoped
        std::scoped_lock lock(terrainMutex);
        return static_cast<int>(terrains.size());
    }

    public:
        void addTerrain(std::shared_ptr<Terrain> terrain) {
            ZoneScoped
            std::scoped_lock lock(terrainMutex);
            terrains.push_back(terrain);
        }

        void start() {

            if(workerThreads.size() > 0) {
                throw std::runtime_error("tried to start terrain loader when its already going!");
            }

            int allowedThreads = std::thread::hardware_concurrency();
            std::cout << allowedThreads << " threads are allowed" << std::endl;
            int allowedWorkerThreads = allowedThreads - 1;

            if(allowedWorkerThreads <= 0) {
                throw std::runtime_error("not enough threads " + std::to_string(allowedThreads));
            }

            if(allowedWorkerThreads > 8) {
                allowedWorkerThreads = 8;
            }

            stopSignal = false;
            mainThread = std::thread(&TerrainLoader::mainTask,this);
            for (size_t i = 0; i < allowedWorkerThreads; i++)
            {
                workerThreads.push_back(std::thread(&TerrainLoader::workerTask,this));
            }
            
            //mainThread = std::thread(&TerrainLoader::mainTask,this);
        }

        void setCameraPosition(vec3 position) {
            std::scoped_lock lock(cameraPositionMutex);
            cameraPosition = position;
        }

        void stop() {
            stopSignal = true;
            
            mainThread.join();
            for (auto& workerThread : workerThreads)
            {
                workerThread.join();
            }
            workerThreads.clear();
        }
    
    
};